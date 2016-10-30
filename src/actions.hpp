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
#include "types.hpp"

#include <map>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class actions
{
public:
    ////////////////////
    actions(const std::string& conf, device&, log::book);
    ~actions();

protected:
    ////////////////////
    log::book clog_;

    std::map<index_t, std::string> index_map_;

    void pressed(index_t);

    proc::process proc_;
    void execute(const std::string&);
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_ACTIONS_HPP
