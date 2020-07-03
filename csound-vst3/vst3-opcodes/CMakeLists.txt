set(vst3_plugins_sources
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/editorhost.cpp
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/editorhost.h
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/appinit.h
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/iapplication.h
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/iplatform.h
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/iwindow.h
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/usediids.cpp
    ${SDK_ROOT}/public.sdk/source/common/memorystream.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/connectionproxy.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/connectionproxy.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/module.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/module.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/optional.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.h 
    ${SDK_ROOT}/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/pluginterfacesupport.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/plugprovider.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/plugprovider.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/stringconvert.cpp
    ${SDK_ROOT}/public.sdk/source/vst/hosting/stringconvert.h
    ${SDK_ROOT}/public.sdk/source/vst/hosting/uid.h
    ${SDK_ROOT}/public.sdk/source/vst/vstinitiids.cpp
    vst3-opcodes.cpp
    )
    
    if(SMTG_MAC)
        set(vst3_plugins_sources
            ${vst3_plugins_sources}
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/mac/platform.mm
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/mac/window.h
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/mac/window.mm
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
        find_package(X11 REQUIRED)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
        include_directories(${X11_INCLUDE_DIR} ${GTKMM3_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
        set(vst3_plugins_sources
            ${vst3_plugins_sources}
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/platform.cpp
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/runloop.h
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/runloop.cpp
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/window.h
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/linux/window.cpp
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_linux.cpp
        )
        set(vst3_plugins_PLATFORM_LIBS
            ${X11_LIBRARIES}
            ${GTK3_LIBRARIES}
            ${GTKMM3_LIBRARIES}
        )
        set(APP_PROPERTIES
        )
    elseif(SMTG_WIN)
        set(vst3_plugins_sources
            ${vst3_plugins_sources}
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/win32/platform.cpp
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/win32/window.h
            ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/win32/window.cpp
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_win32.cpp
        )
        set(vst3_plugins_PLATFORM_LIBS
        )
        set(APP_PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:windows"
        )
    endif()

set(target vst3_plugins)
message("CSOUND_LIBRARIES: ${CSOUND_LIBRARIES}")
add_library(${target} MODULE ${vst3_plugins_sources})
target_link_libraries(${target} PRIVATE base sdk vstgui_standalone ${CSOUND_LIBRARIES} ${vst3_plugins_PLATFORM_LIBS})
target_include_directories(${target} PRIVATE ${CSOUND_INCLUDE_DIRS})
if (APP_PROPERTIES)
    set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
endif()
