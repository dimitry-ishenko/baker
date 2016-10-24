////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "errno_error.hpp"
#include "xk_func.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
xk_func::xk_func(asio::io_service& io, const std::string& path, log::book clog) :
    stream_(io), clog_(std::move(clog))
{
    clog_(level::debug) << "Opening device: " << path << std::endl;
    auto fd = ::open(path.data(), O_RDWR);
    if(fd == -1) throw errno_error();

    stream_.assign(fd);
}

////////////////////////////////////////////////////////////////////////////////
xk_func::~xk_func() noexcept { close(); }

////////////////////////////////////////////////////////////////////////////////
void xk_func::close() noexcept
{
    clog_(level::debug) << "Closing device" << std::endl;
    asio::error_code ec;
    stream_.close(ec);
}

////////////////////////////////////////////////////////////////////////////////
void xk_func::set_leds_on(pie::leds::on_t on)
{
    clog_(level::debug) << "Sending set_leds_on" << std::endl;

    auto store = store_for<pie::set_leds_on>();
    reinterpret_cast<pie::set_leds_on*>(store.data())->on = on;

    asio::write(stream_, asio::buffer(store));
}

void xk_func::set_led(pie::led::color_t color, pie::led::state_t state)
{
    clog_(level::debug) << "Sending set_led" << std::endl;

    auto store = store_for<pie::set_led_state>();
    reinterpret_cast<pie::set_led_state*>(store.data())->color = color;
    reinterpret_cast<pie::set_led_state*>(store.data())->state = state;

    asio::write(stream_, asio::buffer(store));
}

void xk_func::set_uid(byte uid)
{
    clog_(level::debug) << "Sending set_uid" << std::endl;

    auto store = store_for<pie::set_uid>();
    reinterpret_cast<pie::set_uid*>(store.data())->uid = uid;

    asio::write(stream_, asio::buffer(store));
}

void xk_func::request_desc()
{
    clog_(level::debug) << "Sending request_desc" << std::endl;

    auto store = store_for<pie::request_desc>();
    asio::write(stream_, asio::buffer(store));
}

void xk_func::set_stamp(pie::stamp::enable_t enable)
{
    clog_(level::debug) << "Sending enable_stamp" << std::endl;

    auto store = store_for<pie::enable_stamp>();
    reinterpret_cast<pie::enable_stamp*>(store.data())->enable = enable;

    asio::write(stream_, asio::buffer(store));
}

void xk_func::request_data()
{
    clog_(level::debug) << "Sending request_data" << std::endl;

    auto store = store_for<pie::request_data>();
    asio::write(stream_, asio::buffer(store));
}

void xk_func::set_level(pie::light::color_t color, byte value)
{
    clog_(level::debug) << "Sending set_light_level" << std::endl;

    level_[color] = value;
    auto store = store_for<pie::set_light_level>();
    std::memcpy(reinterpret_cast<pie::set_light_level*>(store.data())->level, level_, sizeof(level_));

    asio::write(stream_, asio::buffer(store));
}

void xk_func::toggle_lights()
{
    clog_(level::debug) << "Sending toggle_lights" << std::endl;

    auto store = store_for<pie::toggle_lights>();
    asio::write(stream_, asio::buffer(store));
}

void xk_func::set_rows_on(pie::light::color_t color, pie::row::row_t rows)
{
    clog_(level::debug) << "Sending set_rows_on" << std::endl;

    auto store = store_for<pie::set_rows_on>();
    reinterpret_cast<pie::set_rows_on*>(store.data())->color = color;
    reinterpret_cast<pie::set_rows_on*>(store.data())->rows = rows;

    asio::write(stream_, asio::buffer(store));
}

void xk_func::set_light(pie::light::color_t color, byte index, byte total, pie::light::state_t state)
{
    clog_(level::debug) << "Sending set_light_state" << std::endl;

    auto store = store_for<pie::set_light_state>();
    reinterpret_cast<pie::set_light_state*>(store.data())->color(total) = color;
    reinterpret_cast<pie::set_light_state*>(store.data())->index(total) = index;
    reinterpret_cast<pie::set_light_state*>(store.data())->state = state;

    asio::write(stream_, asio::buffer(store));
}

void xk_func::set_freq(byte freq)
{
    clog_(level::debug) << "Sending set_freq" << std::endl;

    auto store = store_for<pie::set_freq>();
    reinterpret_cast<pie::set_freq*>(store.data())->freq = freq;

    asio::write(stream_, asio::buffer(store));
}

void xk_func::reboot()
{
    clog_(level::debug) << "Sending reboot" << std::endl;

    auto store = store_for<pie::reboot>();
    asio::write(stream_, asio::buffer(store));
}

////////////////////////////////////////////////////////////////////////////////
store xk_func::read_desc()
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
store xk_func::read_data(bool request)
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
