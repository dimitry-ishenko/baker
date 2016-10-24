////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_XK_HPP
#define PIE_XK_HPP

#include "sig/signal.hpp"
#include "xk_base.hpp"

#include <asio/system_timer.hpp>
#include <memory>
#include <set>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

using index_t = int;
static constexpr index_t prog = -1;

////////////////////////////////////////////////////////////////////////////////
class XK : public XK_base
{
public:
    ////////////////////
    XK(asio::io_service& io, const std::string& path, log::book = log::book());

    ////////////////////
    auto uid() const noexcept { return uid_; }
    auto columns() const noexcept { return columns_; }
    auto rows() const noexcept { return rows_; }

    ////////////////////
    sig::signal_proxy<void(int)>& pressed() { return pressed_; }
    sig::signal_proxy<void(int)>& released() { return released_; }

    sig::signal_proxy<void(bool)>& locked() { return locked_; }

protected:
    ////////////////////
    asio::system_timer timer_;

    int uid_;
    std::size_t columns_, rows_, total_;

    bool ps_ = false;
    std::unique_ptr<byte[]> prev_;

    bool lock_ = false;

    void read();
    void schedule_read();

    using press = std::set<index_t>;
    using release = std::set<index_t>;
    virtual std::tuple<press, release> process_read(const store&);

    ////////////////////
    sig::signal<void(int)> pressed_;
    sig::signal<void(int)> released_;

    sig::signal<void(bool)> locked_;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // XK_HPP
