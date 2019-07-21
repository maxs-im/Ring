//
// Client part of chat
//

#ifndef RING_CLIENT_HPP
#define RING_CLIENT_HPP

#include "notification.hpp"
#include <deque>
#include <functional>
#include <boost/asio.hpp>

class ChatClient
{
public:
    ChatClient(boost::asio::io_service& io_service,
                boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
                std::function<void(const std::string&)> fn_notify);

    static Notification credentials2ntf(const std::string& login,
            const std::string& password);

    void write(const Notification& ntf);
    void close();
private:
    void do_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

    // recursive async reading
    void read_notifications();
    // recursive async writing
    void do_write();

    // queue for writing
    std::deque<Notification> ntfs_write;
    // buffers for reading and storing current notification
    boost::asio::streambuf buffer_;
    Notification ntf;

    // handle important messages
    std::function<void(const std::string&)> fn_notify_;

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
};

#endif //RING_CLIENT_HPP
