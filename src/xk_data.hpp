////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_XK_DATA_HPP
#define PIE_XK_DATA_HPP

#include "operators.hpp"

#include <cstddef> // std::size_t
#include <stdexcept>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using byte = uint8_t;
using word = uint16_t;

////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

namespace leds { enum on_t : byte { none, green = 0x40, red = 0x80 }; }

struct set_leds_on
{
    const byte _ = 0;
    const byte command = 186;

    leds::on_t on;
};

////////////////////////////////////////////////////////////////////////////////
namespace led
{
enum color_t : byte { green = 6, red = 7 };
enum state_t : byte { off, on, flash };
}

struct set_led_state
{
    const byte _ = 0;
    const byte command = 179;

    led::color_t color;
    led::state_t state;
};

////////////////////////////////////////////////////////////////////////////////
struct set_uid
{
    const byte _ = 0;
    const byte command = 189;

    byte uid;
};

////////////////////////////////////////////////////////////////////////////////
struct request_desc
{
    const byte _ = 0;
    const byte command = 214;
};

////////////////////////////////////////////////////////////////////////////////
namespace stamp { enum enable_t : byte { off, on }; }

struct enable_stamp
{
    const byte _ = 0;
    const byte command = 210;

    stamp::enable_t enable;
};

////////////////////////////////////////////////////////////////////////////////
struct request_data
{
    const byte _ = 0;
    const byte command = 177;
};

////////////////////////////////////////////////////////////////////////////////
namespace light
{
enum color_t : byte { blue, red, end = red };
}

struct set_light_level
{
    const byte _ = 0;
    const byte command = 187;

    byte level[light::color_t::end + 1];
};

////////////////////////////////////////////////////////////////////////////////
struct toggle_lights
{
    const byte _ = 0;
    const byte command = 184;
};

////////////////////////////////////////////////////////////////////////////////
namespace row
{
    enum row_t : byte
    {
        _0 = 0x01, _1 = 0x02, _2 = 0x04, _3 = 0x08,
        _4 = 0x10, _5 = 0x20, _6 = 0x40, _7 = 0x80
    };

    static constexpr auto none = static_cast<row_t>(0x00);
    static constexpr auto all  = static_cast<row_t>(0xff);
}

struct set_rows_on
{
    const byte _ = 0;
    const byte command = 182;

    light::color_t color;
    row::row_t rows;
};

////////////////////////////////////////////////////////////////////////////////
namespace light
{
    enum state_t : byte { off, on, flash };
}

struct set_light_state
{
    const byte _ = 0;
    const byte command = 181;

private:
    byte _index = 0;
public:
    light::state_t state;

    auto color() { return _color_helper(this); }
    auto index() { return _index_helper(this); }

    ////////////////////
    class _color_helper
    {
        set_light_state* p;
    public:
        constexpr _color_helper(set_light_state* p) : p(p) { }
        constexpr void operator=(light::color_t color)
        {
            if(color == light::blue && p->_index >= 80) p->_index -= 80;
            else if(color == light::red && p->_index < 80) p->_index += 80;
        }
    };
    friend class _color_helper;

    ////////////////////
    class _index_helper
    {
        set_light_state* p;
    public:
        constexpr _index_helper(set_light_state* p) : p(p) { }
        void operator=(byte index)
        {
            if(index >= 80) throw std::out_of_range("set_led_1::_index_helper");
            p->_index = (p->_index >= 80 ? 80 : 0) + index;
        }
    };
    friend class _index_helper;
};

////////////////////////////////////////////////////////////////////////////////
struct set_freq
{
    const byte _ = 0;
    const byte command = 180;

    byte freq;
};

////////////////////////////////////////////////////////////////////////////////
struct reboot
{
    const byte _ = 0;
    const byte command = 238;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct desc
{
    byte uid;
    byte type;

    byte _2[5];

    byte columns;
    byte rows;

    leds::on_t on;
};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
using store = std::vector<byte>;

constexpr std::size_t send_size = 36;
constexpr std::size_t max_recv_size = 48; // ???

template<typename T>
store store_for()
{
    pie::store store(send_size, 0);
    new (store.data()) T();
    return store;
}

}

////////////////////////////////////////////////////////////////////////////////
DECLARE_OPERATORS(pie::leds::on_t)
DECLARE_OPERATORS(pie::row::row_t)

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_XK_DATA_HPP
