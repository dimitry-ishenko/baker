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
    void set_led_all(led_all::on_t);
    void set_led_bank(led_bank::index_t, led_bank::state_t);

    void set_uid(byte uid);
    void request_desc();
    void enable_timestamp(timestamp::enable_t);
    void request_data();

    void set_level_all(level_all::color_t, byte level);
    void toggle_all();
    void set_led_row(led_row::color_t, led_row::rows_t);
    void set_led(led::color_t, byte index, led::state_t);

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
