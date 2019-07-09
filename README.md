# Client Server Chat using TCP

Simple chat server and client using TCP.
Use console UI with simple std::cout/cin(or similar). UI logic should be as simple as possible and does not influence score in this task.

__Requirements__:
* Client at startup should setup its name and server password. Server side can hardcode password for simplicity. If password is wrong server disconnects client with message visible at client side.
* When client joins server, message with client name is broadcasted
* Client should be able to send messages and see received messages from other clients with their names
* Client with name 'Admin' can send a message in format: '@kick Name' which disconnects client Name if it exists.

__Implementation details__:
* Use C++11 - C++17 compiler; use CMake for build system; any operating system, however Linux is preferable. Would be a plus if code is written in cross platform way, but no need to test this.
* It is suggested to use boost::asio, but also may use Berkley sockets.
* Can use any other libraries you want, but make the choice reasonably.
* Write code as clean and as readable as possible.
* Message format/protocol can be binary or json, you should select. Json is definitely slower but it doesn't influence optimization score in this task
* Provide detailes description how to build and start your project
* Write unit tests using gtest google test framework

## Author

**Maksym Halchenko** - [maxs-im](https://github.com/maxs-im)

## Realization

Task should be finished in two weeks.

[Private Github Repository](https://github.com/maxs-im/Ring)
