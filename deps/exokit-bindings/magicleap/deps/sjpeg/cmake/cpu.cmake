## Check for SIMD extensions.

function(sjpeg_check_compiler_flag SJPEG_SIMD_DEFINE ENABLE_SIMD)
  if(NOT ENABLE_SIMD)
    message(STATUS "Disabling ${SJPEG_SIMD_DEFINE} optimization.")
    set(SJPEG_HAVE_${SJPEG_SIMD_DEFINE} 0 PARENT_SCOPE)
    return()
  endif()
  unset(SJPEG_HAVE_FLAG_${SJPEG_SIMD_DEFINE} CACHE)
  check_cxx_source_compiles("
      int main(void) {
        #if !defined(${SJPEG_SIMD_DEFINE})
        this is not valid code
        #endif
        return 0;
      }
    " SJPEG_HAVE_FLAG_${SJPEG_SIMD_DEFINE}
  )
  if(SJPEG_HAVE_FLAG_${SJPEG_SIMD_DEFINE})
    set(SJPEG_HAVE_${SJPEG_SIMD_DEFINE} 1 PARENT_SCOPE)
  else()
    set(SJPEG_HAVE_${SJPEG_SIMD_DEFINE} 0 PARENT_SCOPE)
  endif()
endfunction()

include(CheckCXXSourceCompiles)

# those are included in ifdef's in c++ code.
set(SJPEG_SIMD_DEFINES "__SSE2__;__ARM_NEON__")
if(MSVC)
  set(SIMD_ENABLE_FLAGS "/arch:SSE2;")
  set(SIMD_DISABLE_FLAGS)
else()
  set(SIMD_ENABLE_FLAGS "-msse2;-mfpu=neon")
  set(SIMD_DISABLE_FLAGS "-mno-sse2;")
endif()

if(${ANDROID})
  if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    # This is because Android studio uses the configuration
    # "-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
    # that does not trigger neon optimizations but should
    # (as this configuration does not exist anymore).
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon ")
  endif()
endif()

list(LENGTH SJPEG_SIMD_DEFINES SJPEG_SIMD_DEFINES_LENGTH)
math(EXPR SJPEG_SIMD_DEFINES_RANGE "${SJPEG_SIMD_DEFINES_LENGTH} - 1")

foreach(I_SIMD RANGE ${SJPEG_SIMD_DEFINES_RANGE})
  list(GET SJPEG_SIMD_DEFINES ${I_SIMD} SJPEG_SIMD_DEFINE)

  # First try with no extra flag added as the compiler might have default flags
  # (especially on Android).
  unset(SJPEG_HAVE_${SJPEG_SIMD_DEFINE} CACHE)
  set(CMAKE_REQUIRED_FLAGS_INI ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS)
  sjpeg_check_compiler_flag(${SJPEG_SIMD_DEFINE} ${SJPEG_ENABLE_SIMD})
  if(NOT SJPEG_HAVE_${SJPEG_SIMD_DEFINE})
    list(GET SIMD_ENABLE_FLAGS ${I_SIMD} SIMD_COMPILE_FLAG)
    set(CMAKE_REQUIRED_FLAGS ${SIMD_COMPILE_FLAG})
    sjpeg_check_compiler_flag(${SJPEG_SIMD_DEFINE} ${SJPEG_ENABLE_SIMD})
  else()
    set(SIMD_COMPILE_FLAG " ")
  endif()
  set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_INI})
  # Check which files we should include or not.
  if(SJPEG_HAVE_${SJPEG_SIMD_DEFINE})
    # Add the flags to the compiler for all files.
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${SIMD_COMPILE_FLAG}")
  else()
    # Explicitly disable SIMD.
    if(SIMD_DISABLE_FLAGS)
      list(GET SIMD_DISABLE_FLAGS ${I_SIMD} SIMD_COMPILE_FLAG)
      include(CheckCXXCompilerFlag)
      if(SIMD_COMPILE_FLAG)
        unset(HAS_COMPILE_FLAG CACHE)
        check_cxx_compiler_flag(${SIMD_COMPILE_FLAG} HAS_COMPILE_FLAG)
        if(HAS_COMPILE_FLAG)
          # Do one more check for Clang to circumvent CMake issue 13194.
          if(COMMAND check_compiler_flag_common_patterns)
            # Only in CMake 3.0 and above.
            check_compiler_flag_common_patterns(COMMON_PATTERNS)
          else()
            set(COMMON_PATTERNS)
          endif()
          set(CMAKE_REQUIRED_DEFINITIONS ${SIMD_COMPILE_FLAG})
          check_cxx_source_compiles("int main(void) {return 0;}" FLAG2
            FAIL_REGEX "warning: argument unused during compilation:"
            ${COMMON_PATTERNS}
          )
          if(NOT FLAG2)
            unset(HAS_COMPILE_FLAG CACHE)
          endif()
        endif()
        if(HAS_COMPILE_FLAG)
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SIMD_COMPILE_FLAG}")
        endif()
      endif()
    endif()
  endif()
endforeach()
