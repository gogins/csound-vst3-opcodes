cmake_minimum_required(VERSION 3.10.0)
# Useful information.
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")    
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")    

set(CMAKE_OSX_ARCHITECTURES=arm64;x86_64)
project(vst3_opcodes
    VERSION 1.0.0
    DESCRIPTION "Csound opcodes that host VST3 plugins."
)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
add_compile_definitions(RELEASE=1)
set(SMTG_BUILD_UNIVERSAL_BINARY On)

set(vst3sdk_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vst3sdk)
add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

# Global CMake configuration.
set(CMAKE_CXX_STANDARD 17)
if (SMTG_WIN)
    set(SMTG_USE_STDATOMIC_H OFF CACHE BOOL “” FORCE)
endif()

# Target-specific configuration.
set(vst3_plugins_sources
    vst3-opcodes.cpp
    #    ${vst3sdk_SOURCE_DIR}/public.sdk/samples/vst-hosting/editorhost/source/editorhost.cpp
    #    ${vst3sdk_SOURCE_DIR}/public.sdk/samples/vst-hosting/editorhost/source/editorhost.h
    #    ${vst3sdk_SOURCE_DIR}/public.sdk/samples/vst-hosting/editorhost/source/platform/appinit.h
    #    ${vst3sdk_SOURCE_DIR}/public.sdk/samples/vst-hosting/editorhost/source/platform/iapplication.h
    #    ${vst3sdk_SOURCE_DIR}/public.sdk/samples/vst-hosting/editorhost/source/platform/iplatform.h
    #    ${vst3sdk_SOURCE_DIR}/public.sdk/samples/vst-hosting/editorhost/source/platform/iwindow.h
    #    ${vst3sdk_SOURCE_DIR}/public.sdk/samples/vst-hosting/editorhost/source/usediids.cpp
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
if(SMTG_MAC)
    set(vst3_plugins_sources
        ${vst3_plugins_sources}
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_mac.mm
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_mac.mm
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_mac.mm
    )
    set(vst3_plugins_PLATFORM_LIBS
        "-framework Cocoa"
    )
    get_filename_component(InfoPlistFile "resource/Info.plist" ABSOLUTE)
    set(APP_PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
    )
elseif(SMTG_LINUX)
    #find_package(X11 REQUIRED)
    find_package(PkgConfig REQUIRED)
    #pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
    #include_directories(${X11_INCLUDE_DIR} ${GTKMM3_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
    set(vst3_plugins_sources
        ${vst3_plugins_sources}
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_linux.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_linux.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_linux.cpp
    )
    set(vst3_plugins_PLATFORM_LIBS
        #${X11_LIBRARIES}
        #${GTK3_LIBRARIES}
        #${GTKMM3_LIBRARIES}
    )
    set(APP_PROPERTIES
    )
elseif(SMTG_WIN)
    set(vst3_plugins_sources
        ${vst3_plugins_sources}
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_win32.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_win32.cpp
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_win32.cpp    
    )
    set(vst3_plugins_PLATFORM_LIBS
    )
    set(APP_PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:console"
    )
endif()

set(target vst3_plugins)
add_library(${target} MODULE ${vst3_plugins_sources})
smtg_target_setup_universal_binary(${target})

target_link_libraries(${target} PRIVATE base pluginterfaces sdk sdk_common sdk_hosting ${vst3_plugins_PLATFORM_LIBS})
target_compile_definitions(${target} PRIVATE USE_DOUBLE)
if (SMTG_MAC)
    target_compile_options(${target} PRIVATE "-fobjc-arc")
    target_include_directories(${target} PRIVATE 
        "/Library/Frameworks/CsoundLib64.framework/Headers"
        "/Users/runner/work/csound-vst3-opcodes/csound-vst3-opcodes/csound/include"
        ".")
endif()
target_include_directories(${target} PRIVATE ${CSOUND_INCLUDE_DIR})
if (SMTG_LINUX)
    target_include_directories(${target} PRIVATE 
    "/home/runner/work/csound-vst3-opcodes/csound-vst3-opcodes/csound/include"
    ".")
endif()
if (SMTG_WIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++17")
    target_include_directories(${target} PRIVATE 
        "${CMAKE_SOURCE_DIR}/../csound/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../csound/include"
        ".")
endif()
if (APP_PROPERTIES)
    set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
endif()