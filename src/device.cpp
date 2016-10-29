////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "proc/process.hpp"
#include "device.hpp"

#include <chrono>
#include <climits> // CHAR_BIT
#include <iomanip>
#include <thread>
#include <type_traits>

using log::level;
using namespace std::literals::chrono_literals;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
XK_device::XK_device(asio::io_service& io, byte, std::string name, const std::string& path, log::book clog) :
    name_(std::move(name)), clog_(std::move(clog)), timer_(io), func_(io, path)
{
    name += " on " + path;

    auto desc = func_.read_desc();

    uid_ = desc.uid;
    columns_ = desc.columns;
    rows_ = desc.rows;
    total_ = columns_ * CHAR_BIT;

    prev_.resize(columns_);

    using namespace std;
    clog_(level::info) << name_ << ": unit id " << int(uid_)
                                << ", size " << int(columns_) << " x " << int(rows_)
                                << ", version " << int(desc.version)
                                << ", pid " << setfill('0') << setw(4) << hex << desc.pid << dec
                                << endl;

    ////////////////////
    func_.set_leds_on(leds::none);

    func_.set_rows_on(light::red, row::none);
    func_.set_level(light::red, 255);

    func_.set_level(light::blue, 0);
    func_.set_rows_on(light::blue, row::all);
    func_.set_level(light::blue, 255, light::fade);

    func_.set_freq(10);

    ////////////////////
    func_.request_data();
    schedule_read();
}

////////////////////////////////////////////////////////////////////////////////
void XK_device::close()
{
    clog_(level::info) << name_ << ": closing" << std::endl;

    clog_(level::debug) << "Cancelling read" << std::endl;
    asio::error_code ec;
    timer_.cancel(ec);

    func_.set_level(light::red, 0, light::fade);
    func_.set_level(light::blue, 0, light::fade);
    func_.set_leds_on(leds::none);

    func_.close();
}

////////////////////////////////////////////////////////////////////////////////
void XK_device::read()
{
    auto data = func_.read_data();
    if(data.size() - 2 < columns_) throw std::invalid_argument("Input short read");

    ////////////////////
    auto press_release = process_read(data);

    ////////////////////
    // process presses
    for(auto index : std::get<press>(press_release))
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
                clog_(level::debug) << "Pressed " << int(index) << std::endl;
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
    for(auto index : std::get<release>(press_release))
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
            clog_(level::debug) << "Released " << int(index) << std::endl;
        }
    }

    // reset pending and toggle lock mode
    // on PS switch release
    if(std::get<release>(press_release).count(pie::prog))
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
void XK_device::schedule_read()
{
    clog_(level::trace) << "Scheduling read" << std::endl;
    timer_.expires_from_now(0ms);
    timer_.async_wait(std::bind(&XK_device::read, this));
}

////////////////////////////////////////////////////////////////////////////////
XK_device::press_release XK_device::process_read(const std::vector<byte>& data)
{
    buttons press, release;

    // NB: 1st byte is uid, 2nd byte is PS
    bool ps = data[1] & 0x01;
    if(ps != ps_)
    {
        if(ps) press.insert(pie::prog);
        else release.insert(pie::prog);
        ps_ = ps;
    }

    ////////////////////
    index_t index = 0;

    auto ri = data.begin() + 2;
    for(byte c = 0; c < columns_; ++c, ++ri)
    {
        // when locked, don't allow new button presses
        // and only let the buttons that have been
        // pressed before the lock to be released
        byte on  = !lock_ ? *ri & ~prev_[c] : 0;
        byte off =         ~*ri &  prev_[c];
        prev_[c] = !lock_ ? *ri :  prev_[c] & *ri;

        for(byte r = 0; r < rows_; ++r)
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
////////////////////////////////////////////////////////////////////////////////
static inline index_t mapped(index_t index)
{
    return ((index & 0x07) << 2) | ((index >> 3) & 0x03);
}

////////////////////////////////////////////////////////////////////////////////
XK16_device::press_release XK16_device::process_read(const std::vector<byte>& data)
{
    press_release from = XK16_device::process_read(data), to;

    for(auto index : std::get<press>(from)) std::get<press>(to).insert(mapped(index));
    for(auto index : std::get<release>(from)) std::get<release>(to).insert(mapped(index));

    return to;
}

////////////////////////////////////////////////////////////////////////////////
}
