#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "utils/client.hpp"

Notification read_credentials(std::string& login) {
    std::string password;
    std::cout << "Login: ";
    while (login.empty()) std::getline(std::cin, login);

    std::cout << "Password: ";
    std::getline(std::cin, password);

    return ChatClient::credentials2ntf(login, password);
}

int main(int argc, char* argv[]) {
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: ChatClient <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        ChatClient c(io_service, endpoint_iterator);

        std::thread t([&io_service](){ io_service.run(); });

        std::string text, login;
        c.write(read_credentials(login));
        // FIXME: each new notification will tear current user input
        while (std::getline(std::cin, text)) {
            // exit from the loop
            if (text.empty()) break;
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