////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_FUNCTIONS_HPP
#define PIE_FUNCTIONS_HPP

#include "operators.hpp"

#include <asio.hpp>
#include <cstddef> // std::size_t
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using byte = uint8_t;
using word = uint16_t;

////////////////////////////////////////////////////////////////////////////////
namespace leds
{
    enum on_t : byte { none, green = 0x40, red = 0x80 };
}

namespace led
{
    enum color_t : byte { green = 6, red = 7 };
    enum state_t : byte { off, on, flash };
}

namespace stamp
{
    enum enable_t : byte { off, on };
}

namespace light
{
    enum color_t : byte { blue, red, end = red };
    enum state_t : byte { off, on, flash };
}

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

////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

struct desc
{
    byte uid;
    byte type;

    byte mode;
    byte misc[4];

    byte columns;
    byte rows;

    leds::on_t on;

    byte version;
    word pid;

    byte _[24];
};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class functions
{
public:
    ////////////////////
    functions(asio::io_service&, const std::string& path);
    ~functions() { close(); }

    void close();

    ////////////////////
    void set_leds_on(pie::leds::on_t);
    void set_led(pie::led::color_t, pie::led::state_t);

    void set_uid(byte uid);
    void request_desc();
    void set_stamp(pie::stamp::enable_t);
    void request_data();

    void set_level(pie::light::color_t, byte level);

    void toggle_lights();
    void set_rows_on(pie::light::color_t, pie::row::row_t);
    void set_light(pie::light::color_t, byte index, byte total, pie::light::state_t);

    void set_freq(byte freq);
    void reboot();

    ////////////////////
    pie::desc read_desc();
    std::vector<byte> read_data();

protected:
    ////////////////////
    asio::posix::stream_descriptor stream_;

    // used by set level command
    byte level_[pie::light::color_t::end + 1] { };
};

}

////////////////////////////////////////////////////////////////////////////////
DECLARE_OPERATORS(pie::leds::on_t)
DECLARE_OPERATORS(pie::row::row_t)

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_FUNCTIONS_HPP
