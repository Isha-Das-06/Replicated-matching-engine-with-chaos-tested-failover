cmake_minimum_required(VERSION 3.14)
project(RaftMatchingEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# For now, we will add an executable for the standalone matching engine
add_executable(matching_engine_standalone 
    matching_engine/main.cpp
    matching_engine/OrderBook.cpp
    matching_engine/MatchingEngine.cpp
)

add_executable(matching_engine_benchmark
    client/benchmark_main.cpp
    client/LobsterParser.cpp
    client/ReplayClient.cpp
    matching_engine/OrderBook.cpp
    matching_engine/MatchingEngine.cpp
)

add_executable(matching_engine_chaos
    consensus/chaos_main.cpp
    consensus/Node.cpp
    consensus/MiniRaft.cpp
    client/LobsterParser.cpp
    matching_engine/OrderBook.cpp
    matching_engine/MatchingEngine.cpp
)


# Phase 2 will include NuRaft
