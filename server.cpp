#include <iostream>
#include <deque>
#include <set>
#include <memory>
#include <boost/asio.hpp>
#include "notification.hpp"

using boost::asio::ip::tcp;

class chat_connection;

using ptr_user = std::shared_ptr<chat_connection>;

// TODO: update author on read

class chat_room {
public:
    const char* PASSWORD = "qwerty";

    bool join(ptr_user user) {
        participants_.insert(user);
    }

    void leave(ptr_user user) {
        participants_.erase(user);
    }

    void broadcast(ptr_user user, const Notification& ntf) {
        // parse command logic
    }

private:
    std::set<ptr_user> participants_;
    enum { max_recent_msgs = 100 };
    std::deque<Notification> recent_ntfs_;
};

class chat_connection
    : public std::enable_shared_from_this<chat_connection>
{
public:
    chat_connection(tcp::socket&& socket, chat_room& room)
            : socket_(std::forward<tcp::socket>(socket)),
            room_(room)
    {
    }

    void start()
    {
        read_notifications();
    }

    void deliver(const Notification& ntf)
    {
        bool write_in_progress = !ntfs_write.empty();
        ntfs_write.push_back(ntf);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:

    void close_connection(std::string info = "") {
        //TODO: move to room
        //std::cerr << "DISCONNECTED->"<< login_ << (info.empty() ? "" : info) << "\n";
        room_.leave(shared_from_this());
    }

    bool check_validation() {
        if (ntf_.get_command() == room_.PASSWORD) {
            login_ = ntf_.get_author();
            if (room_.join(shared_from_this()))
                return true;
            else {
                close_connection("Duplicate login");
                return false;
            }
        } else {
            close_connection("Wrong password");
            return false;
        }
    }

    void do_write()
    {
        auto new_message = ntfs_write.front().encode();
        boost::asio::async_write(socket_,
                 boost::asio::buffer(new_message, new_message.size()),
                 // TODO: remove self
                 [this, self = shared_from_this()](boost::system::error_code ec,
                         std::size_t /*length*/)
                 {
                     if (!ec)
                     {
                         ntfs_write.pop_front();
                         if (!ntfs_write.empty())
                         {
                             do_write();
                         }
                     }
                     else
                     {
                         close_connection("Something goes wrong with reading");
                         room_.leave(shared_from_this());
                     }
                 });
    }

    void read_notifications() {
        boost::asio::async_read_until(socket_, buffer_, Notification::DELIMITER,
                [this](boost::system::error_code ec,
                        std::size_t /*length*/)
                {
                    if (!ec) {
                        try {
                            std::string str(
                                    (std::istreambuf_iterator<char>(&buffer_)),
                                            std::istreambuf_iterator<char>());

                            ntf_.update(str);

                            if (login_.empty()) {
                                // will check only once
                                if (!check_validation())
                                    return;
                            }
                            ntf_.update_author(login_);

                            room_.broadcast(shared_from_this(), ntf_);
                            read_notifications();
                        }
                        catch (...) {
                            close_connection("Bad data sended");
                            return;
                        }
                    } else {
                        close_connection("Something goes wrong with reading");
                    }
        });
    }

    std::string login_ = ""; // default value for non-validated user. Also "system" on client side
    boost::asio::streambuf buffer_;
    Notification ntf_;

    tcp::socket socket_;
    chat_room& room_;
    std::deque<Notification> ntfs_write;
};


class chat_server
{
public:
    chat_server(boost::asio::io_service& io_service,
            const unsigned int port)
            : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
              socket_(io_service)
    {
        start_accept();
    }

private:
    void start_accept()
    {
        acceptor_.async_accept(socket_,
                [this](boost::system::error_code ec)
                {
                    if (!ec)
                    {
                        std::make_shared<chat_connection>(std::move(socket_), room_)->start();
                    }

                    start_accept();
                });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    chat_room room_;
};

int main(int argc, char* argv[]) {
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: chat_server <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;
        chat_server server(io_service, std::atoi(argv[1]));

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}