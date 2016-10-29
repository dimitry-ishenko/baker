////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_ACTIONS_HPP
#define PIE_ACTIONS_HPP

#include "device.hpp"
#include "log/book.hpp"
#include "proc/process.hpp"

#include <map>
#include <string>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class actions
{
public:
    ////////////////////
    actions(const std::string& conf, XK_device&, log::book);
    ~actions();

protected:
    ////////////////////
    log::book clog_;

    enum { critical, command }; // indexes for action tuple
    using action = std::tuple<bool, std::string>;

    std::map<index_t, action> map_;

    proc::process proc_;
    void pressed(index_t);
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_ACTIONS_HPP
