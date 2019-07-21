//
// Created by maxs on 19/07/19.
//

#ifndef RING_NOTIFICATION_HPP
#define RING_NOTIFICATION_HPP

#include <regex>
#include <string>
#include <map>

class Notification {
public:
    enum { DELIMITER = '\n' };

    Notification() = default;

    Notification(std::string author, std::string command, std::string message)
            : author_(std::move(author)),
              command_(std::move(command)),
              message_(std::move(message)) {};

    Notification(std::string author, const std::string& text)
            : author_(std::move(author))
    {
        std::smatch m;
        std::regex rgx ("@([^\\s]+) (.*)");
        if (std::regex_search(text, m, rgx)) {
            command_ = m[1].str();
            message_ = m[2].str();
        } else {
            message_ = text;
        }
    }

    void update(const std::string& str) {
        std::smatch m;
        std::regex rgx ("(.*) @([^\\s]*) (.*)");

        if (std::regex_search(str, m, rgx)) {
            author_ = m[1];
            command_ = m[2];
            message_ = m[3];
        } else {
            // TODO: prettify
            throw;
        }
    }

    std::string encode() const {
        return author_ + " @" + command_ + " " + message_ +
               static_cast<char>(DELIMITER);
    }

    bool is_admin() const {
        return author_ == "Admin";
    }

    void update_author(const std::string& author) {
        author_ = author;
    }


    std::string get_author() const {
        return author_;
    }
    std::string get_command() const {
        return command_;
    }
    std::string get_message() const {
        return message_;
    }
private:
    std::string author_, command_, message_;
};

namespace NTFCommand {
    const char *JOIN = "join",
            *LEAVE = "leave",
            *KICK = "kick",
            *PASSWORD = "password";

    std::string decode_notification(const Notification& ntf) {
        if (ntf.get_author().empty()) {
            // system case
            auto cmd(ntf.get_command());

            if (JOIN == cmd)
                return "\tGreet " + ntf.get_message() + " in chat";
            if (LEAVE == cmd)
                return "\t" + ntf.get_message() + " left chat";
            if (KICK == cmd)
                return "\tDISCONNECTED: " + ntf.get_message();
            return "\t----->" + ntf.get_message() + "<-----";
        } else {
            return ntf.get_author() + ": " +
                   (ntf.get_command().empty() ? "" : "@" + ntf.get_command() + " ") +
                   ntf.get_message();
        }
    }
}

#endif //RING_NOTIFICATION_HPP