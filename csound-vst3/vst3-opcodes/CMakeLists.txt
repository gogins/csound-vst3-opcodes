set(vst3_plugins_sources
    ${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost/source/platform/iapplication.h
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
        ${SDK_ROOT}/public.sdk/source/vst/hosting/module_mac.mm)
    set(vst3_plugins_PLATFORM_LIBS "-framework Cocoa")
    get_filename_component(InfoPlistFile "resource/Info.plist" ABSOLUTE)
    set(APP_PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile} )
elseif(SMTG_LINUX)
    set(vst3_plugins_sources
        ${vst3_plugins_sources}
        ${SDK_ROOT}/public.sdk/source/vst/hosting/module_linux.cpp )
    set(vst3_plugins_PLATFORM_LIBS)
    set(APP_PROPERTIES)
elseif(SMTG_WIN)
    set(vst3_plugins_sources
        ${vst3_plugins_sources}
        ${SDK_ROOT}/public.sdk/source/vst/hosting/module_win32.cpp )
    set(vst3_plugins_PLATFORM_LIBS)
    set(APP_PROPERTIES	LINK_FLAGS "/SUBSYSTEM:console"	)
endif()

set(target vst3_plugins)
message("CSOUND_LIBRARIES: ${CSOUND_LIBRARIES}")
add_library(${target} MODULE ${vst3_plugins_sources})
#set_target_properties(${target} PROPERTIES ${SDK_IDE_HOSTING_EXAMPLES_FOLDER})
target_link_libraries(${target} PRIVATE base sdk ${CSOUND_LIBRARIES} ${vst3_plugins_PLATFORM_LIBS})
target_include_directories(${target} PRIVATE ${CSOUND_INCLUDE_DIRS})
if (APP_PROPERTIES)
    set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
endif()
