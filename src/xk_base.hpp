////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_XK_BASE_HPP
#define PIE_XK_BASE_HPP

#include "log/book.hpp"
#include "xk_data.hpp"

#include <asio.hpp>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class XK_base
{
public:
    ////////////////////
    XK_base(asio::io_service& io, const std::string& path, log::book = log::book());
    virtual ~XK_base() noexcept;

    void close() noexcept;

protected:
    ////////////////////
    void set_leds_on(pie::leds::on_t);
    void set_led(pie::led::color_t, pie::led::state_t);

    void set_uid(byte uid);
    void request_desc();
    void set_stamp(pie::stamp::enable_t);
    void request_data();

    void set_level(pie::light::color_t, byte level);
    byte level_[pie::light::color_t::end + 1] { };

    void toggle_lights();
    void set_rows_on(pie::light::color_t, pie::row::row_t);
    void set_light(pie::light::color_t, byte index, pie::light::state_t);

    void set_freq(byte freq);
    void reboot();

    ////////////////////
    store read_desc();
    store read_data(bool request = false);

    ////////////////////
    asio::posix::stream_descriptor stream_;
    log::book clog_;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_XK_BASE_HPP
