cmake_minimum_required(VERSION 3.16.3)

project(FastFormat VERSION 0.1)

set(EXECUTABLE ${PROJECT_NAME})

# GNU Defaults
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(DEFAULT_BUILD_TYPE "Release")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set compiler flags
set(GCC_COVERAGE_COMPILE_FLAGS "-pipe -std=c++17")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS} -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)



# Include src
file(GLOB_RECURSE ${PROJECT_NAME}_APP_SRC CONFIGURE_DEPENDS "main.cpp")

# set the venue file that we want to compile 

set(${PROJECT_NAME}_SRC 
    ${${PROJECT_NAME}_APP_SRC}  
   )
add_executable(${EXECUTABLE} ${${PROJECT_NAME}_SRC})

