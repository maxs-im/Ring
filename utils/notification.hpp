//
//  Data exchange tools
//

#ifndef RING_NOTIFICATION_HPP
#define RING_NOTIFICATION_HPP

#include <string>

class Notification {
public:
    enum { DELIMITER = '\n' };

    Notification() = default;
    Notification(std::string author, std::string command, std::string message);
    // create object from author's message
    Notification(std::string author, const std::string& text);

    // update existing object with new encoded string
    void update(const std::string& str);

    std::string encode() const;

    std::string get_author() const;
    std::string get_command() const;
    std::string get_message() const;
private:
    std::string author_, command_, message_;
};

// Use separated namespace for adding new functionality without changing class
namespace NTFCommand {
    // used command constants
    constexpr const char *JOIN = "join",
            *LEAVE = "leave",
            *KICK = "kick",
            *PASSWORD = "password";

    // original message with adding author in prefix
    std::string decode_notification(const Notification& ntf);
}

#endif //RING_NOTIFICATION_HPP