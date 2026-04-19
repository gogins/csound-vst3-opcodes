cmake_minimum_required(VERSION 3.12)

project(vst3_opcodes
    VERSION 1.1.1
    DESCRIPTION "Csound opcodes that host VST3 plugins."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

file(DOWNLOAD
    "https://raw.githubusercontent.com/gogins/csound-ac/master/CsoundAC/OpcodeBaseAC.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/OpcodeBaseAC.hpp"
)

# Big trouble lurking.
set(CMAKE_OSX_ARCHITECTURES arm64)
# set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)

option(ENABLE_ASAN "Enable AddressSanitizer (ASan)" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer (TSan)" OFF)

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASan and TSan cannot be enabled at the same time.")
endif()

if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled.")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
elseif(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled.")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(SMTG_BUILD_UNIVERSAL_BINARY On)

#
# Find Csound, preferring $HOME installs and then root/system installs.
#
include("${CMAKE_CURRENT_LIST_DIR}/cmake/FindCsoundHomeFirst.cmake")

if(NOT CSOUND_FOUND)
    message(FATAL_ERROR "Csound was not found.")
endif()

message(STATUS "Using Csound executable:    ${CSOUND_EXECUTABLE}")
message(STATUS "Using Csound library:       ${CSOUND_LIBRARY}")
message(STATUS "Using Csound include dirs:  ${CSOUND_INCLUDE_DIRS}")
message(STATUS "Using Csound framework dir: ${CSOUND_FRAMEWORK_DIR}")
message(STATUS "Using Csound version:       ${CSOUND_VERSION}")

set(vst3sdk_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vst3sdk)
add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

if(SMTG_WIN)
    set(SMTG_USE_STDATOMIC_H OFF CACHE BOOL "" FORCE)
endif()

set(vst3_plugins_sources
    vst3-opcodes.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/memorystream.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/connectionproxy.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/connectionproxy.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/eventlist.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/eventlist.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/hostclasses.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/hostclasses.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/utility/optional.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/parameterchanges.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/parameterchanges.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/pluginterfacesupport.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/plugprovider.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/plugprovider.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/processdata.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/processdata.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/utility/stringconvert.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/utility/stringconvert.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/utility/uid.h
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/vstinitiids.cpp
)

set(vst3_plugins_PLATFORM_LIBS)
set(APP_PROPERTIES)

if(SMTG_MAC)
    list(APPEND vst3_plugins_sources
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_mac.mm
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_mac.mm
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_mac.mm
    )

    list(APPEND vst3_plugins_PLATFORM_LIBS
        "-framework Cocoa"
    )

    get_filename_component(InfoPlistFile "resource/Info.plist" ABSOLUTE)

    set(APP_PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
    )

elseif(SMTG_LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)

    list(APPEND vst3_plugins_sources
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_linux.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_linux.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_linux.cpp
    )

elseif(SMTG_WIN)
    list(APPEND vst3_plugins_sources
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_win32.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_win32.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_win32.cpp
    )

    set(APP_PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:console"
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    )
endif()

set(target vst3_plugins)
add_library(${target} MODULE ${vst3_plugins_sources})
smtg_target_setup_universal_binary(${target})

target_link_libraries(${target}
    PRIVATE
        base
        pluginterfaces
        sdk
        sdk_common
        sdk_hosting
        ${vst3_plugins_PLATFORM_LIBS}
        ${CSOUND_LIBRARIES}
)

target_compile_definitions(${target}
    PRIVATE
        RELEASE=1
        USE_DOUBLE
        CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
        CSOUND_VERSION_MINOR=${CSOUND_VERSION_MINOR}
        CSOUND_AC_CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
        CSOUND_AC_CSOUND_VERSION_MINOR=${CSOUND_VERSION_MINOR}
)

target_include_directories(${target}
    PRIVATE
        "${CSOUND_INCLUDE_DIRS}"
        "."
)

if(SMTG_MAC)
    target_compile_options(${target}
        PRIVATE
            "-fobjc-arc"
    )

    if(CSOUND_FRAMEWORK_DIR)
        get_filename_component(_csound_framework_parent "${CSOUND_FRAMEWORK_DIR}" DIRECTORY)
        target_link_options(${target}
            PRIVATE
                "-F${_csound_framework_parent}"
                "-Wl,-rpath,${_csound_framework_parent}"
        )
    endif()
endif()

if(SMTG_LINUX)
    target_include_directories(${target}
        PRIVATE
            "/home/runner/work/csound-vst3-opcodes/csound-vst3-opcodes/csound/include"
    )

    if(CSOUND_LIBRARY)
        get_filename_component(_csound_lib_dir "${CSOUND_LIBRARY}" DIRECTORY)
        target_link_options(${target}
            PRIVATE
                "-Wl,-rpath,${_csound_lib_dir}"
        )
    endif()
endif()

if(SMTG_WIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++17")

    target_include_directories(${target}
        PRIVATE
            "${CMAKE_SOURCE_DIR}/../csound/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../csound/include"
    )
endif()

if(APP_PROPERTIES)
    set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
endif()