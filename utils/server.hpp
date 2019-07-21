//
// Server part of chat
//

#ifndef RING_SERVER_HPP
#define RING_SERVER_HPP

#include "notification.hpp"
#include <deque>
#include <set>
#include <memory>
#include <boost/asio.hpp>

class ServerConnection;

// Handling general chat data
class ChatRoom {
    using ptr_user = std::shared_ptr<ServerConnection>;
public:
    // verify credentials -> and then join to chat
    void verify(ptr_user user, const Notification& ntf);

    void leave(ptr_user user, const std::string& info);

    // logically send one notification for all participants
    void broadcast(const Notification& ntf);

private:
    void join_(ptr_user user);

    std::set<ptr_user> participants_;
    std::deque<Notification> recent_ntfs_;

    // some useful constants that refer to such room
    const char *PASSWORD_ = "qwerty",
            *ADMIN_LOGIN = "Admin";
    enum { max_recent_msgs = 100 };
};

class ServerConnection
        : public std::enable_shared_from_this<ServerConnection>
{
public:
    ServerConnection(boost::asio::ip::tcp::socket&& socket, ChatRoom& room);
    void start();

    // send notification for user
    void deliver(const Notification& ntf);

    std::string get_login();
    void set_login(const std::string& str);
private:

    void close_connection(std::string info = "");

    // recursive async reading
    void read_notifications();
    // recursive async writing
    void do_write();

    // user's(connection) name like id
    // Note: default value for non-validated user. Also "system" on client side
    std::string login_ = "";

    // queue for writing
    std::deque<Notification> ntfs_write;
    // buffers for reading and storing current notification
    boost::asio::streambuf buffer_;
    Notification ntf_;

    boost::asio::ip::tcp::socket socket_;
    ChatRoom& room_;
};

class ChatServer
{
public:
    ChatServer(boost::asio::io_service& io_service,
                const unsigned int port);

private:
    void start_accept();

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    ChatRoom room_;
};


#endif //RING_SERVER_HPP
