#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class chat_connection
    : public std::enable_shared_from_this<chat_connection>
{
public:
    chat_connection(tcp::socket&& socket)
            : socket_(std::forward<tcp::socket>(socket))
    {
    }

    void start()
    {
        // TODO: check password, read messages
        boost::asio::async_write(socket_, boost::asio::buffer("some text", 10),
                [](boost::system::error_code ec, std::size_t /*length*/){
                    std::cout << "Connected\n";
                });
    }

private:

    tcp::socket socket_;
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
                        std::make_shared<chat_connection>(std::move(socket_))->start();
                    }

                    start_accept();
                });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
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