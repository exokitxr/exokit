#
# Copyright (C) The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.10.2)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Werror")


add_library(native-main SHARED ${CMAKE_SOURCE_DIR}/../main.cpp)


add_library( exokit STATIC IMPORTED )
set_target_properties( exokit PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../build/libexokit/libexokit.a )

add_library( node STATIC IMPORTED )
set_target_properties( node PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/libnode.a/libnode.a )

add_library( skia STATIC IMPORTED )
set_target_properties( skia PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-canvas-deps/lib2/android/libskia.a )


add_library( avformat STATIC IMPORTED )
set_target_properties( avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/android/libavformat.a )
add_library( avcodec STATIC IMPORTED )
set_target_properties( avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/android/libavcodec.a )
add_library( avutil STATIC IMPORTED )
set_target_properties( avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/android/libavutil.a )
add_library( avdevice STATIC IMPORTED )
set_target_properties( avdevice PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/android/libavdevice.a )
add_library( swscale STATIC IMPORTED )
set_target_properties( swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/android/libswscale.a )
add_library( swresample STATIC IMPORTED )
set_target_properties( swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/android/libswresample.a )
add_library( opus STATIC IMPORTED )
set_target_properties( opus PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/android/libopus.a )


include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/../node_modules/nan )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/../deps/exokit-bindings/util/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/../.node-gyp/11.10.1/include/node )


# add lib dependencies
target_link_libraries(
    native-main
    android
    EGL
    GLESv1_CM
    GLESv2
    GLESv3
    log
    exokit
    node
    skia
    avformat
    avcodec
    avutil
    avdevice
    swscale
    swresample
    opus
    OpenSLES
        )
