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
#include "xk.hpp"

#include <asio.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class manager
{
public:
    ////////////////////
    explicit manager(asio::io_service&, log::book = log::book());

    void add_device(const info&);
    void remove_device(const info&);

private:
    ////////////////////
    asio::io_service& io_;
    log::book clog_;

    using create = std::function<int(asio::io_service&, const std::string& path, const log::book&)>;
    std::map<regi, create> regis_;

    template<typename> void regi_class();

    static int proxy(const create&, asio::io_service&, const std::string& path, const log::book&);
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_MANAGER_HPP
