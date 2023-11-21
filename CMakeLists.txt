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
    inih
    GIT_REPOSITORY https://github.com/benhoyt/inih.git
    GIT_TAG r57
)

FetchContent_GetProperties(inih)

if(NOT inih_POPULATED)
    FetchContent_Populate(inih)

    # Adding Easylogging++ directly to the build. This defines the EASYLOGGINGPP target.
    include_directories("${inih_SOURCE_DIR}" "${inih_SOURCE_DIR}/cpp")

    add_library(inih STATIC
        "${inih_SOURCE_DIR}/ini.c"
        "${inih_SOURCE_DIR}/cpp/INIReader.cpp"
    )
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

find_library(GPIOD_LIBRARY NAMES gpiod REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mtune=cortex-a72")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a -mtune=cortex-a72")

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

target_link_libraries(rpio_classes PRIVATE ${GPIOD_LIBRARY})

add_library(Ads1258_class STATIC
    "${SRC}/Ads1258.cpp"
)

add_library(DataHandler_class STATIC
    "${SRC}/DataHandler.cpp"
)

add_library(WAVwriter_class STATIC
    "${SRC}/WAVwriter.cpp"
)

target_link_libraries(Ads1258_class PRIVATE rpio_classes)

target_link_libraries(DataHandler_class PRIVATE Ads1258_class WAVwriter_class)

target_link_libraries(Drongo_software PRIVATE  Ads1258_class Threads::Threads easyloggingpp inih WAVwriter_class DataHandler_class iir_static)