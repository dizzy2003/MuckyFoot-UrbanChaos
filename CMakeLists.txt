cmake_minimum_required(
    VERSION 3.9
    FATAL_ERROR
)

project(
    UrbanChaos
    VERSION 1.0.0
    DESCRIPTION "An action-adventure game released for Windows in 1999."
    LANGUAGES C CXX
)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions(
        -D_CRT_NONSTDC_NO_WARNINGS
        -D_CRT_SECURE_NO_WARNINGS
        -D_ENFORCE_BAN_OF_MACRO_NEW
    )
    add_compile_options(
        /source-charset:windows-1252
        /execution-charset:windows-1252
        /validate-charset
    )
endif(MSVC)

add_library(
    ddengine
    STATIC
    ""
)
add_library(
    ddlibrary
    STATIC
    ""
)
add_library(
    mfstdlib
    STATIC
    ""
)
add_library(
    outro
    STATIC
    ""
)
add_library(
    editor
    STATIC
    ""
)
add_library(
    gedit
    STATIC
    ""
)
add_library(
    ledit
    STATIC
    ""
)
add_library(
    sedit
    STATIC
    ""
)
add_executable(
    fallen
    WIN32
    MACOSX_BUNDLE
    ""
)

include(${CMAKE_CURRENT_LIST_DIR}/MFStdLib/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/DDEngine/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/DDLibrary/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/Editor/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/GEdit/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/Ledit/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/outro/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/sedit/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/fallen/CMakeLists.txt)
