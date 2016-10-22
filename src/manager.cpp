////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "manager.hpp"
#include <iomanip>

using log::level;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
bool manager::regi_class(pie::regi regi, pie::func func)
{
    using namespace std;
    clog_(level::debug) << "registering class:"
                        << " vid="   << hex << setfill('0') << setw(4) << regi.vid
                        << " pid="   << hex << setfill('0') << setw(4) << regi.pid
                        << " iface=" << dec                            << regi.iface
                        << endl;
    return regis_.emplace(std::move(regi), std::move(func)).second;
}

////////////////////////////////////////////////////////////////////////////////
void manager::add_device(const pie::info& info)
{
    clog_(level::debug) << "added device " << info.path << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void manager::remove_device(const pie::info& info)
{
    clog_(level::debug) << "removed device " << info.path << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
}
