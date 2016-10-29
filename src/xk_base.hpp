////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_XK_BASE_HPP
#define PIE_XK_BASE_HPP

#include "functions.hpp"
#include "log/book.hpp"
#include "sig/signal.hpp"
#include "signals.hpp"

#include <asio/system_timer.hpp>
#include <set>
#include <string>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class xk_base : public signals
{
public:
    ////////////////////
    xk_base(asio::io_service&, const std::string& path, log::book);
    virtual ~xk_base() { close(); }

    virtual void close();

    ////////////////////
    auto device_id() const noexcept { return "Device " + name_; }

    auto uid() const noexcept { return uid_; }
    auto total() const noexcept { return total_; }

    void set_critical(index_t n) { critical_.insert(n); }
    bool critical(index_t n) const { return critical_.count(n); }

    void reset_critical(index_t n) { critical_.erase(n); }
    void clear_critical() { critical_.clear(); }

protected:
    ////////////////////
    asio::system_timer timer_;
    std::string name_;
    functions func_;
    log::book clog_;

    int uid_;
    std::size_t columns_, rows_, total_;

    bool ps_ = false;
    std::vector<byte> prev_;

    bool lock_ = false;

    using set = std::set<index_t>;
    using press_release = std::tuple<set, set>;

    set critical_;

    static constexpr index_t none = -1;
    index_t pending_ = none;

    void read();
    void schedule_read();

    virtual press_release process_read(const std::vector<byte>&);
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_XK_BASE_HPP
