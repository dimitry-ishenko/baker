////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_DEVICE_HPP
#define PIE_DEVICE_HPP

#include "closing.hpp"
#include "functions.hpp"
#include "log/book.hpp"
#include "signals.hpp"
#include "types.hpp"

#include <asio/system_timer.hpp>
#include <set>
#include <string>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class device : public signals, public closing
{
public:
    ////////////////////
    virtual ~device() { close(); }

    void close() noexcept override;

    ////////////////////
    const std::string& name() const noexcept { return name_; }

    auto uid() const noexcept { return uid_; }
    auto total() const noexcept { return total_; }

    void critical(index_t n) { critical_.insert(n); }

    ////////////////////
    void set_freq(byte freq) { func_.set_freq(freq); }
    void set_level(light::color_t color, byte level) { func_.set_level(color, level, light::fade); }

protected:
    ////////////////////
    device(asio::io_service&, const params&, const info&, log::book);

    ////////////////////
    std::string name_;
    log::book clog_;

    asio::system_timer timer_;
    functions func_;

    byte uid_;
    byte columns_, rows_, total_;

    bool ps_ = false;
    std::vector<byte> prev_;

    bool lock_ = false;

    using buttons = std::set<index_t>;
    enum { press, release }; // indexes for press_release tuple
    using press_release = std::tuple<buttons, buttons>;

    buttons ignore_;
    buttons critical_;

    static constexpr index_t none = static_cast<index_t>(-1);
    index_t pending_ = none;

    void read();
    void schedule_read();

    virtual press_release process_read(const std::vector<byte>&);
};

////////////////////////////////////////////////////////////////////////////////
class device_XK16 : public device
{
protected:
    ////////////////////
    device_XK16(asio::io_service& io, const pie::params& params, const pie::info& info, log::book clog) :
        device(io, params, info, std::move(clog))
    {
        rows_ = params.rows;
    }

    press_release process_read(const std::vector<byte>&) override;
};

////////////////////////////////////////////////////////////////////////////////
class device_jog : public device
{
protected:
    ////////////////////
    device_jog(asio::io_service& io, const pie::params& params, const pie::info& info, log::book clog) :
        device(io, params, info, std::move(clog))
    {
        off_ = params.off_jog;
    }

    std::size_t off_;
    speed_t speed_ = 0;

    press_release process_read(const std::vector<byte>&) override;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_device_HPP
