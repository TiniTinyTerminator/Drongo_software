#include <iostream>

#include "ini.h"

#define ELPP_NO_LOG_TO_FILE
// #define ELPP_THREAD_SAFE

#include "easylogging++.h"

#include "utils/easylogging_setup.h"

#include "DataHandler.h"

using namespace std::chrono_literals;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    easylogging_config();

    START_EASYLOGGINGPP(argc, argv);

    LOG(INFO) << "hello world!";

    DataHandler handler;

    handler.setup_adc(10);

    handler.irq_thread_start();
    std::this_thread::sleep_for(10ms);
    handler.storing_thread_start();
    std::this_thread::sleep_for(10ms);
    handler.plot_thread_start();

    while (true)
    {
        std::this_thread::sleep_for(10ms);
    }
    

    return 0;
}
