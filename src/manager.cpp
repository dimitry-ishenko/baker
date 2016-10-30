////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "actions.hpp"
#include "closing.hpp" // catch_interrupted
#include "manager.hpp"
#include "proc/process.hpp"
#include "xk.hpp"

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
manager::manager(asio::io_service& io, std::string conf, log::book clog) :
    io_(io), conf_(std::move(conf)), clog_(std::move(clog))
{
    regi_device< XK_4  >();
    regi_device< XK_8  >();
    regi_device< XK_16 >();
    regi_device< XK_24 >();
    regi_device<XKR_32 >();
    regi_device< XK_60 >();
    regi_device< XK_80 >();
    regi_device<XKE_128>();
}

////////////////////////////////////////////////////////////////////////////////
void manager::add_device(const pie::info& info)
{
    auto ri = regis_.find(info.regi);
    if(ri != regis_.end())
    {
        io_.notify_fork(asio::io_service::fork_prepare);
        proc::process p(std::cref(ri->second), std::ref(io_), info, clog_);
        io_.notify_fork(asio::io_service::fork_parent);

        clog_(level::info) << "Started process " << p.get_id() << " for " << info.path << std::endl;
        p.detach();
    }
    else clog_(level::info) << "Ignoring " << info.path << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void manager::remove_device(const pie::info& info)
{
    clog_(level::debug) << "Removed device: " << info.path << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
template<typename T>
void manager::regi_device()
{
    for(auto regi : traits<T>::regis)
    {
        clog_(level::info) << "Registering device: " << regi << std::endl;
        regis_.emplace(regi, [this](asio::io_service& io, const pie::info& info, const log::book& clog)
        {
            io.notify_fork(asio::io_service::fork_child);

            try
            {
                XK<T> device(io, info, clog);
                pie::actions actions(conf_, device, clog);

                catch_interrupted(io.run());
                return 0;
            }
            catch(std::exception& e)
            {
                clog_(level::fatal) << "ERROR: " << e.what() << std::endl;
                return 1;
            }
        });
    }
}

////////////////////////////////////////////////////////////////////////////////
}
