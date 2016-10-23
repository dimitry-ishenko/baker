////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_MANAGER_HPP
#define PIE_MANAGER_HPP

#include "info.hpp"
#include "log/book.hpp"

#include <asio.hpp>
#include <map>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class manager
{
public:
    ////////////////////
    explicit manager(asio::io_service& io, log::book clog = log::book()) :
        io_(io), clog_(std::move(clog))
    { }

    ////////////////////
    bool regi_class(regi, func);

    void add_device(const info&);
    void remove_device(const info&);

private:
    ////////////////////
    asio::io_service& io_;
    log::book clog_;

    std::map<regi, func> regis_;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_MANAGER_HPP
