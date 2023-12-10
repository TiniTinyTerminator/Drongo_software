#include <iostream>

#include "argparse/argparse.hpp"

#define ELPP_NO_LOG_TO_FILE
#include "easylogging++.h"

#include "utils/easylogging_setup.h"

#include "DataHandler.h"

using namespace std::chrono_literals;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    easylogging_config();

    START_EASYLOGGINGPP(argc, argv);

    argparse::ArgumentParser program("Drongo Software");

    program.add_argument("-o", "--output")
        .help("set the output directory for the data")
        .default_value("drongo_data")
        .required();

    program.add_argument("-n", "--number_channels")
        .help("specify how many geophones are connected")
        .default_value(4)
        .scan<'i', int>()
        .required();

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &err)
    {
        LOG(ERROR) << err.what() << std::endl;
        LOG(ERROR) << program;
        return 1;
    }

    LOG(INFO) << "hello world!";

    DataHandler handler;

    handler.set_data_path(program.get("--output"));

    auto n_channels = program.get<int>("--number_channels");

    handler.setup_adc(n_channels ,10);

    handler.irq_thread_start();
    std::this_thread::sleep_for(10ms);
    handler.storing_thread_func();


    return 0;
}
