#include <iostream>
#include <deque>
#include <thread>
#include <boost/asio.hpp>
#include "notification.hpp"

using boost::asio::ip::tcp;

namespace console {
    Notification read_credentials(std::string& login) {
        std::string password;
        std::cout << "Login: ";
        while (login.empty()) std::getline(std::cin, login);

        std::cout << "Password: ";
        std::getline(std::cin, password);

        return {login, "password", password};
    }

    void print_notification(std::string message) {
        // TODO: prettify
        std::cout << message << "\n";
    }
}


class chat_client
{
public:
    chat_client(boost::asio::io_service& io_service,
                tcp::resolver::iterator endpoint_iterator)
            : io_service_(io_service),
              socket_(io_service)
    {
        do_connect(std::move(endpoint_iterator));
    }

    void write(const Notification& ntf) {
        io_service_.post(
                [this, ntf]()
                {
                    bool write_in_progress = !ntfs_write.empty();
                    ntfs_write.push_back(ntf);
                    if (!write_in_progress)
                    {
                        do_write();
                    }
                });
    }

    void close()
    {
        io_service_.post([this]() { socket_.close(); });
    }
private:
    void do_connect(tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::async_connect(socket_, std::move(endpoint_iterator),
                [this](boost::system::error_code ec, tcp::resolver::iterator)
                {
                    if (!ec)
                    {
                        read_notifications();
                    }
                });
    }

    void read_notifications() {
        boost::asio::async_read_until(socket_, buffer_, Notification::DELIMITER,
                [this](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (!ec)
                    {
                        // all (disconnecting) notification logic provided here
                        bool leave;
                        try {
                            std::string str(
                                    (std::istreambuf_iterator<char>(&buffer_)),
                                    std::istreambuf_iterator<char>());

                            ntf.update(str);
                            leave = ntf.get_command() == "kick";
                            console::print_notification(
                                    ntf.decode());
                        }
                        catch (...) {
                            leave = true;
                            console::print_notification("Smth goes wrong with receiving");
                        }

                        if (leave) {
                            socket_.close();
                        } else {
                            read_notifications();
                        }
                    } else {
                        console::print_notification("Bad data received");
                        socket_.close();
                    }
                });
    }

    void do_write() {
        auto new_message = ntfs_write.front().encode();
        boost::asio::async_write(socket_,
                boost::asio::buffer(new_message, new_message.size()),
                        [this](boost::system::error_code ec, std::size_t /*length*/)
                        {
                            if (!ec)
                            {
                                ntfs_write.pop_front();
                                if (!ntfs_write.empty())
                                {
                                    do_write();
                                }
                            } else {
                                console::print_notification("Smth goes wrong with sending");
                                socket_.close();
                            }
                        });
    }


    boost::asio::streambuf buffer_;
    Notification ntf;
    std::deque<Notification> ntfs_write;

    boost::asio::io_service& io_service_;
    tcp::socket socket_;
};

int main(int argc, char* argv[]) {
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        chat_client c(io_service, endpoint_iterator);

        std::thread t([&io_service](){ io_service.run(); });

        std::string text, login;
        c.write(console::read_credentials(login));
        // FIXME: each new notification will tear current user input
        while (std::getline(std::cin, text)) {
            c.write({login, text});
        }

        c.close();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}