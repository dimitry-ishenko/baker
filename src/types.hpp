////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_TYPES_HPP
#define PIE_TYPES_HPP

#include <cstdint>
#include <initializer_list>
#include <iomanip>
#include <ostream>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using byte = uint8_t;
using word = uint16_t;

////////////////////////////////////////////////////////////////////////////////
using index_t = byte;
static constexpr index_t prog = static_cast<index_t>(-1);

enum dir_t : int8_t { ccw = -1, cw = 1 };

using speed_t = int8_t;
static constexpr speed_t speed_min = -7;
static constexpr speed_t speed_max = 7;

////////////////////////////////////////////////////////////////////////////////
using vid = word;
using pid = word;
using iface = word;

static constexpr word invalid = static_cast<word>(-1);

////////////////////////////////////////////////////////////////////////////////
struct regi
{
    pie::vid vid = invalid;
    pie::pid pid = invalid;
    pie::iface iface = invalid;

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
struct params
{
    const byte rows = 0;
    std::size_t off_jog = 0; // offset of jog & shuttle bytes
    const std::initializer_list<index_t> ignore = {};
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

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_TYPES_HPP
