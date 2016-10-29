////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "actions.hpp"
#include "pgm/args.hpp"
#include "proc/process.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

using namespace std::literals::chrono_literals;
using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
actions::actions(const std::string& conf, XK_device& device, log::book clog) :
    clog_(clog)
{
    clog_(level::info) << "Reading file " << conf << std::endl;
    std::fstream fs(conf, std::ios_base::in);
    if(!fs.is_open()) throw std::runtime_error("Failed to open file");

    std::vector<std::string> actions(device.total() * 2);
    std::vector<pgm::arg> args;

    // create args in the form <uid>-<index>{*}
    for(std::size_t n = 0; n < actions.size(); ++n)
    {
        auto index = n / 2;
        bool critical = n & 1;

        auto name = std::to_string(device.uid()) + '-' + std::to_string(index);
        if(critical) name += '*';

        args.emplace_back(name, actions[n], "");
    }

    pgm::parse(fs, args, pgm::policy::duplicate::accept, pgm::policy::extra::ignore);

    for(std::size_t n = 0; n < actions.size(); ++n)
        if(actions[n].size())
        {
            auto index = n / 2;
            bool critical = n & 1;

            auto name = std::to_string(device.uid()) + ':' + std::to_string(index);
            if(critical) name += '*';
            clog_(level::debug) << "Found action: " << name << "\t= " << actions[n] << std::endl;

            map_.emplace(index, std::make_tuple(critical, std::move(actions[n])));
        }
    clog_(level::info) << "Found " << map_.size() << " actions for unit id " << device.uid() << std::endl;

    ////////////////////
    for(const auto& pair : map_)
        if(std::get<critical>(pair.second)) device.critical(pair.first);

    device.pressed().connect(std::bind(&actions::pressed, this, std::placeholders::_1));
}

////////////////////////////////////////////////////////////////////////////////
void actions::pressed(index_t index)
{
    auto ri = map_.find(index);
    if(ri != map_.end())
    {
        proc::process p([c = std::get<command>(ri->second)]()
        {
            return proc::this_process::replace("/bin/sh", "-c", std::move(c));
        });
        p.wait_for(0s);
        p.detach();
    }
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
}
