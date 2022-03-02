message(STATUS "CMAKE_FRAMEWORK_PATH: ${CMAKE_FRAMEWORK_PATH}")
message(STATUS "CMAKE_SYSTEM_FRAMEWORK_PATH: ${CMAKE_SYSTEM_FRAMEWORK_PATH}")
if(APPLE)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    find_package(CSOUND REQUIRED)
endif()
message(STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})
set(vst3_plugins_sources
#    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/editorhost.cpp
#    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/editorhost.h
#    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/appinit.h
#    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/iapplication.h
#    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/iplatform.h
#    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/iwindow.h
#    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/usediids.cpp
    ${SDK_ROOT}/public.sdk/source/common/memorystream.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/connectionproxy.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/connectionproxy.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/module.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/module.h
    ${SDK_ROOT}/public.sdk/source/vst/utility/optional.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.h 
    ${SDK_ROOT}/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/pluginterfacesupport.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/plugprovider.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/plugprovider.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.h
    ${SDK_ROOT}/public.sdk/source/vst/utility/stringconvert.cpp
    ${SDK_ROOT}/public.sdk/source/vst/utility/stringconvert.h
    ${SDK_ROOT}/public.sdk/source/vst/utility/uid.h
    ${SDK_ROOT}/public.sdk/source/vst/vstinitiids.cpp
    vst3-opcodes.cpp
    )
    
    if(SMTG_MAC)
        set(vst3_plugins_sources
            ${vst3_plugins_sources}
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/mac/platform.mm
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/mac/window.h
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/mac/window.mm
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_mac.mm
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
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/platform.cpp
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/runloop.h
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/runloop.cpp
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/window.h
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/window.cpp
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_linux.cpp
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
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/win32/platform.cpp
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/win32/window.h
            #${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/win32/window.cpp
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_win32.cpp
        )
        set(vst3_plugins_PLATFORM_LIBS
        )
        set(APP_PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:console"
        )
    endif()

set(target vst3_plugins)
add_library(${target} MODULE ${vst3_plugins_sources})
target_link_libraries(${target} PRIVATE base sdk ${vst3_plugins_PLATFORM_LIBS})
target_compile_definitions(${target} PRIVATE USE_DOUBLE)
if (SMTG_MAC)
    target_compile_options(${target} PRIVATE "-fobjc-arc")
    target_include_directories(${target} PRIVATE "$ENV{HOMEBREW_PREFIX}/include/csound")
    target_link_libraries(${target} PRIVATE "-F$ENV{HOMEBREW_PREFIX}/Frameworks -framework CsoundLib64")
else()
    target_include_directories(${target} PRIVATE ${CSOUND_INCLUDE_DIR})
endif()
if (APP_PROPERTIES)
    set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
endif()
