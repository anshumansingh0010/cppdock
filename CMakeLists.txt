cmake_minimum_required(VERSION 3.20)

project(cppdock
    VERSION 0.1.0
    LANGUAGES CXX
)

# =========================
# Global configuration
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler warnings and sanitizers
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
    -fno-omit-frame-pointer
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fsanitize=undefined -g)
    add_link_options(-fsanitize=address -fsanitize=undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native)
endif()

# Linux-only guard
if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "cppdock supports Linux only")
endif()

# =========================
# Dependencies
# =========================
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAP REQUIRED libcap)
pkg_check_modules(LIBSECCOMP REQUIRED libseccomp)

# =========================
# Core Library
# =========================
add_library(cppdock_core
    src/common/Logger.cpp
    src/common/Utils.cpp
    src/runtime/Cgroup.cpp
    src/runtime/Container.cpp
    src/runtime/Namespace.cpp
    src/runtime/Process.cpp
    src/runtime/RootFS.cpp
    src/daemon/Daemon.cpp
    src/daemon/RequestHandler.cpp
    src/daemon/SocketServer.cpp
    src/network/NetNS.cpp
    src/network/Veth.cpp
    src/security/Capabilities.cpp
    src/security/Seccomp.cpp
)
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

target_include_directories(cppdock_core PUBLIC 
    include
    ${LIBCAP_INCLUDE_DIRS}
    ${LIBSECCOMP_INCLUDE_DIRS}
    ${INCLUDE_DIR}
)

target_compile_definitions(cppdock_core PUBLIC _GNU_SOURCE)
target_link_libraries(cppdock_core PUBLIC 
    pthread
    ${LIBCAP_LIBRARIES}
    ${LIBSECCOMP_LIBRARIES}
)

# =========================
# Executables
# =========================

# CLI
add_executable(cppdock src/cmd/cppdock/main.cpp)
target_link_libraries(cppdock PRIVATE cppdock_core)

# Daemon
add_executable(cppdockd src/cmd/cppdockd/main.cpp)
target_link_libraries(cppdockd PRIVATE cppdock_core)

# Output directory for binaries
set_target_properties(cppdock cppdockd PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# =========================
# Testing
# =========================
enable_testing()
# add_subdirectory(tests) # Uncomment when tests are added
