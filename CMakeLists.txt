# Copyright 2018 The Ctagfs Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. See the AUTHORS file for names of contributors.

cmake_minimum_required(VERSION 3.5)
project(ctgfs VERSION 1.0.0 LANGUAGES C CXX)

# This project requires C++11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-ggdb3 -std=c++11 -pthread ${CMAKE_CXX_FLAGS} -lprotobuf -luuid -Wall -ltcmalloc_minimal -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=25")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

include_directories(${PROJECT_SOURCE_DIR}/src/include
                    ${PROJECT_SOURCE_DIR}/src/proto)

include_directories(${PROJECT_SOURCE_DIR}/third_party/gtest/googletest/include)
link_directories(/usr/local/lib)
enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(example)
add_subdirectory(third_party)
