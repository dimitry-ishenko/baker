////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_MONITOR_HPP
#define PIE_MONITOR_HPP

#include "sig/signal.hpp"

#include <asio.hpp>
#include <asio/system_timer.hpp>
#include <chrono>

#include <libudev.h>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class monitor
{
public:
    ////////////////////
    explicit monitor(asio::io_service& io);
    ~monitor() noexcept { close(); }

    void close() noexcept;

    sig::signal_proxy<void(const std::string&)>& device_added() { return device_added_; }
    sig::signal_proxy<void(const std::string&)>& device_removed() { return device_removed_; }

private:
    ////////////////////
    asio::system_timer timer_;

    udev* udev_ = nullptr;
    udev_monitor* monitor_ = nullptr;

    void enumerate();

    void poll();
    bool first_poll = true;

    sig::signal<void(const std::string&)> device_added_;
    sig::signal<void(const std::string&)> device_removed_;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_MONITOR_HPP
