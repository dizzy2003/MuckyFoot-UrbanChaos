cmake_minimum_required(VERSION 3.9.0
    FATAL_ERROR)
project(OpenChaos
    VERSION 0.1.0
    DESCRIPTION "An overhaul of the Urban Chaos source code for modern operating systems."
    LANGUAGES CXX)
# Always require the latest C++ standards, without any of that extension garbage.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Global compiler flags.
set(CMAKE_CXX_FLAGS_MINSIZEREL
    ${CMAKE_CXX_FLAGS_MINSIZEREL}
    -DFINAL)
set(CMAKE_CXX_FLAGS_RELEASE
    ${CMAKE_CXX_FLAGS_RELEASE}
    -DFINAL)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
    -DFINAL)
# Optional features.
set(FALLEN_ENABLE_AUDIO ON
    CACHE BOOL
    "Whether to build support for audio playback.")
if(NOT FALLEN_ENABLE_AUDIO)
    add_definitions(-DNO_SOUND)
endif(NOT FALLEN_ENABLE_AUDIO)
# Build configuration for Windows desktop.
if(WIN32)
    # Windows-specifc optional features.
    set(FALLEN_AUDIO_ENABLE_A3D_BACKEND OFF
        CACHE BOOL
        "Whether to build support for audio playback via Aureal A3D.")
    # TODO: not sure if DirectSound support was actually implemented...
    set(FALLEN_AUDIO_ENABLE_DIRECTSOUND_BACKEND OFF
        CACHE BOOL
        "Whether to build support for audio playback via DirectSound.")
    set(FALLEN_AUDIO_ENABLE_QMDX_BACKEND OFF
        CACHE BOOL
        "Whether to build support for audio playback via QMDX.")
    if(FALLEN_ENABLE_AUDIO)
        if(FALLEN_AUDIO_ENABLE_A3D_BACKEND)
            # Locate the A3D SDK (it's out there!).
            message(STATUS "Looking for `ia3dapi.h`")
            find_path(IA3DAPI_H_PATH ia3dapi.h
                HINTS $ENV{A3D_SDK_DIR}/inc)
            if(${IA3DAPI_H_PATH} EQUAL "IA3DAPI_H_PATH-NOTFOUND")
                message(FATAL_ERROR "could not find `ia3dapi.h`")
            endif(${IA3DAPI_H_PATH} EQUAL "IA3DAPI_H_PATH-NOTFOUND")
            add_definitions(-DA3D_SOUND)
            include_directories(SYSTEM ${IA3DAPI_H_PATH})
        endif(FALLEN_AUDIO_ENABLE_A3D_BACKEND)
        if(FALLEN_AUDIO_ENABLE_QMDX_BACKEND)
            # Locate the QMDX SDK (it's out there too!).
            message(STATUS "Looking for `qmdx.h`")
            find_path(QMDX_H_PATH qmdx.h
                HINTS $ENV{QMDX_SDK_DIR}/qmdx)
            if(${QMDX_H_PATH} EQUAL "QMDX_H_PATH-NOTFOUND")
                message(FATAL_ERROR "could not find `qmdx.h`")
            endif(${QMDX_H_PATH} EQUAL "QMDX_H_PATH-NOTFOUND")
            include_directories(SYSTEM ${QMDX_H_PATH})
            add_definitions(-DQ_SOUND)
            link_directories(${QMDX_H_PATH})
        endif(FALLEN_AUDIO_ENABLE_QMDX_BACKEND)
        if(FALLEN_AUDIO_ENABLE_DIRECTSOUND_BACKEND)
            add_definitions(-DDS_SOUND)
        endif(FALLEN_AUDIO_ENABLE_DIRECTSOUND_BACKEND)
    endif(FALLEN_ENABLE_AUDIO)
    # MSVC users need to install the DirectX SDK (August 2007), as several DirectX components were
    # removed from later SDKs.
    # MinGW comes with suitable replacements so this isn't a problem for its users.
    if(MSVC)
        # Old versions of the DirectX SDK ship with an outdated version of `rpcsal.h` that
        # conflicts with the Windows SDK, so we need to find the newer version and have it override
        # the DirectX SDK's version.
        message(STATUS "Looking for `rpcsal.h`")
        find_path(RPCSAL_H_PATH rpcsal.h
            HINTS $ENV{INCLUDE} $ENV{PATH})
        if(${RPCSAL_H_PATH} EQUAL "RPCSAL_H_PATH-NOTFOUND")
            message(FATAL_ERROR "could not find `rpcsal.h`")
        endif(${RPCSAL_H_PATH} EQUAL "RPCSAL_H_PATH-NOTFOUND")
        include_directories(BEFORE SYSTEM ${RPCSAL_H_PATH})
        include_directories(AFTER SYSTEM $ENV{DXSDK_DIR}Include)
        # The DirectX libraries we're interested in come in 32- and 64-bit flavors.
        if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            set(DXSDK_LIBRARY_PATH $ENV{DXSDK_DIR}Lib/x86)
        elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            set(DXSDK_LIBRARY_PATH $ENV{DXSDK_DIR}Lib/x64)
        else(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            message(FATAL_ERROR
                "target CPU appears to be neither 32-bit nor 64-bit, which is unsupported")
        endif(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
        link_directories(${DXSDK_LIBRARY_PATH})
    endif(MSVC)
    # The dreaded `afxres.h` strikes again!
    # That's part of MFC, by the way.
    include(FindMFC)
    if(NOT ${MFC_FOUND})
        message(FATAL_ERROR "MFC not found; ensure that MFC is installed and try again")
    endif(NOT ${MFC_FOUND})
endif(WIN32)
# Rust bails out during the linker phase if you try to cross-compile under a Visual Studio command
# prompt. Just build the Rust libs manually then run CMake...
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(OPENCHAOS_FFI_DIR $ENV{OPENCHAOS_RUST_OUTPUT_DIR}/debug)
else(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(OPENCHAOS_FFI_DIR $ENV{OPENCHAOS_RUST_OUTPUT_DIR}/release)
endif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
link_directories(${OPENCHAOS_FFI_DIR})
# Build the C++ code.
add_subdirectory(MFStdLib)
add_subdirectory(fallen)
