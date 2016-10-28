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
    timer_(io), name_(path), func_(io, path), clog_(std::move(clog))
{
    auto p = name_.find_last_of('/');
    if(p != std::string::npos) name_.erase(0, p + 1);

    auto store = func_.read_desc();
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
                       << dec << endl;

    ////////////////////
    func_.set_leds_on(leds::none);

    func_.set_level(light::blue, 128);
    func_.set_rows_on(light::blue, row::all);

    func_.set_level(light::red, 255);
    func_.set_rows_on(light::red, row::none);

    ////////////////////
    func_.request_data();
    schedule_read();
}

////////////////////////////////////////////////////////////////////////////////
void xk_base::close()
{
    clog_(level::info) << device_id() << ": closing" << std::endl;

    clog_(level::debug) << "Cancelling read" << std::endl;
    asio::error_code ec;
    timer_.cancel(ec);

    func_.set_leds_on(leds::none);
    func_.set_rows_on(light::blue, row::none);
    func_.set_rows_on(light::red, row::none);

    func_.close();
}

////////////////////////////////////////////////////////////////////////////////
void xk_base::read()
{
    auto store = func_.read_data();
    if(store.size() + 2 < columns_) throw std::invalid_argument("Input short read");

    ////////////////////
    auto pr = process_read(store);

    ////////////////////
    // process presses
    for(auto index : std::get<0>(pr))
    {
        if(index == pie::prog)
        {
            func_.set_led(led::red, led::on);
            pressed_(index);
            clog_(level::debug) << "Pressed PS" << std::endl;
        }
        else
        {
            // reset pending button, when a different
            // button is pressed
            if(pending_ != index && pending_ != none)
            {
                func_.set_light(light::blue, pending_, total_, light::on);
                func_.set_light(light::red, pending_, total_, light::off);
                pending_ = none;
            }

            // this is a non-critical button being pressed once
            // or a critical button being pressed second time;
            // send pressed event
            if(pending_ == index || !critical_.count(index))
            {
                pending_ = none;
                func_.set_light(light::blue, index, total_, light::off);
                func_.set_light(light::red, index, total_, light::on);
                pressed_(index);
                clog_(level::debug) << "Pressed " << index << std::endl;
            }

            // this is a critical button being pressed once;
            // make it flash
            else
            {
                pending_ = index;
                func_.set_light(light::blue, index, total_, light::off);
                func_.set_light(light::red, index, total_, light::flash);
            }
        }
    }

    // process releases
    for(auto index : std::get<1>(pr))
    {
        if(index == pie::prog)
        {
            func_.set_led(led::red, led::off);
            released_(index);
            clog_(level::debug) << "Released PS" << std::endl;
        }
        else if(index != pending_)
        {
            // when in locked mode, leave the button red
            if(!lock_)
            {
                func_.set_light(light::red, index, total_, light::off);
                func_.set_light(light::blue, index, total_, light::on);
            }
            released_(index);
            clog_(level::debug) << "Released " << index << std::endl;
        }
    }

    // reset pending and toggle lock mode
    // on PS switch release
    if(std::get<1>(pr).count(pie::prog))
    {
        pending_ = none;

        lock_ = !lock_;
        func_.set_rows_on(light::blue, lock_ ? row::none : row::all);
        func_.set_rows_on(light::red, lock_ ? row::all : row::none);

        locked_(lock_);
        clog_(level::debug) << (lock_ ? "Locked " : "Unlocked") << std::endl;
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
xk_base::press_release xk_base::process_read(const std::vector<byte>& store)
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
        // when locked, don't allow new button presses
        // and only let the buttons that have been
        // pressed before the lock to be released
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
