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

#include <initializer_list>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
template<typename> struct traits { };

template<typename T>
class XK : public traits<T>::base
{
public:
    XK(asio::io_service& io, const pie::info& info, log::book clog) :
        traits<T>::base(io, traits<T>::params, info, std::move(clog))
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
    static constexpr pie::params params = { 1 };
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x0467, 0 },
        { 0x05f3, 0x0469, 0 },
    };
    using base = device_XK16;
};

template<>
struct traits<XK_8>
{
    static constexpr pie::params params = { 2 };
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x046a, 0 },
        { 0x05f3, 0x046c, 0 },
    };
    using base = device_XK16;
};

template<>
struct traits<XK_16>
{
    static constexpr pie::params params = { 4 };
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x0419, 0 },
        { 0x05f3, 0x041b, 0 },
    };
    using base = device_XK16;
};

template<>
struct traits<XK_24>
{
    static constexpr pie::params params = {};
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x0403, 0 },
        { 0x05f3, 0x0405, 0 },
    };
    using base = device;
};

template<>
struct traits<XKR_32>
{
    static constexpr pie::params params = {};
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x04ff, 0 },
        { 0x05f3, 0x0502, 0 },
    };
    using base = device;
};

template<>
struct traits<XK_60>
{
    static constexpr pie::params params = {};
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x0461, 0 },
        { 0x05f3, 0x0463, 0 },
    };
    using base = device;
};

template<>
struct traits<XK_80>
{
    static constexpr pie::params params = {};
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x0441, 0 },
        { 0x05f3, 0x0443, 0 },
    };
    using base = device;
};

template<>
struct traits<XKE_128>
{
    static constexpr pie::params params = {};
    static constexpr std::initializer_list<regi> regis =
    {
        { 0x05f3, 0x04cb, 0 },
        { 0x05f3, 0x04ce, 0 },
    };
    using base = device;
};

}

////////////////////////////////////////////////////////////////////////////////
#endif // PIE_XK_HPP
