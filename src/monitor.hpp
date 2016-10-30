////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_MONITOR_HPP
#define PIE_MONITOR_HPP

#include "closing.hpp"
#include "log/book.hpp"
#include "sig/signal.hpp"
#include "types.hpp"

#include <asio.hpp>
#include <asio/system_timer.hpp>

#include <libudev.h>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class monitor : public closing
{
public:
    ////////////////////
    explicit monitor(asio::io_service& io, log::book);
    virtual ~monitor() noexcept { close(); }

    void close() noexcept override;

    sig::signal_proxy<void(const info&)>& device_added() { return device_added_; }
    sig::signal_proxy<void(const info&)>& device_removed() { return device_removed_; }

private:
    ////////////////////
    asio::system_timer timer_;
    log::book clog_;

    udev* udev_ = nullptr;
    udev_monitor* monitor_ = nullptr;

    asio::posix::stream_descriptor stream_;

    void enumerate();

    void schedule_poll();
    void poll();

    sig::signal<void(const info&)> device_added_;
    sig::signal<void(const info&)> device_removed_;

    info from_device(udev_device*&);
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_MONITOR_HPP
