#include <iostream>

#include "ini.h"
#include "easylogging++.h"

#include "utils/easylogging_setup.h"

#include "DataHandler.h"

using namespace std::chrono_literals;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    easylogging_config();

    LOG(INFO) << "hello world!";

    DataHandler handler;

    handler.setup_adc(10);

    handler.irq_thread_start();
    handler.processing_thread_start();

    while (true)
    {
        std::this_thread::sleep_for(10ms);
    }
    

    return 0;
}
