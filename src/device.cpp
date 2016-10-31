////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "device.hpp"

#include <chrono>
#include <climits> // CHAR_BIT
#include <iomanip>

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
device::device(asio::io_service& io, const pie::params& params, const pie::info& info, log::book clog) :
    closing(io), name_(info.product), clog_(std::move(clog)), timer_(io), func_(io, info.path), ignore_(params.ignore)
{
    name_ += " on " + info.path;

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
void device::close() noexcept
{
    if(func_.is_open())
    {
        clog_(level::info) << "Closing " << name_ << std::endl;

        clog_(level::debug) << "Cancelling read" << std::endl;
        asio::error_code ec;
        timer_.cancel(ec);

        try
        {
            func_.set_level(light::red, 0, light::fade);
            func_.set_level(light::blue, 0, light::fade);
            func_.set_leds_on(leds::none);
        }
        catch(...) { }

        func_.close();
    }
}

////////////////////////////////////////////////////////////////////////////////
void device::read()
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
void device::schedule_read()
{
    using namespace std::literals::chrono_literals;

    clog_(level::trace) << "Scheduling read" << std::endl;
    timer_.expires_from_now(0ms);
    timer_.async_wait(std::bind(&device::read, this));
}

////////////////////////////////////////////////////////////////////////////////
device::press_release device::process_read(const std::vector<byte>& data)
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
        // and only let the buttons pressed before the lock to be released
        byte on  = !lock_ ? *ri & ~prev_[c] : 0;
        byte off =         ~*ri &  prev_[c];
        prev_[c] = !lock_ ? *ri :  prev_[c] & *ri;

        for(byte r = 0; r < rows_; ++r)
        {
            if(!ignore_.count(index) && (on & 1)) press.insert(index);
            on >>= 1;

            if(!ignore_.count(index) && (off & 1)) release.insert(index);
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
    return static_cast<index_t>((index & 0x07) << 2)
         | static_cast<index_t>((index >> 3) & 0x03);
}

////////////////////////////////////////////////////////////////////////////////
device_XK16::press_release device_XK16::process_read(const std::vector<byte>& data)
{
    press_release from = device::process_read(data), to;

    for(auto index : std::get<press>(from)) std::get<press>(to).insert(mapped(index));
    for(auto index : std::get<release>(from)) std::get<release>(to).insert(mapped(index));

    return to;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
device_jog::press_release device_jog::process_read(const std::vector<byte>& data)
{
    if(data.size() - 2 < off_) throw std::invalid_argument("Input short read (jog)");

    auto dir = static_cast<dir_t>(data[off_]);
    if(!lock_ && dir)
    {
        jog_(dir);
        clog_(level::debug) << "Jog " << (dir == ccw ? "CCW" : "CW") << std::endl;
    }

    auto speed = static_cast<speed_t>(data[off_ + 1]);
    if(!lock_ && speed != speed_)
    {
        shuttle_(speed_ = speed);
        clog_(level::debug) << "Shuttle " << static_cast<int>(speed_) << std::endl;
    }

    return device::process_read(data);
}

////////////////////////////////////////////////////////////////////////////////
}
