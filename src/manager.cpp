////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "manager.hpp"
#include "proc/process.hpp"

#include <stdexcept>

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
template<typename XK>
void manager::regi_device()
{
    for(auto regi : XK::regis)
    {
        clog_(level::info) << "Registering device: " << regi << std::endl;
        regis_.emplace(regi, [](asio::io_service& io, const std::string& path, const log::book& clog)
        {
            io.notify_fork(asio::io_service::fork_child);
            XK device(io, path, clog);
            io.run(); return 0;
        });
    }
}

////////////////////////////////////////////////////////////////////////////////
manager::manager(asio::io_service& io, log::book clog) :
    io_(io), clog_(std::move(clog))
{
    regi_device<xk4   >();
    regi_device<xk8   >();
    regi_device<xk16  >();
    regi_device<xk24  >();
    regi_device<xkr32 >();
    regi_device<xk60  >();
    regi_device<xk80  >();
    regi_device<xke128>();
}

////////////////////////////////////////////////////////////////////////////////
void manager::add_device(const pie::info& info)
{
    auto ri = regis_.find(info.regi);
    if(ri != regis_.end())
    {
        clog_(level::debug) << "Found regi for: " << info.regi << std::endl;

        io_.notify_fork(asio::io_service::fork_prepare);
        proc::process p(std::cref(ri->second), std::ref(io_), info.path, clog_);

        if(p.get_status() == proc::process::running)
        {
            io_.notify_fork(asio::io_service::fork_parent);

            clog_(level::info) << "Started process " << p.get_id() << " for device: " << info.path << std::endl;
            p.detach();
        }
        else throw std::runtime_error("Failed to start process");
    }
    else clog_(level::debug) << "No regi for: " << info.regi << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void manager::remove_device(const pie::info& info)
{
    clog_(level::debug) << "Removed device: " << info.path << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
}
