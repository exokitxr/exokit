# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# Source files.
#

# cefsimple sources.
FILE( GLOB CEFSIMPLE_SRCS "*.cc" "*.cpp" "*.h")
source_group(cefsimple FILES ${CEFSIMPLE_SRCS})

set(CEFSIMPLE_SRCS
  ${CEFSIMPLE_SRCS}
  ${CEFSIMPLE_RES_SRCS}
  )

# cefsimple helper sources.
set(CEFSIMPLE_HELPER_SRCS_MACOSX
  process_helper_mac.cc
  )
APPEND_PLATFORM_SOURCES(CEFSIMPLE_HELPER_SRCS)
source_group(cefsimple FILES ${CEFSIMPLE_HELPER_SRCS})

#
# Shared configuration.
#

# Target executable names.
set(CEF_TARGET "cefsimple")

# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()

#########################################################
SET( CEFSIMPLE_EXT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ext )

MACRO( APPEND_LINK_LIBS )
	LIST( APPEND CEFSIMPLE_LINK_LIBS ${ARGN} )
ENDMACRO( APPEND_LINK_LIBS )

# Add glew
INCLUDE_DIRECTORIES( ${CEFSIMPLE_EXT_DIR}/glew/include )
APPEND_LINK_LIBS( libglew_shared )

# Add glfw
INCLUDE_DIRECTORIES( ${CEFSIMPLE_EXT_DIR}/glfw/include )
APPEND_LINK_LIBS( glfw )

# Add glm
INCLUDE_DIRECTORIES( ${CEFSIMPLE_EXT_DIR}/glm )

#########################################################

# glew는 디버그/릴리즈에서 다른 이름의 dll 파일이 생성된다
# 근데 파일이 없으면 cmake 복사가 제대로 돌아가지 않더라
# 그래서 가짜 파일 만들어서 돌아가도록 만듬
# TODO Debug/Release 를 구분할수 있는 더 좋은 방법 있으면 바꿀수 있을텐데
MACRO(COPY_FILE_FORCE target source_file target_dir)
  SET(GLEW_DLL_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>")
  SET(GLEW_DLL_DIR_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug")
  SET(GLEW_DLL_DIR_RELEASE "${CMAKE_BINARY_DIR}/bin/Release")
  
  SET(GLEW_DLL_DIRS ${GLEW_DLL_DIR_DEBUG} ${GLEW_DLL_DIR_RELEASE})
  FOREACH(DIR ${GLEW_DLL_DIRS})
    ADD_CUSTOM_COMMAND(
      TARGET ${target}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${DIR}
      VERBATIM
    )
	ADD_CUSTOM_COMMAND(
      TARGET ${target}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E touch "${DIR}/${source_file}"
      VERBATIM
    )
  ENDFOREACH()
  
  ADD_CUSTOM_COMMAND(
    TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GLEW_DLL_DIR}/${source_file}" ${target_dir}
    VERBATIM
  )
ENDMACRO()


#
# Windows configuration.
#

if(OS_WINDOWS)
  # Executable target.
  #add_executable(${CEF_TARGET} WIN32 ${CEFSIMPLE_SRCS})
  add_executable(${CEF_TARGET} ${CEFSIMPLE_SRCS})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  
  TARGET_LINK_LIBRARIES( ${CEF_TARGET} ${CEFSIMPLE_LINK_LIBS} )

  if(USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
  endif()

  # Add the custom manifest files to the executable.
  ADD_WINDOWS_MANIFEST("${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET}")

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
  
  # copy glew dll  
  COPY_FILE_FORCE(${CEF_TARGET} "glewd.dll" ${CEF_TARGET_OUT_DIR})
  COPY_FILE_FORCE(${CEF_TARGET} "glew.dll" ${CEF_TARGET_OUT_DIR})
  
  # copy shader
  COPY_FILES("${CEF_TARGET}" "shaders" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET_OUT_DIR}/..")
endif()

