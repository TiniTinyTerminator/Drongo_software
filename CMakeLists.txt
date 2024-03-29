cmake_minimum_required(VERSION 3.25)

project(Drongo_software)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    easyloggingpp
    GIT_REPOSITORY https://github.com/amrayn/easyloggingpp.git
    GIT_TAG v9.97.1
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

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v3.0
)

FetchContent_GetProperties(argparse)

if(NOT argparse_POPULATED)
    FetchContent_Populate(argparse)

    # Adding Easylogging++ directly to the build. This defines the EASYLOGGINGPP target.
    include_directories("${argparse_SOURCE_DIR}/include")

endif()

FetchContent_Declare(
    iir1
    GIT_REPOSITORY https://github.com/berndporr/iir1.git
    GIT_TAG 1.9.4
)

FetchContent_GetProperties(iir1)

if(NOT iir1_POPULATED)
    FetchContent_Populate(iir1)

    add_subdirectory(${iir1_SOURCE_DIR})
    include_directories(${iir1_SOURCE_DIR})
endif()

find_package(Threads REQUIRED)

find_library(GPIOD_LIBRARY NAMES libgpiod.a REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mtune=cortex-a72 -ftree-vectorize")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a -mtune=cortex-a72 -ftree-vectorize")

set(SRC "src")
set(INC "inc")

include_directories(${INC})

add_executable(Drongo_software
    "main.cpp"
)

add_library(rpio_classes STATIC
    "${SRC}/gpio.cpp"
    "${SRC}/spi.cpp"
)

add_library(Ads1258_class STATIC
    "${SRC}/Ads1258.cpp"
)

add_library(DataHandler_class STATIC
    "${SRC}/DataHandler.cpp"
)

add_library(WAVwriter_class STATIC
    "${SRC}/WAVwriter.cpp"
)

target_link_libraries(rpio_classes PRIVATE ${GPIOD_LIBRARY})

target_link_libraries(Ads1258_class PRIVATE rpio_classes)

target_link_libraries(DataHandler_class PRIVATE Ads1258_class WAVwriter_class)

target_link_libraries(Drongo_software PRIVATE Ads1258_class Threads::Threads easyloggingpp WAVwriter_class DataHandler_class iir_static)

# Installation rules
install(TARGETS Drongo_software DESTINATION bin)

# If you have any configuration files or other data files to install:
install(FILES Drongo.conf DESTINATION etc)

install(FILES drongo.service
        DESTINATION "/etc/systemd/system/"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# Find Doxygen Package
find_package(Doxygen REQUIRED)

# Set Doxygen options here or use a Doxygen configuration file
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# Request to configure the file
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

# Add a custom target to run Doxygen whenever the project is built
add_custom_target( doc_doxygen ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )

# Optional: if you want to install the generated documentation
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)