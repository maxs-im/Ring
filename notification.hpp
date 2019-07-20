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

    Notification(){}

    Notification(std::string author, std::string command, std::string message)
                 : author_(std::move(author)),
                 command_(std::move(command)),
                 message_(std::move(message)) {};

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

    static Notification create(const std::string& text,
            const std::string& author = "")
    {
        std::string command, message;
        std::smatch m;
        std::regex rgx ("@([^\\s]+) (.*)");
        if (std::regex_search(text, m, rgx)) {
            command = m[0].str();
            message = m[1].str();
        } else {
            message = text;
        }

        return {author, command, message};
    }

    std::string get_message() const {
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

    bool is_system() const {
        return author_.empty();
    }

    bool is_banned() const {
        return command_ == "kick" || command_ == "password";
    }

    void update_author(const std::string& author) {
        author_ = author;
    }

private:
    std::string author_, command_, message_;
};

#endif //RING_NOTIFICATION_HPP