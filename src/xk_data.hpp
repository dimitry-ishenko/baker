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

struct led_all
{
    const byte _ = 0;
    const byte command = 186;

    enum on_t : byte { none, blue = 0x40, red = 0x80 } on;
};

////////////////////////////////////////////////////////////////////////////////
struct led_bank
{
    const byte _ = 0;
    const byte command = 179;

    enum index_t : byte { blue = 6, red = 7 } index;
    enum state_t : byte { off, on, flash } state;
};

////////////////////////////////////////////////////////////////////////////////
struct uid
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
struct timestamp
{
    const byte _ = 0;
    const byte command = 210;

    enum enable_t : byte { off, on } enable;
};

////////////////////////////////////////////////////////////////////////////////
struct request_data
{
    const byte _ = 0;
    const byte command = 177;
};

////////////////////////////////////////////////////////////////////////////////
struct level_all
{
    const byte _ = 0;
    const byte command = 187;

    enum color_t : byte { blue, red, size };
    byte level[color_t::size];
};

////////////////////////////////////////////////////////////////////////////////
struct toggle_all
{
    const byte _ = 0;
    const byte command = 184;
};

////////////////////////////////////////////////////////////////////////////////
struct led_row
{
    const byte _ = 0;
    const byte command = 182;

    enum color_t : byte { blue, red } color;
    enum rows_t : byte
    {
        row_0 = 0x01, row_1 = 0x02, row_2 = 0x04, row_3 = 0x08,
        row_4 = 0x10, row_5 = 0x20, row_6 = 0x40, row_7 = 0x80
    }
    rows;
};

////////////////////////////////////////////////////////////////////////////////
struct led
{
    const byte _ = 0;
    const byte command = 181;

private:
    byte _index = 0;
public:
    enum state_t : byte { off, on, flash } state;

    enum color_t : byte { blue, red };
    auto color() { return _color_helper(this); }
    auto index() { return _index_helper(this); }

    ////////////////////
    class _color_helper
    {
        led* p;
    public:
        constexpr _color_helper(led* p) : p(p) { }
        constexpr void operator=(color_t color)
        {
            if(color == blue && p->_index >= 80) p->_index -= 80;
            else if(color == red && p->_index < 80) p->_index += 80;
        }
    };
    friend class _color_helper;

    ////////////////////
    class _index_helper
    {
        led* p;
    public:
        constexpr _index_helper(led* p) : p(p) { }
        void operator=(byte index)
        {
            if(index >= 80) throw std::out_of_range("set_led_1::_index_helper");
            p->_index = (p->_index >= 80 ? 80 : 0) + index;
        }
    };
    friend class _index_helper;
};

////////////////////////////////////////////////////////////////////////////////
struct freq
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
    byte _;

    byte uid;
    byte type;

    byte _2[5];

    byte columns;
    byte rows;

    enum on_t : byte { none, blue = 0x40, red = 0x80 } on;
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
DECLARE_OPERATORS(pie::led_all::on_t)
DECLARE_OPERATORS(pie::led_row::rows_t)
DECLARE_OPERATORS(pie::desc::on_t)

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_XK_DATA_HPP
