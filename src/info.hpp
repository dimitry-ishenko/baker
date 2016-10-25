////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_INFO_HPP
#define PIE_INFO_HPP

#include <iomanip>
#include <ostream>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
static constexpr int invalid = -1;

////////////////////////////////////////////////////////////////////////////////
using vid = int;
using pid = int;
using iface = int;

////////////////////////////////////////////////////////////////////////////////
struct regi
{
    pie::vid vid;
    pie::pid pid;
    pie::iface iface;

    bool is_valid() const noexcept { return vid != invalid && pid != invalid && iface != invalid; }
};

////////////////////////////////////////////////////////////////////////////////
struct info
{
    std::string path;
    pie::regi regi;
    std::string product;
};

////////////////////////////////////////////////////////////////////////////////
inline bool operator<(const regi& x, const regi& y) noexcept
{
    return (x.vid < y.vid)
        || (x.vid== y.vid && (x.pid < y.pid
                          || (x.pid== y.pid && x.iface < y.iface)));
}

////////////////////////////////////////////////////////////////////////////////
inline std::ostream& operator<<(std::ostream& os, const pie::regi& regi)
{
    using namespace std;
    if(regi.is_valid())
    {
        os << hex << setfill('0') << setw(4) << regi.vid << ':'
           << hex << setfill('0') << setw(4) << regi.pid << '.'
           << dec                            << regi.iface;
    }
    return os;
}

////////////////////////////////////////////////////////////////////////////////
inline std::ostream& operator<<(std::ostream& os, const pie::info& info)
{
    if(info.regi.is_valid()) os << info.regi << ' ';
    if(info.product.size())  os << info.product << ' ';
                             os << "on " << info.path;
    return os;
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_INFO_HPP
