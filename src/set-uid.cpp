////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "functions.hpp"
#include "monitor.hpp"
#include "pgm/args.hpp"
#include "version.hpp"
#include "xk.hpp"

#include <asio.hpp>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

app::version version = { 2, 0, 1 };

////////////////////////////////////////////////////////////////////////////////
void list_devices()
{
    using namespace pie;

    std::set<regi> regis;
    regis.insert(traits< XK_4   >::regis);
    regis.insert(traits< XK_8   >::regis);
    regis.insert(traits< XK_12JS>::regis);
    regis.insert(traits< XK_16  >::regis);
    regis.insert(traits< XK_24  >::regis);
    regis.insert(traits<XKR_32  >::regis);
    regis.insert(traits< XK_60  >::regis);
    regis.insert(traits< XK_68JS>::regis);
    regis.insert(traits< XK_80  >::regis);
    regis.insert(traits<XKE_128 >::regis);

    asio::io_service io;
    pie::monitor monitor(io, log::book(), enum_only);

    monitor.device_added().connect([&regis, &io](const pie::info& info)
    {
        if(regis.count(info.regi))
        {
            pie::functions func(io, info.path);
            func.request_desc();
            pie::desc desc = func.read_desc();

            std::cout << "Found " << info.product
                      << " with uid " << int(desc.uid)
                      << " on " << info.path << std::endl;
        }
    });
    io.run();
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    ////////////////////
    std::string path;
    int uid;

    pgm::arg args[] =
    {
        { 'l', "list"   , "List connected devices and exit" },
        { 'v', "version", "Show version and exit"           },
        { 'h', "help"   , "Show this screen and exit"       },

        { path          , "Path to device"                  },
        { uid           , "Unit id"                         },
    };
    std::string desc = "Set unit id on an X-keys device.";

    ////////////////////
    try
    {
        try
        {
            // since path and uid are mandatory non-option args,
            // parse will throw, if they are not specified;
            // so we need an inner try/catch to handle options
            // like -h -v and -l
            pgm::parse(app::make_args(argc, argv), args);
        }
        catch(pgm::invalid_option&)
        {
            if(pgm::count(args, "help"))
            {
                std::cout << pgm::usage(args, argv[0], desc) << std::endl;
                return 0;
            }
            else if(pgm::count(args, "version"))
            {
                std::cout << argv[0] << " version " << version << std::endl;
                return 0;
            }
            else if(pgm::count(args, "list"))
            {
                list_devices();
                return 0;
            }
            else throw;
        }

        if(uid < 0 || uid > 255) throw std::out_of_range("Unit id must be within 0-255");

        asio::io_service io;
        pie::functions func_(io, path);
        func_.set_uid(static_cast<pie::byte>(uid));

        std::cout << "Done" << std::endl;
        return 0;
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
