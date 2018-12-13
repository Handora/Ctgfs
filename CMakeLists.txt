# Copyright 2018 The Ctagfs Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. See the AUTHORS file for names of contributors.

cmake_minimum_required(VERSION 3.9)
project(ctgfs VERSION 1.0.0 LANGUAGES C CXX)

# This project requires C++11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-O2 -ggdb3 -std=c++11 -pthread ${CMAKE_CXX_FLAGS} -lprotobuf")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

find_package(Threads REQUIRED)
include(ExternalProject)

add_custom_target(third)

ExternalProject_Add(
    google_test
    URL https://github.com/google/googletest/archive/release-1.8.1.zip
    PREFIX ${PROJECT_SOURCE_DIR}/third_party/gtest
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_SOURCE_DIR}/third_party/gtest/build -DBUILD_SHARED_LIBS=OFF
)
add_dependencies(third google_test)

ExternalProject_Add(
    google_protobuf
    URL https://github.com/google/protobuf/archive/v3.6.1.zip
    PREFIX ${PROJECT_SOURCE_DIR}/third_party/protobuf
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND sh autogen.sh && ./configure --prefix=${PROJECT_SOURCE_DIR}/third_party/protobuf/build --disable-shared && make -j8
    INSTALL_COMMAND make install
)
add_dependencies(third google_protobuf)

set(gflags_PREFIX ${PROJECT_SOURCE_DIR}/third_party/gflags)
set(gflags_INSTALL ${PROJECT_SOURCE_DIR}/third_party/gflags/build)
set(GFLAGS_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${GFLAGS_EXTRA_COMPILER_FLAGS})
set(GFLAGS_C_FLAGS ${CMAKE_C_FLAGS} ${GFLAGS_EXTRA_COMPILER_FLAGS})

ExternalProject_Add(
    gflags
    PREFIX ${gflags_PREFIX}
    GIT_REPOSITORY "https://github.com/gflags/gflags.git"
    GIT_TAG "v2.2.2"
    UPDATE_COMMAND ""
    INSTALL_DIR ${gflags_INSTALL}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${gflags_INSTALL}
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_STATIC_LIBS=ON
    -DBUILD_PACKAGING=OFF
    -DBUILD_TESTING=OFF
    -DBUILD_NC_TESTS=OFF
    -BUILD_CONFIG_TESTS=OFF
    -DINSTALL_HEADERS=ON
    -DCMAKE_C_FLAGS=${GFLAGS_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${GFLAGS_CXX_FLAGS}
    LOG_DOWNLOAD 1
    LOG_INSTALL 1
)

set(GFLAGS_LIBRARY_DIRS ${gflags_INSTALL}/lib)

set(glog_PREFIX ${PROJECT_SOURCE_DIR}/third_party/glog)
set(glog_INSTALL ${PROJECT_SOURCE_DIR}/third_party/glog/build)
set(GLOG_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${GLOG_EXTRA_COMPILER_FLAGS})
set(GLOG_C_FLAGS ${CMAKE_C_FLAGS} ${GLOG_EXTRA_COMPILER_FLAGS})
add_dependencies(third gflags)

ExternalProject_Add(
    glogs
    DEPENDS gflags
    PREFIX ${glog_PREFIX}
    GIT_REPOSITORY "https://github.com/google/glog"
    GIT_TAG "v0.3.5"
    BUILD_IN_SOURCE true
    UPDATE_COMMAND ""
    INSTALL_DIR ${gflags_INSTALL}
    BUILD_COMMAND autoreconf --force --install && env "CFLAGS=${GLOG_C_FLAGS}" "CXXFLAGS=${GLOG_CXX_FLAGS}" ${glog_PREFIX}/src/glogs/configure --prefix=${glog_INSTALL} --enable-shared=no --enable-static=yes --with-gflags=${GFLAGS_LIBRARY_DIRS}/..
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)


add_dependencies(third glogs)

include_directories(${PROJECT_SOURCE_DIR}/src/include
                    ${PROJECT_SOURCE_DIR}/src/proto)
include_directories(${PROJECT_SOURCE_DIR}/third_party/gtest/build/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party/glog/build/include)

link_directories(
    "${PROJECT_SOURCE_DIR}/third_party/gtest/build/lib64"
    "${PROJECT_SOURCE_DIR}/third_party/glog/build/lib"
    "${PROJECT_SOURCE_DIR}/third_party/protobuf/build/lib"
)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(example)
