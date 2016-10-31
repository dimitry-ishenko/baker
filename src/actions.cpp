////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "actions.hpp"
#include "pgm/args.hpp"

#include <fstream>
#include <vector>

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

static inline byte stob(const std::string& value)
{
    return static_cast<byte>(std::stoi(value));
}

////////////////////////////////////////////////////////////////////////////////
actions::actions(const std::string& conf, device& device, log::book clog) :
    clog_(clog)
{
    clog_(level::info) << "Reading file " << conf << std::endl;
    std::fstream fs(conf, std::ios_base::in);
    if(!fs.is_open()) throw std::fstream::failure("Failed to open file");

    auto sec = std::to_string(device.uid()) + '-';
    std::vector<pgm::arg> args;

    ////////////////////
    args.emplace_back(sec + "freq", [this, &device](std::string s)
    {
        clog_(level::debug) << "Setting freq to " << s << std::endl;
        device.set_freq(stob(s));
        return true;
    }, "");

    args.emplace_back(sec + "red" , [this, &device](std::string s)
    {
        clog_(level::debug) << "Setting red level to " << s << std::endl;
        device.set_level(light::red, stob(s));
        return true;
    }, "");

    args.emplace_back(sec + "blue", [this, &device](std::string s)
    {
        clog_(level::debug) << "Setting blue level to " << s << std::endl;
        device.set_level(light::blue, stob(s));
        return true;
    }, "");

    ////////////////////
    for(byte index = 0; index < device.total(); ++index)
    {
        auto name = sec + std::to_string(index);
        args.emplace_back(name, [this, index](std::string s)
        {
            clog_(level::debug) << "Adding action: " << int(index) << " = " << s << std::endl;
            indexes_.emplace(index, std::move(s));
            return true;
        }, "");

        name += '*';
        args.emplace_back(name, [this, index, &device](std::string s)
        {
            clog_(level::debug) << "Adding action: " << int(index) << "* = " << s << std::endl;
            indexes_.emplace(index, std::move(s));
            device.critical(index);
            return true;
        }, "");
    }

    ////////////////////
    args.emplace_back(sec + "ccw", [this](std::string s)
    {
        clog_(level::debug) << "Adding action: ccw = " << s << std::endl;
        dirs_.emplace(ccw, std::move(s));
        return true;
    }, "");

    args.emplace_back(sec + "cw", [this](std::string s)
    {
        clog_(level::debug) << "Adding action: cw = " << s << std::endl;
        dirs_.emplace(cw, std::move(s));
        return true;
    }, "");

    for(auto speed = speed_min; speed <= speed_max; ++speed)
    {
        args.emplace_back(sec + '@' + std::to_string(speed), [this, speed](std::string s)
        {
            clog_(level::debug) << "Adding action: @" << int(speed) << " = " << s << std::endl;
            speeds_.emplace(speed, std::move(s));
            return true;
        }, "");
    }

    ////////////////////
    pgm::parse(fs, args, pgm::policy::duplicate::accept, pgm::policy::extra::ignore);
    clog_(level::info) << "Found " << indexes_.size() + dirs_.size() + speeds_.size() << " actions for device " << device.name() << std::endl;

    device.pressed().connect(std::bind(&actions::pressed, this, std::placeholders::_1));
    device.jog().connect(std::bind(&actions::jog, this, std::placeholders::_1));
    device.shuttle().connect(std::bind(&actions::shuttle, this, std::placeholders::_1));
}

////////////////////////////////////////////////////////////////////////////////
actions::~actions()
{
    proc_.get_status();
    proc_.detach();
}

////////////////////////////////////////////////////////////////////////////////
void actions::pressed(index_t index)
{
    auto ri = indexes_.find(index);
    if(ri != indexes_.end()) execute(ri->second);
}

////////////////////////////////////////////////////////////////////////////////
void actions::jog(dir_t dir)
{
    auto ri = dirs_.find(dir);
    if(ri != dirs_.end()) execute(ri->second);
}

////////////////////////////////////////////////////////////////////////////////
void actions::shuttle(speed_t speed)
{
    auto ri = speeds_.find(speed);
    if(ri != speeds_.end()) execute(ri->second);
}

////////////////////////////////////////////////////////////////////////////////
void actions::execute(const std::string& c)
{
    proc_.get_status();
    proc_.detach();
    proc_ = proc::process([c]()
    {
        return proc::this_process::replace("/bin/sh", "-c", std::move(c));
    });
}

////////////////////////////////////////////////////////////////////////////////
}
