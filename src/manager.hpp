////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_MANAGER_HPP
#define PIE_MANAGER_HPP

#include "log/book.hpp"
#include "types.hpp"

#include <asio.hpp>
#include <functional>
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
    explicit manager(asio::io_service&, std::string conf, log::book);

    void add_device(const info&);
    void remove_device(const info&);

private:
    ////////////////////
    asio::io_service& io_;
    std::string conf_;
    log::book clog_;

    using create = std::function<int(asio::io_service&, const std::string& path, const log::book&)>;
    std::map<regi, create> regis_;

    template<typename> void regi_device();
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_MANAGER_HPP
