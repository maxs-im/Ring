//
// Created by maxs on 19/07/19.
//

#ifndef RING_NOTIFICATION_HPP
#define RING_NOTIFICATION_HPP

#include <regex>
#include <string>

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
            command_ = m[0].str();
            message_ = m[1].str();
        } else {
            message_ = text;
        }
    }

    void update(const std::string& str) {
        std::smatch m;
        std::regex rgx ("(.*) @([^\\s]*) (.*)");

        if (std::regex_search(str, m, rgx)) {
            author_ = m[0];
            command_ = m[1];
            message_ = m[2];
        } else {
            // TODO: prettify
            throw;
        }
    }

    std::string decode() const {
        // TODO: finish with more situation info + names
        if (command_.empty()) {
            return message_;
        } else {
            return "@" + command_ + " " + message_;
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

#endif //RING_NOTIFICATION_HPP