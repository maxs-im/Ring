cmake_minimum_required(VERSION 3.14)
project(Ring)

set(CMAKE_CXX_STANDARD 17)

add_executable(Ring main.cpp)

find_package(Boost 1.65.1.0 REQUIRED system)
message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )       # define HAS_BOOST

    set(Boost_USE_STATIC_LIBS        OFF)
    set(Boost_USE_MULTITHREADED      ON)
    set(Boost_USE_STATIC_RUNTIME     OFF)
    set(BOOST_ALL_DYN_LINK           ON)   # force dynamic linking for all libraries

    target_link_libraries(Ring ${Boost_LIBRARIES})
ENDIF()