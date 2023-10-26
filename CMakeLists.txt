cmake_minimum_required(VERSION 3.12)

project(Drongo_software)

set(SRC "src")

set(INC "inc")

set(CMAKE_CXX_STANDARD 20)

include_directories(${INC})

add_executable(Drongo_software 
    "main.cpp"
)

add_library(gpio_class
    "${SRC}/gpio.cpp"
)

add_library(spi_class
    "${SRC}/spi.cpp"
)