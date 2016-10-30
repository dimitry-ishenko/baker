////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_XK_HPP
#define PIE_XK_HPP

#include "device.hpp"
#include "types.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
template<typename> struct traits { };

template<typename T>
class XK : public traits<T>::base
{
public:
    XK(asio::io_service& io, const std::string& path, log::book clog) :
        traits<T>::base(io, traits<T>::rows, traits<T>::name, path, std::move(clog))
    { }
};

////////////////////////////////////////////////////////////////////////////////
// device tags
struct  XK_4   { };
struct  XK_8   { };
struct  XK_16  { };
struct  XK_24  { };
struct XKR_32  { };
struct  XK_60  { };
struct  XK_80  { };
struct XKE_128 { };

template<>
struct traits<XK_4>
{
    static constexpr auto name = "XK-4";
    static constexpr regi regis[] = { { 0x05f3, 0x0467, 0 }, { 0x05f3, 0x0469, 0 } };
    static constexpr byte rows = 1;
    using base = XK16_device;
};

template<>
struct traits<XK_8>
{
    static constexpr auto name = "XK-8";
    static constexpr regi regis[] = { { 0x05f3, 0x046a, 0 }, { 0x05f3, 0x046c, 0 } };
    static constexpr byte rows = 2;
    using base = XK16_device;
};

template<>
struct traits<XK_16>
{
    static constexpr auto name = "XK-16";
    static constexpr regi regis[] = { { 0x05f3, 0x0419, 0 }, { 0x05f3, 0x041b, 0 } };
    static constexpr byte rows = 4;
    using base = XK16_device;
};

template<>
struct traits<XK_24>
{
    static constexpr auto name = "XK-24";
    static constexpr regi regis[] = { { 0x05f3, 0x0403, 0 }, { 0x05f3, 0x0405, 0 } };
    static constexpr byte rows = 0; // ignored
    using base = XK_device;
};

template<>
struct traits<XKR_32>
{
    static constexpr auto name = "XKR-32";
    static constexpr regi regis[] = { { 0x05f3, 0x04ff, 0 }, { 0x05f3, 0x0502, 0 } };
    static constexpr byte rows = 0; // ignored
    using base = XK_device;
};

template<>
struct traits<XK_60>
{
    static constexpr auto name = "XK-60";
    static constexpr regi regis[] = { { 0x05f3, 0x0461, 0 }, { 0x05f3, 0x0463, 0 } };
    static constexpr byte rows = 0; // ignored
    using base = XK_device;
};

template<>
struct traits<XK_80>
{
    static constexpr auto name = "XK-80";
    static constexpr regi regis[] = { { 0x05f3, 0x0441, 0 }, { 0x05f3, 0x0443, 0 } };
    static constexpr byte rows = 0; // ignored
    using base = XK_device;
};

template<>
struct traits<XKE_128>
{
    static constexpr auto name = "XKE-128";
    static constexpr regi regis[] = { { 0x05f3, 0x04cb, 0 }, { 0x05f3, 0x04ce, 0 } };
    static constexpr byte rows = 0; // ignored
    using base = XK_device;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_XK_HPP
