cmake_minimum_required(VERSION 3.16.3)

project(SpinLock VERSION 0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include src

file(GLOB_RECURSE ${PROJECT_NAME}_MAIN_SRC CONFIGURE_DEPENDS "main.cpp")

set(${PROJECT_NAME}_SRC 
    ${${PROJECT_NAME}_MAIN_SRC} 
   )

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

# Include dirs
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type d -name "include" ! -path ${CMAKE_CURRENT_SOURCE_DIR}/lib/* -printf "%p " OUTPUT_VARIABLE SpinLock_INC_DIRS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(SpinLock_INC_DIRS UNIX_COMMAND ${SpinLock_INC_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${SpinLock_INC_DIRS})

list (APPEND  CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

find_library(LIBRT rt REQUIRED)
target_link_Libraries(${PROJECT_NAME} PRIVATE ${LIBRT})

# Set compiler flags
set(GCC_COVERAGE_COMPILE_FLAGS "-pipe -std=c++17")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS} -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

