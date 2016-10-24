////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "errno_error.hpp"
#include "xk_base.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
XK_base::XK_base(asio::io_service& io, const std::string& path, log::book clog) :
    stream_(io), clog_(std::move(clog))
{
    clog_(level::debug) << "Opening device: " << path << std::endl;
    auto fd = ::open(path.data(), O_RDWR);
    if(fd == -1) throw errno_error();

    stream_.assign(fd);
}

////////////////////////////////////////////////////////////////////////////////
XK_base::~XK_base() noexcept { close(); }

////////////////////////////////////////////////////////////////////////////////
void XK_base::close() noexcept
{
    clog_(level::debug) << "Closing device" << std::endl;
    asio::error_code ec;
    stream_.close(ec);
}

////////////////////////////////////////////////////////////////////////////////
void XK_base::set_led_all(led_all::on_t on)
{
    clog_(level::debug) << "Sending set_led_all" << std::endl;

    auto store = store_for<led_all>();
    reinterpret_cast<led_all*>(store.data())->on = on;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::set_led_bank(led_bank::index_t index, led_bank::state_t state)
{
    clog_(level::debug) << "Sending set_led_bank" << std::endl;

    auto store = store_for<led_bank>();
    reinterpret_cast<led_bank*>(store.data())->index = index;
    reinterpret_cast<led_bank*>(store.data())->state = state;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::set_uid(byte uid)
{
    clog_(level::debug) << "Sending set_uid" << std::endl;

    auto store = store_for<pie::uid>();
    reinterpret_cast<pie::uid*>(store.data())->uid = uid;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::request_desc()
{
    clog_(level::debug) << "Sending request_desc" << std::endl;

    auto store = store_for<pie::request_desc>();
    asio::write(stream_, asio::buffer(store));
}

void XK_base::enable_timestamp(timestamp::enable_t en)
{
    clog_(level::debug) << "Sending enable_timestamp" << std::endl;

    auto store = store_for<timestamp>();
    reinterpret_cast<timestamp*>(store.data())->enable = en;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::request_data()
{
    clog_(level::debug) << "Sending request_data" << std::endl;

    auto store = store_for<pie::request_data>();
    asio::write(stream_, asio::buffer(store));
}

void XK_base::set_level_all(level_all::color_t color, byte value)
{
    clog_(level::debug) << "Sending set_level_all" << std::endl;

    auto store = store_for<level_all>();
    reinterpret_cast<level_all*>(store.data())->level[color] = value;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::toggle_all()
{
    clog_(level::debug) << "Sending toggle_all" << std::endl;

    auto store = store_for<pie::toggle_all>();
    asio::write(stream_, asio::buffer(store));
}

void XK_base::set_led_row(led_row::color_t color, led_row::rows_t rows)
{
    clog_(level::debug) << "Sending set_led_row" << std::endl;

    auto store = store_for<led_row>();
    reinterpret_cast<led_row*>(store.data())->color = color;
    reinterpret_cast<led_row*>(store.data())->rows = rows;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::set_led(led::color_t color, byte index, led::state_t state)
{
    clog_(level::debug) << "Sending set_led" << std::endl;

    auto store = store_for<led>();
    reinterpret_cast<led*>(store.data())->color() = color;
    reinterpret_cast<led*>(store.data())->index() = index;
    reinterpret_cast<led*>(store.data())->state = state;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::set_freq(byte freq)
{
    clog_(level::debug) << "Sending set_freq" << std::endl;

    auto store = store_for<pie::freq>();
    reinterpret_cast<pie::freq*>(store.data())->freq = freq;

    asio::write(stream_, asio::buffer(store));
}

void XK_base::reboot()
{
    clog_(level::debug) << "Sending reboot" << std::endl;

    auto store = store_for<pie::reboot>();
    asio::write(stream_, asio::buffer(store));
}

////////////////////////////////////////////////////////////////////////////////
store XK_base::read_desc()
{
    request_desc();

    clog_(level::debug) << "Receiving read_desc" << std::endl;

    pie::store desc(max_recv_size, 0);
    auto size = stream_.read_some(asio::buffer(desc));
    if(size < sizeof(pie::desc)) throw std::invalid_argument("Descriptor short read");
    desc.resize(size);

    return desc;
}

////////////////////////////////////////////////////////////////////////////////
store XK_base::read_data(bool request)
{
    if(request) request_data();

    clog_(level::debug) << "Receiving read_data" << std::endl;

    pie::store data(max_recv_size, 0);
    auto size = stream_.read_some(asio::buffer(data));
    data.resize(size);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
}
