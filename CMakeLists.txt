cmake_minimum_required ( VERSION 2.8.12 )

project ( oxide )

macro(get_WIN32_WINNT version)
    if (WIN32 AND CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" ver ${ver})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

        set(${version} "0x${ver}")
    endif()
endmacro()

# Tell CMake where to find additional find modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/" )

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(OS_MACOSX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(OS_LINUX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(OS_WINDOWS 1)
endif()

# Import macros
include ( CheckCXXCompilerFlag )

# C++11 Requirement for Linux
if ( OS_POSIX )
    check_cxx_compiler_flag ( "-std=c++11" COMPILER_SUPPORTS_CXX11 )
    check_cxx_compiler_flag ( "-std=c++0x" COMPILER_SUPPORTS_CXX0X )
    set ( CMAKE_CXX_COMPILER_ARG1 "-std=c++11" )

    if ( COMPILER_SUPPORTS_CXX11 )
        set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11" )
    elseif ( COMPILER_SUPPORTS_CXX0X )
        set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++0x" )
    else ( )
        message ( STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif ( )
    
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wno-deprecated-declarations" )  # Optimize
    include_directories ( "/usr/local/include" )
else ( )
    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})

    if ( MINGW )
        check_cxx_compiler_flag ( "-std=c++11" COMPILER_SUPPORTS_CXX11 )
        check_cxx_compiler_flag ( "-std=c++0x" COMPILER_SUPPORTS_CXX0X )
        set ( CMAKE_CXX_COMPILER_ARG1 "-std=c++11" )
    
        if ( COMPILER_SUPPORTS_CXX11 )
            set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
        elseif ( COMPILER_SUPPORTS_CXX0X )
            set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )
        else ( )
            message ( STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
        endif ( )
        
        set ( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++" )
    else ( )
        add_definitions ( "/wd4244 /wd4996" )
    endif ( )
endif ( )

find_package ( FlatBuffers )

if ( NOT FLATBUFFERS_FOUND )
    message( FATAL_ERROR "Could not find FlatBuffers. Cannot continue" )
endif()
include_directories ( FLATBUFFERS_INCLUDE_DIR )

add_subdirectory ( client )
