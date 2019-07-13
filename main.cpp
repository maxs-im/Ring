#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class printer {
public:
    printer(boost::asio::io_service& io)
            : timer_1(io, boost::posix_time::seconds(1)),
              timer_2(io, boost::posix_time::seconds(1)),
              strand_(io),
              count_(0)
    {
        timer_1.async_wait(strand_.wrap(boost::bind(&printer::print1, this)));

        timer_2.async_wait(strand_.wrap(boost::bind(&printer::print2, this)));
    }

    void print1()
    {
        if (count_ < 10)
        {
            std::cout << "Timer 1: " << count_ << std::endl;
            ++count_;

            timer_1.expires_at(timer_1.expires_at() + boost::posix_time::seconds(1));

            timer_1.async_wait(strand_.wrap(boost::bind(&printer::print1, this)));
        }
    }

    void print2()
    {
        if (count_ < 10)
        {
            std::cout << "Timer 2: " << count_ << std::endl;
            ++count_;

            timer_2.expires_at(timer_2.expires_at() + boost::posix_time::seconds(1));

            timer_2.async_wait(strand_.wrap(boost::bind(&printer::print2, this)));
        }
    }

    ~printer()
    {
        std::cout << "Final count is " << count_ << std::endl;
    }
private:
    boost::asio::io_service::strand strand_;
    boost::asio::deadline_timer timer_1;
    boost::asio::deadline_timer timer_2;
    int count_;
};

int main() {
    boost::asio::io_service io;
    printer p(io);

    boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
    io.run();
    t.join();

    return 0;
}