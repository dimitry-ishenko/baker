////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_SIGNALS_HPP
#define PIE_SIGNALS_HPP

#include "sig/signal.hpp"
#include "types.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class signals
{
public:
    ////////////////////
    sig::signal_proxy<void(index_t)>& pressed() { return pressed_; }
    sig::signal_proxy<void(index_t)>& released() { return released_; }

    sig::signal_proxy<void(bool)>& locked() { return locked_; }

protected:
    ////////////////////
    sig::signal<void(index_t)> pressed_;
    sig::signal<void(index_t)> released_;

    sig::signal<void(bool)> locked_;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_SIGNALS_HPP
