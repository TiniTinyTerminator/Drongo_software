cmake_minimum_required(VERSION 3.12)

project(Drongo_software)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

find_library(GPIOD_LIBRARY
    NAMES gpiod
)

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

target_link_libraries(Drongo_software gpio_class spi_class)