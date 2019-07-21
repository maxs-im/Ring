#include <iostream>
#include <deque>
#include <set>
#include <memory>
#include <stdexcept>
#include <boost/asio.hpp>
#include "notification.hpp"

using boost::asio::ip::tcp;

class chat_member {
public:
    virtual std::string get_login() = 0;
    virtual void deliver(const Notification& ntf) = 0;
};

class chat_room {
    using ptr_user = std::shared_ptr<chat_member>;
public:

    void verify(ptr_user user, const Notification& ntf) {
        if (ntf.get_command() == "password" &&
            ntf.get_message() == PASSWORD_)
        {
            auto new_user = ntf.get_author();
            for (const auto& p : participants_) {
                if (p->get_login() == new_user)
                    throw std::runtime_error("Duplicate login");
            }

            join_(user);
        } else {
            throw std::runtime_error("Wrong password");
        }
    }

    void leave(ptr_user user, const std::string& info) {
        participants_.erase(user);
        broadcast(Notification("", "leave", user->get_login()));
        user->deliver(Notification("", "kick", info));

        std::cout << "DISCONNECTED->"<< user->get_login() << ":" << info << "\n";
    }

    void broadcast(const Notification& ntf) {
        // notification logic provided here
        if (ntf.get_author() == ADMIN_LOGIN) {
            if (ntf.get_command() == "kick") {
                auto it = std::find_if(participants_.begin(), participants_.end(),
                        [s = ntf.get_message()](const ptr_user item)
                        { return item->get_login() == s; });
                if (it != participants_.end()) {
                    leave(*it, "Kicked by " + ntf.get_author());
                    return;
                }
            }
        }

        recent_ntfs_.push_back(ntf);
        while (recent_ntfs_.size() > max_recent_msgs)
            recent_ntfs_.pop_front();

        for (const auto p : participants_)
            p->deliver(ntf);
    }

private:
    void join_(ptr_user user) {
        // notify members about new user
        broadcast(Notification("", "join", user->get_login()));
        participants_.insert(user);
        // join current
        for (const auto& ntf : recent_ntfs_)
            user->deliver(ntf);

        std::cout << "CONNECTED->"<< user->get_login() << "\n";
    }

    const char *PASSWORD_ = "qwerty",
                *ADMIN_LOGIN = "Admin";
    std::set<ptr_user> participants_;
    enum { max_recent_msgs = 100 };
    std::deque<Notification> recent_ntfs_;
};

class chat_connection
    :   public chat_member,
        public std::enable_shared_from_this<chat_connection>
{
public:
    chat_connection(tcp::socket&& socket, chat_room& room)
            : socket_(std::forward<tcp::socket>(socket)),
            room_(room)
    {
    }

    std::string get_login() {
        return login_;
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
        room_.leave(shared_from_this(), info);
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

                            // check validation
                            if (login_.empty()) {
                                // will check only once
                                try {
                                    room_.verify(shared_from_this(), ntf_);
                                } catch (std::exception e) {
                                    close_connection(e.what());
                                    return;
                                }
                            }
                            ntf_.update_author(login_);

                            room_.broadcast(ntf_);
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