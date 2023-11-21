

#include "easylogging++.h"

inline void easylogging_config(void)
{
    el::Configurations logging_conf;

    logging_conf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime{%A %d/%M/%Y %H:%m:%s} (%level): %msg");

    logging_conf.set(el::Level::Global, el::ConfigurationType::Filename, "Drongo_software.log");

    el::Loggers::reconfigureAllLoggers(logging_conf);

}