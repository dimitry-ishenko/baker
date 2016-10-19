////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "monitor.hpp"

#include <chrono>
#include <stdexcept>
#include <string>

#include <poll.h>

using namespace std::literals::chrono_literals;
using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
monitor::monitor(asio::io_service& io, log::book clog) try :
    timer_(io), clog_(std::move(clog))
{
    clog_(level::debug) << "connecting to udev" << std::endl;
    udev_ = udev_new();
    if(!udev_) throw std::runtime_error("failed to connect to udev");

    clog_(level::debug) << "connecting to udev monitor" << std::endl;
    monitor_ = udev_monitor_new_from_netlink(udev_, "udev");
    if(!monitor_) throw std::runtime_error("unable to connect to udev monitor");

    clog_(level::debug) << "adding monitor filter for hidraw" << std::endl;
    udev_monitor_filter_add_match_subsystem_devtype(monitor_, "hidraw", nullptr);

    clog_(level::debug) << "enabling receiving" << std::endl;
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
        clog_(level::debug) << "disconnecting from udev monitor" << std::endl;
        udev_monitor_unref(monitor_);
        monitor_ = nullptr;
    }
    if(udev_)
    {
        clog_(level::debug) << "disconnecting from udev" << std::endl;
        udev_unref(udev_);
        udev_ = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
void monitor::enumerate()
{
    ////////////////////
    clog_(level::debug) << "connecting to udev enumerate" << std::endl;
    auto enumerate = udev_enumerate_new(udev_);
    if(!enumerate) throw std::runtime_error("failed to connect to udev enumerate");

    clog_(level::debug) << "adding filter for hidraw" << std::endl;
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");

    clog_(level::debug) << "scanning for devices" << std::endl;
    udev_enumerate_scan_devices(enumerate);

    clog_(level::debug) <<  "enumerating existing devices" << std::endl;
    udev_list_entry* entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(enumerate))
    {
        auto device = udev_device_new_from_syspath(udev_, udev_list_entry_get_name(entry));
        if(device)
        {
            std::string subsystem = udev_device_get_subsystem(device);
            if(subsystem == "hidraw")
            {
                std::string path = udev_device_get_devnode(device);
                clog_(level::debug) << "adding device " << path << std::endl;

                device_added_(path);
            }

            udev_device_unref(device);
        }
    }

    clog_(level::debug) << "disconnecting from udev enumerate" << std::endl;
    udev_enumerate_unref(enumerate);

    schedule_poll();
}

////////////////////////////////////////////////////////////////////////////////
void monitor::schedule_poll()
{
    clog_(level::debug) << "scheduling poll" << std::endl;
    timer_.expires_from_now(0s);
    timer_.async_wait(std::bind(&monitor::poll, this));
}

////////////////////////////////////////////////////////////////////////////////
void monitor::poll()
{
    pollfd fds[] =
    {
        { udev_monitor_get_fd(monitor_), POLLIN, 0 },
    };

    if(::poll(fds, sizeof(fds) / sizeof(fds[0]), -1) > 0)
    {
        auto device = udev_monitor_receive_device(monitor_);
        if(device)
        {
            std::string subsystem = udev_device_get_subsystem(device);
            if(subsystem == "hidraw")
            {
                std::string path = udev_device_get_devnode(device);
                std::string action = udev_device_get_action(device);

                if(action == "add")
                {
                    clog_(level::debug) << "adding device " << path << std::endl;
                    device_added_(path);
                }
                else if(action == "remove")
                {
                    clog_(level::debug) << "removing device " << path << std::endl;
                    device_removed_(path);
                }
            }

            udev_device_unref(device);
        }
    }

    schedule_poll();
}

////////////////////////////////////////////////////////////////////////////////
}
