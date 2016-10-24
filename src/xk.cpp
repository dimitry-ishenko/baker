////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "xk.hpp"
#include <climits> // CHAR_BIT

using log::level;
using namespace std::literals::chrono_literals;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
XK::XK(asio::io_service& io, const std::string& path, log::book clog) :
    XK_base(io, path, std::move(clog)),
    timer_(io)
{
    clog_(level::debug) << "Initializing" << std::endl;

    auto store = read_desc();
    auto desc = reinterpret_cast<pie::desc*>(store.data());

    uid_ = desc->uid;
    columns_ = desc->columns;
    rows_ = desc->rows;
    total_ = columns_ * CHAR_BIT;

    prev_.reset(new byte[columns_]);

    clog_(level::debug) << "Detected device:"
                        << " uid=" << uid_ << " columns=" << columns_ << " rows=" << rows_
                        << std::endl;

    ////////////////////
    set_leds_on(leds::none);

    set_level(light::blue, 128);
    set_rows_on(light::blue, row::all);

    set_level(light::red, 255);
    set_rows_on(light::red, row::none);

    ////////////////////
    request_data();
    schedule_read();
}

////////////////////////////////////////////////////////////////////////////////
void XK::read()
{
    auto store = read_data();
    if(store.size() + 2 < columns_) throw std::invalid_argument("Input short read");

    auto pr = process_read(store);

    for(auto index : std::get<0>(pr))
    {
        if(index != pie::prog)
        {
            set_light(light::blue, index, total_, light::off);
            set_light(light::red, index, total_, light::on);
        }
        else set_led(led::red, led::on);

        pressed_(index);
    }
    for(auto index : std::get<1>(pr))
    {
        if(index != pie::prog)
        {
            set_light(light::red, index, total_, light::off);
            set_light(light::blue, index, total_, light::on);
        }
        else set_led(led::red, led::off);

        released_(index);
    }

    schedule_read();
}

////////////////////////////////////////////////////////////////////////////////
void XK::schedule_read()
{
    clog_(level::debug) << "Scheduling read" << std::endl;
    timer_.expires_from_now(0s);
    timer_.async_wait(std::bind(&XK::read, this));
}

////////////////////////////////////////////////////////////////////////////////
std::tuple<XK::press, XK::release> XK::process_read(const store& store)
{
    XK::press press;
    XK::release release;

    bool ps = store[1] & 0x01;
    if(ps != ps_)
    {
        if(ps) press.insert(pie::prog);
        else release.insert(pie::prog);
        ps_ = ps;
    }

    ////////////////////
    index_t index = 0;

    auto ri = store.begin() + 2;
    for(std::size_t c = 0; c < columns_; ++c, ++ri)
    {
        byte on  =  *ri & ~prev_[c];
        byte off = ~*ri &  prev_[c];
        prev_[c] =  *ri;

        for(std::size_t r = 0; r < rows_; ++r)
        {
            if(on & 1) press.insert(index);
            on >>= 1;

            if(off & 1) release.insert(index);
            off >>= 1;

            ++index;
        }
        index += CHAR_BIT - rows_;
    }

    return std::make_tuple(press, release);
}

////////////////////////////////////////////////////////////////////////////////
}
