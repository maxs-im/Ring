//
// Entrance to the server application
//

#include <iostream>
#include <boost/asio.hpp>
#include "utils/server.hpp"

int main(int argc, char* argv[]) {
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: ChatServer <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;
        ChatServer server(io_service, std::atoi(argv[1]));

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}