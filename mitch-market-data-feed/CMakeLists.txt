cmake_minimum_required(VERSION 3.20)

project(MitchFeedHandler VERSION 0.1)

set(EXECUTABLE ${PROJECT_NAME}-bin)

# GNU Defaults
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(DEFAULT_BUILD_TYPE "Release")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set compiler flags
set(GCC_COVERAGE_COMPILE_FLAGS "-pipe -std=c++2a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -lboost_system  -lcrypto -lssl -levent_core")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS} -ggdb -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CXX_FLAGS
        -fPIC
        -Wall
        -D__STDC_FORMAT_MACROS
        -Wno-unused-parameter
        -Wno-unused-function
        -Wunused-variable
        -Wunused-value
        -Wshadow
        -Wcast-qual
        -Wcast-align
        -Wwrite-strings
        -Wsign-compare
        -Winvalid-pch
        -fms-extensions
        -Wfloat-equal
        -Wextra
        -std=c++2a
        )

# Include src
file(GLOB_RECURSE ${PROJECT_NAME}_APP_SRC CONFIGURE_DEPENDS "Application/*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_Decoder_SRC CONFIGURE_DEPENDS "Decoder/*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_FeedHandler_SRC CONFIGURE_DEPENDS "FeedHandler/*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_Connection_SRC CONFIGURE_DEPENDS "Connection/*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_Utils_SRC CONFIGURE_DEPENDS "Utils/*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_EventProcessingQueue_SRC CONFIGURE_DEPENDS "EventProcessingQueue/*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_Message_SRC CONFIGURE_DEPENDS "Message/*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_SubscriptionManager_SRC CONFIGURE_DEPENDS "SubscriptionManager/*.cpp")

set(${PROJECT_NAME}_SRC ${${PROJECT_NAME}_APP_SRC}
        ${${PROJECT_NAME}_Decoder_SRC}
        ${${PROJECT_NAME}_FeedHandler_SRC}
        ${${PROJECT_NAME}_Connection_SRC}
        ${${PROJECT_NAME}_Utils_SRC}
        ${${PROJECT_NAME}_EventProcessingQueue_SRC}
        ${${PROJECT_NAME}_Message_SRC}
        ${${PROJECT_NAME}_SubscriptionManager_SRC})


add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC})
add_executable(${EXECUTABLE} ${${PROJECT_NAME}_SRC})

#enable/disable log calls at compile time according to global level.
#SPDLOG_LEVEL_TRACE
#SPDLOG_LEVEL_DEBUG
#SPDLOG_LEVEL_INFO
#SPDLOG_LEVEL_WARN
#SPDLOG_LEVEL_ERROR
#SPDLOG_LEVEL_CRITICAL
#SPDLOG_LEVEL_OFF

#If debug, do not optimise away any logging macro.
#If release, optimise away all macros under logging level INFO.
add_compile_definitions($<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>$<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>)

# Include dirs
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type d -name "include" ! -path ${CMAKE_CURRENT_SOURCE_DIR}/lib/* -printf "%p " OUTPUT_VARIABLE INCLUDE_DIRS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(INCLUDE_DIRS UNIX_COMMAND ${INCLUDE_DIRS})

foreach (INCLUDE_DIR ${INCLUDE_DIRS})
    message(STATUS "Including Directory: " ${INCLUDE_DIR}/)
    target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${INCLUDE_DIR}/>")
    target_include_directories(${EXECUTABLE} PRIVATE ${INCLUDE_DIR})
endforeach ()
target_include_directories(${PROJECT_NAME} PUBLIC "$<INSTALL_INTERFACE:${PROJECT_NAME}/${CMAKE_INSTALL_INCLUDE_DIR}>")

# Link libraries
find_package(OpenSSL REQUIRED)
target_link_libraries(${EXECUTABLE} PRIVATE OpenSSL::SSL OpenSSL::Crypto)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${EXECUTABLE} PRIVATE Threads::Threads)

find_library(LIBRT rt REQUIRED)
target_link_Libraries(${PROJECT_NAME} PRIVATE ${LIBRT})
target_link_Libraries(${EXECUTABLE} PRIVATE ${LIBRT})

#No longer used.
#find_library(EVPP_LIB NAMES evpp)
#target_link_Libraries(${PROJECT_NAME} PRIVATE ${EVPP_LIB})
#target_link_Libraries(${EXECUTABLE} PRIVATE ${EVPP_LIB})

#mitch-md-feed itself uses spdlog, but EVPP refuses to work without glog installed.
#find_package(glog REQUIRED)
#target_link_libraries(${PROJECT_NAME} PRIVATE glog::glog)
#target_link_libraries(${EXECUTABLE} PRIVATE glog::glog)

#spdlog works for now, if we want lower latency we can go for nanolog in the future
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${EXECUTABLE} PRIVATE spdlog::spdlog)

find_package(Boost 1.55 REQUIRED COMPONENTS serialization)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
target_link_libraries(${EXECUTABLE} PRIVATE ${Boost_LIBRARIES})
target_include_directories(${EXECUTABLE} PRIVATE ${Boost_INCLUDE_DIRS})

# Install
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_TARGETS
        LIBRARY DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR}
        )

foreach (INCLUDE_DIR ${INCLUDE_DIRS})
    install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR})
endforeach ()
install(EXPORT ${PROJECT_NAME}_TARGETS
        FILE ${PROJECT_NAME}_TARGETS.cmake
        DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION $${PROJECT_NAME}/{CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )

set(version 1.0.0)
set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${version})
set_property(TARGET ${PROJECT_NAME} PROPERTY SOVERSION 1)
set_property(TARGET ${PROJECT_NAME} PROPERTY
        INTERFACE_${PROJECT_NAME}_MAJOR_VERSION 1)

set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING ${PROJECT_NAME}_MAJOR_VERSION
        )

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION "${version}"
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )
#include(thirdparty/evpp/CMakeLists.txt)
