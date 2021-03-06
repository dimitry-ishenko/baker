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

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
monitor::monitor(asio::io_service& io, log::book clog, bool poll) try :
    closing(io), timer_(io), clog_(std::move(clog)), poll_(poll), stream_(io)
{
    clog_(level::info) << "Connecting to udev" << std::endl;
    udev_ = udev_new();
    if(!udev_) throw std::runtime_error("Failed to connect to udev");

    clog_(level::debug) << "Connecting to udev monitor" << std::endl;
    monitor_ = udev_monitor_new_from_netlink(udev_, "udev");
    if(!monitor_) throw std::runtime_error("Unable to connect to udev monitor");

    clog_(level::debug) << "Adding monitor filter for hidraw" << std::endl;
    udev_monitor_filter_add_match_subsystem_devtype(monitor_, "hidraw", nullptr);

    clog_(level::debug) << "Enabling receiving" << std::endl;
    udev_monitor_enable_receiving(monitor_);

    stream_.assign(udev_monitor_get_fd(monitor_));

    ////////////////////
    using namespace std::literals::chrono_literals;

    clog_(level::trace) << "Scheduling enumerate" << std::endl;
    timer_.expires_from_now(0ms);
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
    clog_(level::debug) << "Cancelling poll/enumerate" << std::endl;
    asio::error_code ec;
    timer_.cancel(ec);

    if(monitor_)
    {
        clog_(level::debug) << "Disconnecting from udev monitor" << std::endl;
        udev_monitor_unref(monitor_);
        monitor_ = nullptr;
    }
    if(udev_)
    {
        clog_(level::info) << "Disconnecting from udev" << std::endl;
        udev_unref(udev_);
        udev_ = nullptr;
    }

    closing::close();
}

////////////////////////////////////////////////////////////////////////////////
info monitor::from_device(udev_device*& dev)
{
    pie::info info;

    info.path = udev_device_get_devnode(dev);

    dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_interface");

    if(auto iface = udev_device_get_sysattr_value(dev, "bInterfaceNumber"))
        info.regi.iface = static_cast<word>(std::stoi(iface));

    dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

    if(auto vid = udev_device_get_sysattr_value(dev, "idVendor"))
        info.regi.vid = static_cast<word>(std::stoi("0x" + std::string(vid), 0, 0));

    if(auto pid = udev_device_get_sysattr_value(dev, "idProduct"))
        info.regi.pid = static_cast<word>(std::stoi("0x" + std::string(pid), 0, 0));

    if(auto product = udev_device_get_sysattr_value(dev, "product"))
        info.product = product;

    return info;
}

////////////////////////////////////////////////////////////////////////////////
void monitor::enumerate()
{
    ////////////////////
    clog_(level::debug) << "Connecting to udev enumerate" << std::endl;
    auto enu = udev_enumerate_new(udev_);
    if(!enu) throw std::runtime_error("Failed to connect to udev enumerate");

    clog_(level::debug) << "Adding filter for hidraw" << std::endl;
    udev_enumerate_add_match_subsystem(enu, "hidraw");

    clog_(level::debug) << "Scanning devices" << std::endl;
    udev_enumerate_scan_devices(enu);

    clog_(level::info) <<  "Enumerating existing devices" << std::endl;
    udev_list_entry* entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(enu))
    {
        if(auto dev = udev_device_new_from_syspath(udev_, udev_list_entry_get_name(entry)))
        {
            pie::info info = from_device(dev);
            udev_device_unref(dev);

            clog_(level::info) << "Found device: " << info << std::endl;
            device_added_(info);
        }
    }

    clog_(level::debug) << "Disconnecting from udev enumerate" << std::endl;
    udev_enumerate_unref(enu);

    if(poll_)
    {
        clog_(level::info) <<  "Polling for changes" << std::endl;
        schedule_poll();
    }
    else close();
}

////////////////////////////////////////////////////////////////////////////////
void monitor::schedule_poll()
{
    using namespace std::literals::chrono_literals;

    clog_(level::trace) << "Scheduling poll" << std::endl;
    timer_.expires_from_now(0ms);
    timer_.async_wait(std::bind(&monitor::poll, this));
}

////////////////////////////////////////////////////////////////////////////////
void monitor::poll()
{
    stream_.read_some(asio::null_buffers());

    if(auto dev = udev_monitor_receive_device(monitor_))
    {
        std::string act = udev_device_get_action(dev);
        pie::info info = from_device(dev);
        udev_device_unref(dev);

        if(act == "add")
        {
            clog_(level::info) << "Device connect: " << info << std::endl;
            device_added_(info);
        }
        else if(act == "remove")
        {
            clog_(level::info) << "Device disconnect: " << info << std::endl;
            device_removed_(info);
        }
    }

    schedule_poll();
}

////////////////////////////////////////////////////////////////////////////////
}
