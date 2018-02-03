cmake_minimum_required(VERSION 3.6)

add_compile_options(-std=c++14 -pthread)

file(GLOB app_src_files
  "main.cpp"
  "deps/exokit-bindings/bindings/src/*.cc"
  "deps/exokit-bindings/canvas/src/*.cpp"
  "deps/exokit-bindings/nanosvg/src/*.cpp"
  "deps/exokit-bindings/canvascontext/src/*.cc"
  "deps/exokit-bindings/webglcontext/src/*.cc"
)

add_library( nodebinding SHARED ${app_src_files} )

include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/deps/exokit-bindings/node )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/deps/exokit-bindings/native_app_glue )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/deps/exokit-bindings/bindings/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/deps/exokit-bindings/canvas/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/deps/exokit-bindings/nanosvg/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/deps/exokit-bindings/canvascontext/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/deps/exokit-bindings/webglcontext/include )


find_library( egl-lib EGL )
find_library( glesv3-lib GLESv3 )
target_link_libraries( nodebinding ${egl-lib} ${glesv3-lib} )
