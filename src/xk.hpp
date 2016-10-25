////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_XK_HPP
#define PIE_XK_HPP

#include "info.hpp"
#include "xk_base.hpp"
#include "xk16_base.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class xk4 : public xk16_base
{
public:
    xk4(asio::io_service& io, const std::string& path, log::book clog = log::book()) :
        xk16_base(1, io, path, std::move(clog))
    { }
    static constexpr pie::regi regis[] = { { 0x05f3, 0x0467, 0 }, { 0x05f3, 0x0469, 0 } };
};

////////////////////////////////////////////////////////////////////////////////
class xk8 : public xk16_base
{
public:
    xk8(asio::io_service& io, const std::string& path, log::book clog = log::book()) :
        xk16_base(2, io, path, std::move(clog))
    { }
    static constexpr pie::regi regis[] = { { 0x05f3, 0x046a, 0 }, { 0x05f3, 0x046c, 0 } };
};

////////////////////////////////////////////////////////////////////////////////
class xk16 : public xk16_base
{
public:
    xk16(asio::io_service& io, const std::string& path, log::book clog = log::book()) :
        xk16_base(4, io, path, std::move(clog))
    { }
    static constexpr pie::regi regis[] = { { 0x05f3, 0x0419, 0 }, { 0x05f3, 0x041b, 0 } };
};

////////////////////////////////////////////////////////////////////////////////
class xk24 : public xk_base
{
public:
    using xk_base::xk_base;
    static constexpr pie::regi regis[] = { { 0x05f3, 0x0403, 0 }, { 0x05f3, 0x0405, 0 } };
};

////////////////////////////////////////////////////////////////////////////////
class xkr32 : public xk_base
{
public:
    using xk_base::xk_base;
    static constexpr pie::regi regis[] = { { 0x05f3, 0x04ff, 0 }, { 0x05f3, 0x0502, 0 } };
};

////////////////////////////////////////////////////////////////////////////////
class xk60 : public xk_base
{
public:
    using xk_base::xk_base;
    static constexpr pie::regi regis[] = { { 0x05f3, 0x0461, 0 }, { 0x05f3, 0x0463, 0 } };
};

////////////////////////////////////////////////////////////////////////////////
class xk80 : public xk_base
{
public:
    using xk_base::xk_base;
    static constexpr pie::regi regis[] = { { 0x05f3, 0x0441, 0 }, { 0x05f3, 0x0443, 0 } };
};

////////////////////////////////////////////////////////////////////////////////
class xke128 : public xk_base
{
public:
    using xk_base::xk_base;
    static constexpr pie::regi regis[] = { { 0x05f3, 0x04cb, 0 }, { 0x05f3, 0x04ce, 0 } };
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_XK_HPP
