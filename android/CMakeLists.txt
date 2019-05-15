cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Werror")
add_library(native-main SHARED ${CMAKE_SOURCE_DIR}/../main.cpp ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/../node_modules/nan )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/../deps/exokit-bindings/util/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/../.node-gyp/12.2.0/include/node )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/../build/libexokit )
include_directories( SYSTEM ${ANDROID_NDK}/sources/android/native_app_glue )

add_library( exokit STATIC IMPORTED )
set_target_properties( exokit PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../build/libexokit/libexokit.a )

add_library( node STATIC IMPORTED )
set_target_properties( node PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/libnode.a-android/libnode.a )

add_library( vrapi SHARED IMPORTED )
set_target_properties( vrapi PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-oculus-mobile-deps/lib/android/arm64-v8a/libvrapi.so )

add_library( skia STATIC IMPORTED )
set_target_properties( skia PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-canvas-deps/lib2/android/libskia.a )

add_library( labsound STATIC IMPORTED )
set_target_properties( labsound PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-audio-deps/lib2/magicleap/libLabSound.a )

add_library( avformat STATIC IMPORTED )
set_target_properties( avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/magicleap/libavformat.a )
add_library( avcodec STATIC IMPORTED )
set_target_properties( avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/magicleap/libavcodec.a )
add_library( avutil STATIC IMPORTED )
set_target_properties( avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/magicleap/libavutil.a )
add_library( avdevice STATIC IMPORTED )
set_target_properties( avdevice PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/magicleap/libavdevice.a )
add_library( swscale STATIC IMPORTED )
set_target_properties( swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/magicleap/libswscale.a )
add_library( swresample STATIC IMPORTED )
set_target_properties( swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/magicleap/libswresample.a )
add_library( opus STATIC IMPORTED )
set_target_properties( opus PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-video-deps/lib2/magicleap/libopus.a )

add_library( webrtc STATIC IMPORTED )
set_target_properties( webrtc PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../node_modules/native-webrtc-deps/lib2/magicleap/libwebrtc.a )

target_link_libraries(native-main
    android
    log
    EGL
    GLESv1_CM
    GLESv2
    GLESv3
    vrapi
    exokit
    node
    skia
    labsound
    avformat
    avcodec
    avutil
    avdevice
    swscale
    swresample
    opus
    webrtc
    OpenSLES
)
