////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Dimitry Ishenko
// Distributed under the GNU GPL v2. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com

////////////////////////////////////////////////////////////////////////////////
#include "log/book.hpp"
#include "manager.hpp"
#include "monitor.hpp"
#include "pgm/args.hpp"

#include <asio.hpp>
#include <exception>
#include <iostream>
#include <string>

using log::level;
log::book clog;

int main(int argc, char* argv[])
try
{
    ////////////////////
    clog.log_to(log::cerr(), log::range<level::fatal, level::info>);

    pgm::arg args[] =
    {
        { 'v', "verbose", "Print debugging messages"   },
        { 'h', "help",    "Print this screen and exit" },
    };
    std::string desc = "Controller for P.I. Engineering X-keys devices.";

    pgm::parse(app::make_args(argc, argv), args);

    if(pgm::count(args, "help"))
    {
        std::cout << pgm::usage(args, argv[0], desc) << std::endl;
        return 0;
    }
    if(pgm::count(args, "verbose")) clog.log_to(log::cerr(), level::debug);

    ////////////////////
    asio::io_service io;

    pie::monitor monitor(io);
    pie::manager manager;

    monitor.device_added().connect(std::bind(&pie::manager::add_device, &manager, std::placeholders::_1));
    monitor.device_removed().connect(std::bind(&pie::manager::remove_device, &manager, std::placeholders::_1));

    io.run();

    ////////////////////
    return 0;
}
catch(std::exception& e)
{
    clog(level::fatal) << "Error: " << e.what() << std::endl;
    return 1;
}
catch(...)
{
    clog(level::fatal) << "Unknown error" << std::endl;
    return 1;
}
