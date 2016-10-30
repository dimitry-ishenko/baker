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

    auto section = std::to_string(device.uid()) + '-';
    std::vector<pgm::arg> args;

    ////////////////////
    args.emplace_back(section + "freq", [&device](std::string s)
    {
        device.set_freq(stob(s));
        return true;
    }, "");

    args.emplace_back(section + "red" , [&device](std::string s)
    {
        device.set_level(light::red, stob(s));
        return true;
    }, "");

    args.emplace_back(section + "blue", [&device](std::string s)
    {
        device.set_level(light::blue, stob(s));
        return true;
    }, "");

    ////////////////////
    std::vector<std::string> actions(device.total() * 2);

    // create args in the form <uid>-<index>{*}
    for(std::size_t n = 0; n < actions.size(); ++n)
    {
        auto index = n / 2;
        bool critical = n & 1;

        auto name = section + std::to_string(index);
        if(critical) name += '*';

        args.emplace_back(name, actions[n], "");
    }

    pgm::parse(fs, args, pgm::policy::duplicate::accept, pgm::policy::extra::ignore);

    for(std::size_t n = 0; n < actions.size(); ++n)
        if(actions[n].size())
        {
            auto index = n / 2;
            bool critical = n & 1;

            auto name = section + std::to_string(index);
            if(critical) name += '*';
            clog_(level::debug) << "Found action: " << name << "\t= " << actions[n] << std::endl;

            index_map_.emplace(index, std::make_tuple(critical, std::move(actions[n])));
        }
    clog_(level::info) << "Found " << index_map_.size() << " actions for device " << device.name() << std::endl;

    ////////////////////
    for(const auto& pair : index_map_)
        if(std::get<critical>(pair.second)) device.critical(pair.first);

    device.pressed().connect(std::bind(&actions::pressed, this, std::placeholders::_1));
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
    auto ri = index_map_.find(index);
    if(ri != index_map_.end()) execute(std::get<command>(ri->second));
}

////////////////////////////////////////////////////////////////////////////////
void actions::execute(const std::string& c)
{
    proc_.get_status();
    proc_ = proc::process([c]()
    {
        return proc::this_process::replace("/bin/sh", "-c", std::move(c));
    });
}

////////////////////////////////////////////////////////////////////////////////
}
