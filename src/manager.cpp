////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "manager.hpp"
#include "proc/process.hpp"

#include <iomanip>
#include <ostream>
#include <stdexcept>

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const pie::regi& regi)
{
    using namespace std;
    return os <<   "vid=" << hex << setfill('0') << setw(4) << regi.vid << ' '
              <<   "pid=" << hex << setfill('0') << setw(4) << regi.pid << ' '
              << "iface=" << dec                            << regi.iface;
}

////////////////////////////////////////////////////////////////////////////////
bool manager::regi_class(pie::regi regi, pie::func func)
{
    clog_(level::debug) << "registering class: " << regi << std::endl;
    return regis_.emplace(std::move(regi), std::move(func)).second;
}

////////////////////////////////////////////////////////////////////////////////
static int func_proxy(asio::io_service& io, const pie::func& func, const std::string& path)
{
    io.notify_fork(asio::io_service::fork_child);
    return func(path);
}

////////////////////////////////////////////////////////////////////////////////
void manager::add_device(const pie::info& info)
{
    auto ri = regis_.find(info.regi);
    if(ri != regis_.end())
    {
        clog_(level::debug) << "found regi for: " << info.regi << std::endl;

        io_.notify_fork(asio::io_service::fork_prepare);
        proc::process p(func_proxy, std::ref(io_), ri->second, info.path);

        if(p.get_status() == proc::process::running)
        {
            io_.notify_fork(asio::io_service::fork_parent);

            clog_(level::debug) << "started new process: " << p.get_id() << std::endl;
            p.detach();
        }
        else throw std::runtime_error("failed to start new process");
    }
    else clog_(level::debug) << "no regi for: " << info.regi << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void manager::remove_device(const pie::info& info)
{
    clog_(level::debug) << "removed device " << info.path << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
}
