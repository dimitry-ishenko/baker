////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "closing.hpp" // catch_interrupted
#include "log/book.hpp"
#include "manager.hpp"
#include "monitor.hpp"
#include "pgm/args.hpp"
#include "version.hpp"

#include <asio.hpp>
#include <exception>
#include <iostream>
#include <string>

app::version version = { 2, 0, 1 };

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    std::string name = argv[0];
    auto pos = name.find_last_of('/');
    if(pos != std::string::npos) name.erase(0, pos + 1);

    ////////////////////
    using log::level;

    log::book clog;
    clog.log_to(log::cerr(), log::range<level::fatal, level::info>);

    ////////////////////
    std::string conf = "/etc/baker.conf";
    bool verbose = false;
    bool quiet = false;
    bool syslog = false;

    pgm::arg args[] =
    {
        { 'c', "conf"   , conf   , "Override path to config file"    },
        { 'v', "version"         , "Show version and exit"           },
        { 'q', "quiet"  , quiet  , "Quiet mode (show no messages)"   },
        { 'o', "verbose", verbose, "Show verbose messages"           },
        { 's', "syslog" , syslog , "Log messages to syslog"          },
        { 'h', "help"            , "Show this screen and exit"       },
    };
    std::string desc = "Controller for P.I. Engineering X-keys devices.";

    ////////////////////
    try
    {
        pgm::parse(app::make_args(argc, argv), args);

        if(pgm::count(args, "help"))
        {
            std::cout << pgm::usage(args, argv[0], desc) << std::endl;
            return 0;
        }
        if(pgm::count(args, "version"))
        {
            std::cout << name << " version " << version << std::endl;
            return 0;
        }

        if(syslog)
        {
            clog.clear();
            clog.log_to(log::cerr(), level::fatal);
            clog.log_to(log::system(), log::range<level::fatal, level::info>);
        }
        if(verbose)
        {
            if(syslog) clog.log_to(log::system(), level::debug);
            else clog.log_to(log::cerr(), level::debug);
        }
        if(quiet) clog.clear();

        ////////////////////
        clog(level::info) << "Starting " << name << std::endl;
        {
            asio::io_service io;

            pie::monitor monitor(io, clog);
            pie::manager manager(io, conf, clog);

            monitor.device_added().connect(std::bind(&pie::manager::add_device, &manager, std::placeholders::_1));
            monitor.device_removed().connect(std::bind(&pie::manager::remove_device, &manager, std::placeholders::_1));

            catch_interrupted(io.run());
        }
        clog(level::info) << "Exiting " << name << std::endl;

        return 0;
    }
    catch(std::exception& e)
    {
        clog(level::fatal) << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
