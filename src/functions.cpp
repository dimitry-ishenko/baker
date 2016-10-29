////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "errno_error.hpp"
#include "functions.hpp"

#include <array>
#include <chrono>
#include <cstring> // std::memcpy
#include <stdexcept>
#include <thread>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
constexpr std::size_t send_size = 36;

#define PADDING_0 byte _[send_size - sizeof(id) - sizeof(command)] { };
#define PADDING_1(one) byte _[send_size - sizeof(id) - sizeof(command) - sizeof(one)] { };
#define PADDING_2(one, two) byte _[send_size - sizeof(id) - sizeof(command) - sizeof(one) - sizeof(two)] { };

#define ASSERT_SIZE(name) static_assert(sizeof(name) == send_size, "Incorrect size of " # name);

#pragma pack(push, 1)

struct set_leds_on
{
    const byte id = 0;
    const byte command = 186;
    leds::on_t on;
    PADDING_1(on)
};
ASSERT_SIZE(set_leds_on)

struct set_led_state
{
    const byte id = 0;
    const byte command = 179;
    led::color_t color;
    led::state_t state;
    PADDING_2(color, state)
};
ASSERT_SIZE(set_led_state)

struct set_uid
{
    const byte id = 0;
    const byte command = 189;
    byte uid;
    PADDING_1(uid)
};
ASSERT_SIZE(set_uid)

struct request_desc
{
    const byte id = 0;
    const byte command = 214;
    PADDING_0
};
ASSERT_SIZE(request_desc)

struct enable_stamp
{
    const byte id = 0;
    const byte command = 210;
    stamp::enable_t enable;
    PADDING_1(enable)
};
ASSERT_SIZE(enable_stamp)

struct request_data
{
    const byte id = 0;
    const byte command = 177;
    PADDING_0
};
ASSERT_SIZE(request_data)

struct set_light_level
{
    const byte id = 0;
    const byte command = 187;
    byte level[light::color_t::size];
    PADDING_1(level)
};
ASSERT_SIZE(set_light_level)

struct toggle_lights
{
    const byte id = 0;
    const byte command = 184;
    PADDING_0
};
ASSERT_SIZE(toggle_lights)

struct set_rows_on
{
    const byte id = 0;
    const byte command = 182;
    light::color_t color;
    row::row_t rows;
    PADDING_2(color, rows)
};
ASSERT_SIZE(set_rows_on)

struct set_light_state
{
    const byte id = 0;
    const byte command = 181;
private:
    byte _index = 0;
public:
    light::state_t state;
    PADDING_2(_index, state)

    auto color(byte total) { return _color_helper(this, total); }
    auto index(byte total) { return _index_helper(this, total); }

    ////////////////////
    class _color_helper
    {
        set_light_state* p;
        byte total;
    public:
        constexpr _color_helper(set_light_state* p, byte total) : p(p), total(total) { }
        void operator=(light::color_t color)
        {
            if(color == light::blue && p->_index >= total) p->_index -= total;
            else if(color == light::red && p->_index < total) p->_index += total;
        }
    };
    friend class _color_helper;

    ////////////////////
    class _index_helper
    {
        set_light_state* p;
        byte total;
    public:
        constexpr _index_helper(set_light_state* p, byte total) : p(p), total(total) { }
        void operator=(byte index)
        {
            if(index >= total) throw std::out_of_range("set_led_state::_index_helper: index out of range");
            p->_index = (p->_index >= total ? total : 0) + index;
        }
    };
    friend class _index_helper;
};
ASSERT_SIZE(set_light_state)

struct set_freq
{
    const byte id = 0;
    const byte command = 180;
    byte freq;
    PADDING_1(freq)
};
ASSERT_SIZE(set_freq)

struct reboot
{
    const byte id = 0;
    const byte command = 238;
    PADDING_0
};
ASSERT_SIZE(reboot)

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
functions::functions(asio::io_service& io, const std::string& path) :
    stream_(io)
{
    auto fd = ::open(path.data(), O_RDWR);
    if(fd == -1) throw errno_error();

    stream_.assign(fd);
}

////////////////////////////////////////////////////////////////////////////////
void functions::close()
{
    asio::error_code ec;
    stream_.close(ec);
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_leds_on(pie::leds::on_t on)
{
    pie::set_leds_on data;
    data.on = on;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_led(pie::led::color_t color, pie::led::state_t state)
{
    pie::set_led_state data;
    data.color = color;
    data.state = state;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_uid(byte uid)
{
    pie::set_uid data;
    data.uid = uid;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::request_desc()
{
    pie::request_desc data;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_stamp(pie::stamp::enable_t enable)
{
    pie::enable_stamp data;
    data.enable = enable;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::request_data()
{
    pie::request_data data;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_level(pie::light::color_t color, byte value)
{
    level_[color] = value;
    pie::set_light_level data;
    std::memcpy(data.level, level_, sizeof(level_));
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_level(pie::light::color_t color, byte value, light::fade_t)
{
    byte step = (value > level_[color]) - (level_[color] > value);

    for(byte level = level_[color]; level != value; level += step)
    {
        set_level(color, level);
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }
}

////////////////////////////////////////////////////////////////////////////////
void functions::toggle_lights()
{
    pie::toggle_lights data;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_rows_on(pie::light::color_t color, pie::row::row_t rows)
{
    pie::set_rows_on data;
    data.color = color;
    data.rows = rows;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_light(pie::light::color_t color, byte index, byte total, pie::light::state_t state)
{
    pie::set_light_state data;
    data.color(total) = color;
    data.index(total) = index;
    data.state = state;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::set_freq(byte freq)
{
    pie::set_freq data;
    data.freq = freq;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
void functions::reboot()
{
    pie::reboot data;
    asio::write(stream_, asio::buffer(&data, sizeof(data)));
}

////////////////////////////////////////////////////////////////////////////////
pie::desc functions::read_desc()
{
    request_desc();

    pie::desc desc;
    auto size = stream_.read_some(asio::buffer(&desc, sizeof(desc)));
    if(size < sizeof(desc) - sizeof(desc._)) throw std::invalid_argument("Descriptor short read");

    return desc;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<byte> functions::read_data()
{
    std::vector<byte> data(48); // arbitrary
    auto size = stream_.read_some(asio::buffer(data));
    data.resize(size);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
}
