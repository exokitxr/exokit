cmake_minimum_required(VERSION 2.8.7)
project(sjpeg CXX)

# Options for coder / decoder executables.
option(SJPEG_ENABLE_SIMD "Enable any SIMD optimization." ON)
option(SJPEG_BUILD_EXAMPLES "Build the sjpeg / vjpeg command line tools." ON)

set(SJPEG_DEP_LIBRARIES)
set(SJPEG_DEP_INCLUDE_DIRS)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE
    "Build type: Release, Debug or RelWithDebInfo" STRING FORCE
  )
endif()

set(PROJECT_VERSION 0.1)

################################################################################
# Android only.
if(ANDROID)
  include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
  add_library(cpufeatures STATIC
              ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c
  )
  target_link_libraries(cpufeatures dl)
  set(SJPEG_DEP_LIBRARIES ${SJPEG_DEP_LIBRARIES} cpufeatures)
  set(SJPEG_DEP_INCLUDE_DIRS ${SJPEG_DEP_INCLUDE_DIRS}
      ${ANDROID_NDK}/sources/android/cpufeatures
  )
endif()

## Check for SIMD extensions.
include(${CMAKE_CURRENT_LIST_DIR}/cmake/cpu.cmake)

################################################################################
# sjpeg source files.

# Build the sjpeg library.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ ${SJPEG_DEP_INCLUDE_DIRS})
add_library(sjpeg ${CMAKE_CURRENT_SOURCE_DIR}/src/bit_writer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bit_writer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/colors_rgb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dichotomy.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/enc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fdct.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/headers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/jpeg_tools.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/score_7.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sjpeg.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sjpegi.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/yuv_convert.cc
)
if(SJPEG_DEP_LIBRARIES)
  target_link_libraries(sjpeg ${SJPEG_DEP_LIBRARIES})
endif()

# Make sure the OBJECT libraries are built with position independent code
# (it is not ON by default).
set_target_properties(sjpeg PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Set the version numbers.
set_target_properties(sjpeg PROPERTIES VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION})

# Find the standard image libraries.
set(SJPEG_DEP_IMG_LIBRARIES)
set(SJPEG_DEP_IMG_INCLUDE_DIRS)
foreach(I_LIB PNG JPEG)
  find_package(${I_LIB})
  set(SJPEG_HAVE_${I_LIB} ${${I_LIB}_FOUND})
  if(${I_LIB}_FOUND)
    set(SJPEG_DEP_IMG_LIBRARIES ${SJPEG_DEP_IMG_LIBRARIES}
        ${${I_LIB}_LIBRARIES})
    set(SJPEG_DEP_IMG_INCLUDE_DIRS ${SJPEG_DEP_IMG_INCLUDE_DIRS}
        ${${I_LIB}_INCLUDE_DIRS})
  endif()
endforeach()

# Find the OpenGL/GLUT libraries.
set(SJPEG_DEP_GL_LIBRARIES)
set(SJPEG_DEP_GL_INCLUDE_DIRS)
find_package(OpenGL)
if(OPENGL_gl_LIBRARY)
  set(SJPEG_DEP_GL_LIBRARIES
      ${SJPEG_DEP_GL_LIBRARIES} ${OPENGL_gl_LIBRARY})
  set(SJPEG_DEP_GL_INCLUDE_DIRS
      ${SJPEG_DEP_GL_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
  set(SJPEG_HAVE_OPENGL TRUE)
endif()
find_package(GLUT)
if(GLUT_FOUND)
  set(SJPEG_DEP_GL_LIBRARIES
      ${SJPEG_DEP_GL_LIBRARIES} ${GLUT_glut_LIBRARY})
  set(SJPEG_DEP_GL_INCLUDE_DIRS
      ${SJPEG_DEP_GL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR})
  set(SJPEG_HAVE_GLUT TRUE)
endif()

# build the utils library
include_directories(${SJPEG_DEP_IMG_INCLUDE_DIRS})
add_library(utils ${CMAKE_CURRENT_SOURCE_DIR}/examples/utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/utils.h
)
if(WIN32)
  # quiet warnings related to fopen, sscanf
  target_compile_definitions(utils PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
if(SJPEG_HAVE_OPENGL)
  # check pthread for GL libraries
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads)
  if(Threads_FOUND)
    if(CMAKE_USE_PTHREADS_INIT)
      list(APPEND SJPEG_DEP_GL_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    endif()
  endif()
  target_compile_definitions(utils PUBLIC SJPEG_HAVE_OPENGL)
  add_definitions(${OPENGL_DEFINITIONS})
endif()
if(SJPEG_HAVE_GLUT)
  add_definitions(${GLUT_DEFINITIONS})
endif()
if(SJPEG_HAVE_JPEG)
  target_compile_definitions(utils PUBLIC SJPEG_HAVE_JPEG)
endif()
if(SJPEG_HAVE_PNG)
  target_compile_definitions(utils PUBLIC SJPEG_HAVE_PNG)
endif()
if(SJPEG_DEP_IMG_LIBRARIES)
  # check pthread for GL libraries
  target_link_libraries(utils ${SJPEG_DEP_IMG_LIBRARIES}
                        ${SJPEG_DEP_GL_LIBRARIES})
endif()
# set_target_properties(utils PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Build the executables if asked for.
if(SJPEG_BUILD_EXAMPLES)
  # sjpeg
  add_executable(sjpeg-bin ${CMAKE_CURRENT_SOURCE_DIR}/examples/sjpeg.cc)
  target_link_libraries(sjpeg-bin sjpeg utils)
  set_target_properties(sjpeg-bin PROPERTIES OUTPUT_NAME sjpeg)

  # vjpeg
  include_directories(${SJPEG_DEP_GL_INCLUDE_DIRS})
  add_executable(vjpeg ${CMAKE_CURRENT_SOURCE_DIR}/examples/vjpeg.cc)
  # Force to link against pthread.
  include(CheckCXXSourceCompiles)
  set(CMAKE_REQUIRED_FLAGS_INI ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "-Wl,--no-as-needed")
  check_cxx_source_compiles("int main(void){return 0;}" FLAG_NO_AS_NEEDED)
  set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_INI})
  if(FLAG_NO_AS_NEEDED)
    target_link_libraries(vjpeg "-Wl,--no-as-needed")
  endif()

  # check whether we need to include GLUT/glut.h or GL/glut.h
  include(CheckIncludeFileCXX)
  check_include_file_cxx(GLUT/glut.h HAVE_GLUT_GLUT_H)
  check_include_file_cxx(GL/glut.h HAVE_GL_GLUT_H)
  if(HAVE_GLUT_GLUT_H)
    add_definitions(-DHAVE_GLUT_GLUT_H)
  elseif(HAVE_GL_GLUT_H)
    add_definitions(-DHAVE_GL_GLUT_H)
  endif()
  target_link_libraries(vjpeg ${SJPEG_DEP_GL_LIBRARIES} sjpeg utils)

  install(TARGETS sjpeg-bin vjpeg RUNTIME DESTINATION bin)
endif()

# Install the different headers and libraries.
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/sjpeg.h
        DESTINATION include)
install(TARGETS sjpeg
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# Create the CMake version file.
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/sjpegConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Create the Config file.
include(CMakePackageConfigHelpers)
set(ConfigPackageLocation lib/sjpeg/cmake/)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sjpegConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/sjpegConfig.cmake
  INSTALL_DESTINATION ${ConfigPackageLocation}
)

# Install the generated CMake files.
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/sjpegConfigVersion.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/sjpegConfig.cmake"
  DESTINATION ${ConfigPackageLocation}
)


################################################################################
# Man page.
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/sjpeg.1
              ${CMAKE_CURRENT_SOURCE_DIR}/man/vjpeg.1
  DESTINATION share/man/man1)
