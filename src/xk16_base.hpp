////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef XK16_BASE_HPP
#define XK16_BASE_HPP

#include "xk_base.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class xk16_base : public xk_base
{
protected:
    ////////////////////
    xk16_base(std::size_t rows, asio::io_service&, const std::string& path, log::book);

    press_release process_read(const store&) override;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // XK16_BASE_HPP
