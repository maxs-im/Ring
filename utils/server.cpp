//
// Server part of chat realization
//

#include "server.hpp"

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

ChatRoom::ChatRoom(const std::function<void(const std::string&)>& fn_notify)
    : fn_notify_(fn_notify)
{}

void ChatRoom::verify(ptr_user user, const Notification& ntf) {
    if (ntf.get_message() == PASSWORD_)
    {
        auto new_user = ntf.get_author();
        for (const auto& p : participants_) {
            if (p->get_login() == new_user)
                throw std::invalid_argument("Duplicate login");
        }
        user->set_login(new_user);
        join_(user);
    } else {
        throw std::invalid_argument("Wrong password");
    }
}

void ChatRoom::leave(ptr_user user, const std::string& info) {
    user->deliver(Notification("", NTFCommand::KICK, info));

    if (participants_.erase(user)) {
        broadcast(Notification("", NTFCommand::LEAVE, user->get_login()));
        if (fn_notify_)
            fn_notify_("DISCONNECTED->" + user->get_login() + ":" + info);
    }
}

void ChatRoom::broadcast(const Notification& ntf) {
    // notification logic provided here
    if (ntf.get_author() == ADMIN_LOGIN) {
        if (ntf.get_command() == NTFCommand::KICK) {
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

    for (const auto p : participants_) {
        // here we can ignore back-sending for author
        p->deliver(ntf);
    }
}

void ChatRoom::join_(ptr_user user) {
    // notify members about new user
    broadcast(Notification("", NTFCommand::JOIN, user->get_login()));
    participants_.insert(user);
    // join current
    for (const auto& ntf : recent_ntfs_)
        user->deliver(ntf);

    if (fn_notify_)
        fn_notify_("CONNECTED->" + user->get_login());
}

ServerConnection::ServerConnection(tcp::socket&& socket, ChatRoom& room)
        : socket_(std::forward<tcp::socket>(socket)),
          room_(room)
{
}

void ServerConnection::start()
{
    read_notifications();
}

void ServerConnection::deliver(const Notification& ntf)
{
    bool write_in_progress = !ntfs_write.empty();
    ntfs_write.push_back(ntf);
    if (!write_in_progress)
    {
        do_write();
    }
}

std::string ServerConnection::get_login() {
    return login_;
}

void ServerConnection::set_login(const std::string& str) {
    login_ = str;
}

void ServerConnection::close_connection(std::string info) {
    room_.leave(shared_from_this(), info);
}

void ServerConnection::do_write()
{
    auto new_message = ntfs_write.front().encode();
    boost::asio::async_write(socket_,
                             boost::asio::buffer(new_message, new_message.size()),
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

void ServerConnection::read_notifications() {
    boost::asio::async_read_until(socket_, buffer_, Notification::DELIMITER,
              [this, self = shared_from_this()](boost::system::error_code ec,
                                                std::size_t /*length*/)
              {
                  if (!ec) {
                      try {
                          // TODO: change buffer
                          std::istream is(&buffer_);
                          std::string str;
                          std::getline(is, str);

                          ntf_.update(str);

                          // check validation
                          if (login_.empty()) {
                              // will check only once
                              try {
                                  room_.verify(shared_from_this(), ntf_);
                              } catch (std::exception& e) {
                                  close_connection(e.what());
                                  return;
                              }
                          } else {
                              room_.broadcast(ntf_);
                          }

                          read_notifications();
                      }
                      catch (...) {
                          close_connection("Invalid data sent");
                          return;
                      }
                  } else {
                      close_connection("Something goes wrong with reading");
                  }
              });
}


// --------------------------- ChatServer ---------------------------

ChatServer::ChatServer(boost::asio::io_service& io_service,
            const unsigned int port,
            std::function<void(const std::string&)> fn_notify)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          socket_(io_service),
          fn_notify_(std::move(fn_notify)),
          room_({fn_notify_})
{
    start_accept();
}

void ChatServer::start_accept()
{
    acceptor_.async_accept(socket_,
               [this](boost::system::error_code ec)
               {
                   if (!ec)
                   {
                       std::make_shared<ServerConnection>(std::move(socket_), room_)->start();
                   }

                   start_accept();
               });
}