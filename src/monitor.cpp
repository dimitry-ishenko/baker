////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "log/book.hpp"
#include "monitor.hpp"

#include <chrono>
#include <stdexcept>

using namespace std::literals::chrono_literals;

using log::level;
extern log::book clog;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
monitor::monitor(asio::io_service& io) try : timer_(io)
{
    clog(level::debug) << "connecting to udev" << std::endl;
    udev_ = udev_new();
    if(!udev_) throw std::runtime_error("failed to connect to udev");

    clog(level::debug) << "connecting to udev monitor" << std::endl;
    monitor_ = udev_monitor_new_from_netlink(udev_, "udev");
    if(!monitor_) throw std::runtime_error("unable to connect to udev monitor");

    clog(level::debug) << "adding monitor filter for hidraw" << std::endl;
    udev_monitor_filter_add_match_subsystem_devtype(monitor_, "hidraw", nullptr);

    clog(level::debug) << "enabling receiving" << std::endl;
    udev_monitor_enable_receiving(monitor_);

    ////////////////////
    timer_.expires_from_now(0s);
    timer_.async_wait(std::bind(&monitor::enumerate, this));
}
catch(...)
{
    close();
    throw;
}

////////////////////////////////////////////////////////////////////////////////
void monitor::close() noexcept
{
    asio::error_code ec;
    timer_.cancel(ec);

    if(monitor_)
    {
        clog(level::debug) << "disconnecting from udev monitor" << std::endl;
        udev_monitor_unref(monitor_);
        monitor_ = nullptr;
    }
    if(udev_)
    {
        clog(level::debug) << "disconnecting from udev" << std::endl;
        udev_unref(udev_);
        udev_ = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
void monitor::enumerate()
{
    clog(level::debug) << "enumerating existing devices" << std::endl;

    // enumerate

    timer_.expires_from_now(0s);
    timer_.async_wait(std::bind(&monitor::poll, this));
}

////////////////////////////////////////////////////////////////////////////////
void monitor::poll()
{
    if(first_poll)
    {
        clog(level::debug) << "polling for changes" << std::endl;
        first_poll = false;
    }

    // poll

    timer_.expires_from_now(0s);
    timer_.async_wait(std::bind(&monitor::poll, this));
}

////////////////////////////////////////////////////////////////////////////////
}
