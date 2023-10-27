cmake_minimum_required(VERSION 3.12)

project(Drongo_software)

set(CMAKE_TOOLCHAIN_FILE "toolchain.cmake")

message(STATUS "CMAKE_SYSTEM_PREFIX_PATH: ${CMAKE_SYSTEM_PREFIX_PATH}")
message(STATUS "CMAKE_SYSTEM_LIBRARY_PATH: ${CMAKE_SYSTEM_LIBRARY_PATH}")
message(STATUS "CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}")
message(STATUS "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")
message(STATUS "CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_library(GPIOD_LIBRARY
    NAMES gpiod
    HINTS
)

set(SRC "src")

set(INC "inc")

set(CMAKE_CXX_STANDARD 20)

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