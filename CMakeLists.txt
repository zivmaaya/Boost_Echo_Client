cmake_minimum_required(VERSION 3.15)
project(Boost_Echo_Client)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

link_libraries("-lboost_system")

add_executable(Boost_Echo_Client
        include/connectionHandler.h
        include/bookClubClient.h
        src/connectionHandler.cpp
        src/bookClubClient.cpp
        src/echoClient.cpp
        src/stompClient.cpp
        )
