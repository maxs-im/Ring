cmake_minimum_required(VERSION 3.14)
project(Ring)

set(CMAKE_CXX_STANDARD 17)

#       include Google Tests
add_executable(Tests Tests/tests_main.cpp Tests/ntf.cpp utils/notification.cpp)

add_subdirectory(lib/googletest-master)
include_directories(lib/googletest-master/googletest/include)
include_directories(lib/googletest-master/googlemock/include)

target_link_libraries(Tests gtest gtest_main)
# ------------------------------------------------------------

#       include applications code
include_directories("utils/")

add_executable(Client client_main.cpp utils/client.cpp utils/notification.cpp)
add_executable(Server server_main.cpp utils/server.cpp utils/notification.cpp)
# ------------------------------------------------------------

#       include boost library
find_package(Boost 1.65.1.0 REQUIRED system thread)
message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )       # define HAS_BOOST

    set(Boost_USE_STATIC_LIBS        OFF)
    set(Boost_USE_MULTITHREADED      ON)
    set(Boost_USE_STATIC_RUNTIME     OFF)
    set(BOOST_ALL_DYN_LINK           ON)   # force dynamic linking for all libraries

    target_link_libraries(Client ${Boost_LIBRARIES})
    target_link_libraries(Server ${Boost_LIBRARIES})
ENDIF()
# ------------------------------------------------------------