//
// Client part of chat realization
//

#include "client.hpp"

#include <iostream>

using boost::asio::ip::tcp;

ChatClient::ChatClient(boost::asio::io_service& io_service,
            tcp::resolver::iterator endpoint_iterator,
            std::function<void(const std::string&)> fn_notify)
        : io_service_(io_service),
          socket_(io_service),
          fn_notify_(std::move(fn_notify))
{
    do_connect(std::move(endpoint_iterator));
}

Notification ChatClient::credentials2ntf(const std::string& login,
        const std::string& password)
{
    return {login, NTFCommand::PASSWORD, password};
}


void ChatClient::write(const Notification& ntf) {
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

void ChatClient::close()
{
    io_service_.post([this]() { socket_.close(); });
}

void ChatClient::do_connect(tcp::resolver::iterator endpoint_iterator)
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

void ChatClient::read_notifications() {
    boost::asio::async_read_until(socket_, buffer_, Notification::DELIMITER,
              [this](boost::system::error_code ec, std::size_t /*length*/)
              {
                  if (!ec)
                  {
                      // all (disconnecting) notification logic provided here
                      bool leave;
                      try {
                          std::istream is(&buffer_);
                          std::string str;
                          std::getline(is, str);

                          ntf.update(str);

                          leave = ntf.get_command() == NTFCommand::KICK;
                          fn_notify_(NTFCommand::decode_notification(ntf));
                      }
                      catch (...) {
                          leave = true;
                          fn_notify_("Smth goes wrong with receiving");
                      }

                      if (leave) {
                          socket_.close();
                      } else {
                          read_notifications();
                      }
                  } else {
                      fn_notify_("Bad data received");
                      socket_.close();
                  }
              });
}

void ChatClient::do_write() {
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
                     fn_notify_("Smth goes wrong with sending");
                     socket_.close();
                 }
             });
}
