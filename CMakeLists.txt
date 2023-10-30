cmake_minimum_required(VERSION 3.12)

project(Drongo_software)

include(FetchContent)

FetchContent_Declare(
    easyloggingpp
    GIT_REPOSITORY https://github.com/amrayn/easyloggingpp.git
    GIT_TAG        v9.97.1
)

FetchContent_GetProperties(easyloggingpp)
if(NOT easyloggingpp_POPULATED)
    FetchContent_Populate(easyloggingpp)
    # Adding Easylogging++ directly to the build. This defines the EASYLOGGINGPP target.

    include_directories("${easyloggingpp_SOURCE_DIR}/src")

    add_library(easyloggingpp STATIC
    "${easyloggingpp_SOURCE_DIR}/src/easylogging++.cc"
    )

endif()

find_package(Threads REQUIRED)

find_library(GPIOD_LIBRARY NAMES gpiod REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC "src")
set(INC "inc")

include_directories(${INC})

add_executable(Drongo_software 
    "main.cpp"
)

add_library(gpio_class STATIC
    "${SRC}/gpio.cpp"
)

target_link_libraries(gpio_class PRIVATE ${GPIOD_LIBRARY})

add_library(spi_class STATIC
    "${SRC}/spi.cpp"
)

target_link_libraries(Drongo_software PRIVATE gpio_class spi_class Threads::Threads easyloggingpp)