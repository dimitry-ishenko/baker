////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "xk16_base.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

xk16_base::xk16_base(std::size_t rows, asio::io_service& io, const std::string& path, log::book clog) :
    xk_base(io, path, std::move(clog))
{
    rows_ = rows;
}

////////////////////////////////////////////////////////////////////////////////
static inline index_t map(index_t index)
{
    return ((index & 0x07) << 2) | ((index >> 3) & 0x03);
}

////////////////////////////////////////////////////////////////////////////////
xk16_base::press_release xk16_base::process_read(const std::vector<byte>& store)
{
    press_release pr1 = xk_base::process_read(store), pr2;

    for(auto n : std::get<0>(pr1)) std::get<0>(pr2).insert(map(n));
    for(auto n : std::get<1>(pr1)) std::get<1>(pr2).insert(map(n));

    return pr2;
}

////////////////////////////////////////////////////////////////////////////////
}
