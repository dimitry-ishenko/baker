////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "proc/process.hpp"
#include "xk_base.hpp"

#include <climits> // CHAR_BIT
#include <iomanip>

using log::level;
using namespace std::literals::chrono_literals;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
xk_base::xk_base(asio::io_service& io, const std::string& path, log::book clog) :
    xk_func(io, path, std::move(clog)),
    timer_(io), name_(path)
{
    auto p = name_.find_last_of('/');
    if(p != std::string::npos) name_.erase(0, p + 1);

    auto store = read_desc();
    auto desc = reinterpret_cast<pie::desc*>(store.data());

    uid_ = desc->uid;
    columns_ = desc->columns;
    rows_ = desc->rows;
    total_ = columns_ * CHAR_BIT;

    prev_.resize(columns_);

    using namespace std;
    clog_(level::info) << device_id()
                       << ": unit id " << uid_
                       << ", size " << columns_ << " x " << rows_
                       << ", version " << static_cast<int>(desc->version)
                       << ", pid " << hex << setfill('0') << setw(4) << desc->pid
                       << endl;

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
void xk_base::close()
{
    clog_(level::info) << device_id() << ": closing" << std::endl;

    clog_(level::debug) << "Cancelling read" << std::endl;
    asio::error_code ec;
    timer_.cancel(ec);

    set_leds_on(leds::none);
    set_rows_on(light::blue, row::none);
    set_rows_on(light::red, row::none);

    xk_func::close();
}

////////////////////////////////////////////////////////////////////////////////
void xk_base::read()
{
    auto store = read_data();
    if(store.size() + 2 < columns_) throw std::invalid_argument("Input short read");

    ////////////////////
    auto pr = process_read(store);

    ////////////////////
    for(auto index : std::get<0>(pr))
    {
        if(index == pie::prog)
        {
            set_led(led::red, led::on);
            pressed_(index);
        }
        else
        {
            if(pending_ != index && pending_ != none)
            {
                set_light(light::blue, pending_, total_, light::on);
                set_light(light::red, pending_, total_, light::off);
                pending_ = none;
            }

            if(pending_ == index || !critical_.count(index))
            {
                pending_ = none;
                set_light(light::blue, index, total_, light::off);
                set_light(light::red, index, total_, light::on);
                pressed_(index);
            }
            else
            {
                pending_ = index;
                set_light(light::blue, index, total_, light::off);
                set_light(light::red, index, total_, light::flash);
            }
        }
    }
    for(auto index : std::get<1>(pr))
    {
        if(index == pie::prog)
        {
            set_led(led::red, led::off);
            released_(index);
        }
        else if(pending_ != index)
        {
            set_light(light::red, index, total_, light::off);
            set_light(light::blue, index, total_, light::on);
            released_(index);
        }
    }

    // toggle lock on PS release
    if(std::get<1>(pr).count(pie::prog))
    {
        pending_ = none;

        lock_ = !lock_;
        set_rows_on(light::blue, lock_ ? row::none : row::all);
        set_rows_on(light::red, lock_ ? row::all : row::none);

        locked_(lock_);
    }

    schedule_read();
}

////////////////////////////////////////////////////////////////////////////////
void xk_base::schedule_read()
{
    clog_(level::debug) << "Scheduling read" << std::endl;
    timer_.expires_from_now(0s);
    timer_.async_wait(std::bind(&xk_base::read, this));
}

////////////////////////////////////////////////////////////////////////////////
xk_base::press_release xk_base::process_read(const store& store)
{
    set press, release;

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
        byte on  = !lock_ ? *ri & ~prev_[c] : 0;
        byte off =         ~*ri &  prev_[c];
        prev_[c] = !lock_ ? *ri :  prev_[c] & *ri;

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
