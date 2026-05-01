cmake_minimum_required(VERSION 3.21)

project(vst3_opcodes
    VERSION 1.1.1
    DESCRIPTION "Csound opcodes that host VST3 plugins."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(ENABLE_ASAN "Enable AddressSanitizer." OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer." OFF)
option(CSOUND_VST3_OPCODE_SIGN "Sign executable artifacts when signing tools and credentials are available." ON)
option(CSOUND_VST3_OPCODE_NOTARIZE "Notarize the macOS distribution archive when notary credentials are available." ON)

set(CSOUND_VST3_OPCODE_CODESIGN_IDENTITY "$ENV{APPLE_CODESIGN_IDENTITY}" CACHE STRING "macOS codesigning identity.")
set(CSOUND_VST3_OPCODE_APPLE_ID "$ENV{APPLE_ID}" CACHE STRING "Apple ID for notarytool.")
set(CSOUND_VST3_OPCODE_APPLE_TEAM_ID "$ENV{APPLE_TEAM_ID}" CACHE STRING "Apple team ID for notarytool.")
set(CSOUND_VST3_OPCODE_APPLE_APP_PASSWORD "$ENV{APPLE_APP_PASSWORD}" CACHE STRING "Apple app-specific password for notarytool.")
set(CSOUND_VST3_OPCODE_REPOSITORY_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." CACHE PATH "Repository root containing README.md and examples/.")
set(CSOUND_VST3_OPCODE_DIST_DIR "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/dist" CACHE PATH "Distribution staging directory.")
set(CSOUND_VST3_OPCODE_ARCHIVE_NAME "csound-vst3-opcodes-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}.zip" CACHE STRING "Distribution archive filename.")

set(CSOUND_VST3_OPCODE_CODESIGN_IDENTITY
    "$ENV{APPLE_CODESIGN_IDENTITY}"
    CACHE STRING "macOS codesigning identity."
)

# App Store Connect API key (preferred for notarytool)
set(CSOUND_VST3_OPCODE_NOTARY_KEY
    "$ENV{APPLE_API_KEY_PATH}"
    CACHE FILEPATH "Path to AuthKey_XXXX.p8 for notarytool."
)

set(CSOUND_VST3_OPCODE_NOTARY_KEY_ID
    "$ENV{APPLE_API_KEY_ID}"
    CACHE STRING "App Store Connect API Key ID."
)

set(CSOUND_VST3_OPCODE_NOTARY_ISSUER
    "$ENV{APPLE_API_ISSUER_ID}"
    CACHE STRING "App Store Connect API Issuer ID."
)

set(CSOUND_VST3_OPCODE_REPOSITORY_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/.."
    CACHE PATH "Repository root containing README.md and examples/."
)

set(CSOUND_VST3_OPCODE_DIST_DIR
    "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/dist"
    CACHE PATH "Distribution staging directory."
)

set(CSOUND_VST3_OPCODE_ARCHIVE_NAME
    "csound-vst3-opcodes-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}.zip"
    CACHE STRING "Distribution archive filename."
)

set(CSOUND_VST3_OPCODE_ARCHIVE
    "${CMAKE_BINARY_DIR}/${CSOUND_VST3_OPCODE_ARCHIVE_NAME}"
    CACHE FILEPATH "Full path to the distribution archive."
)

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASan and TSan cannot both be enabled.")
endif()

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
elseif(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "CSOUND_VST3_OPCODE_REPOSITORY_ROOT: ${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}")
message(STATUS "CSOUND_VST3_OPCODE_DIST_DIR: ${CSOUND_VST3_OPCODE_DIST_DIR}")
message(STATUS "CSOUND_VST3_OPCODE_ARCHIVE_NAME: ${CSOUND_VST3_OPCODE_ARCHIVE_NAME}")
message(STATUS "CSOUND_VST3_OPCODE_ARCHIVE: ${CSOUND_VST3_OPCODE_ARCHIVE}")

include("${CMAKE_CURRENT_LIST_DIR}/cmake/FindCsoundHomeFirst.cmake")

if(NOT CSOUND_FOUND)
    message(FATAL_ERROR "Csound was not found.")
endif()

message(STATUS "Using Csound executable:    ${CSOUND_EXECUTABLE}")
message(STATUS "Using Csound library:       ${CSOUND_LIBRARY}")
message(STATUS "Using Csound include dirs:  ${CSOUND_INCLUDE_DIRS}")
message(STATUS "Using Csound framework dir: ${CSOUND_FRAMEWORK_DIR}")
message(STATUS "Using Csound version:       ${CSOUND_VERSION}")

set(vst3sdk_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../vst3sdk" CACHE PATH "Steinberg VST3 SDK source directory.")
add_subdirectory("${vst3sdk_SOURCE_DIR}" "${PROJECT_BINARY_DIR}/vst3sdk")
smtg_enable_vst3_sdk()

if(SMTG_WIN)
    set(SMTG_USE_STDATOMIC_H OFF CACHE BOOL "" FORCE)
endif()

set(vst3_plugins_sources
    ${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/vst3-opcodes/vst3-opcodes.cpp
    ${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/vst3-opcodes/OpcodeBaseAC.hpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/memorystream.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/connectionproxy.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/eventlist.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/hostclasses.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/parameterchanges.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/plugprovider.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/processdata.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/utility/stringconvert.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/vstinitiids.cpp
)

set(vst3_plugins_platform_libs)

if(SMTG_MAC)
    list(APPEND vst3_plugins_sources
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_mac.mm
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_mac.mm
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_mac.mm
    )
    list(APPEND vst3_plugins_platform_libs "-framework Cocoa" "-framework Foundation")
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
endif()

add_library(vst3_plugins MODULE ${vst3_plugins_sources})
smtg_target_setup_universal_binary(vst3_plugins)

target_link_libraries(vst3_plugins
    PRIVATE
        base
        pluginterfaces
        sdk
        sdk_common
        sdk_hosting
        ${vst3_plugins_platform_libs}
        ${CSOUND_LIBRARIES}
)

target_compile_definitions(vst3_plugins
    PRIVATE
        RELEASE=1
        USE_DOUBLE
        CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
        CSOUND_VERSION_MINOR=${CSOUND_VERSION_MINOR}
        CSOUND_AC_CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
        CSOUND_AC_CSOUND_VERSION_MINOR=${CSOUND_VERSION_MINOR}
)

target_include_directories(vst3_plugins
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CSOUND_INCLUDE_DIRS}"
)

if(SMTG_MAC)
    set_target_properties(vst3_plugins PROPERTIES
        PREFIX "lib"
        SUFFIX ".dylib"
    )
    target_compile_options(vst3_plugins PRIVATE -fobjc-arc)
    if(CSOUND_FRAMEWORK_DIR)
        get_filename_component(csound_framework_parent "${CSOUND_FRAMEWORK_DIR}" DIRECTORY)
        target_link_options(vst3_plugins PRIVATE
            "-F${csound_framework_parent}"
            "-Wl,-rpath,${csound_framework_parent}"
        )
    endif()
elseif(SMTG_LINUX)
    set_target_properties(vst3_plugins PROPERTIES PREFIX "lib" SUFFIX ".so")
    if(CSOUND_LIBRARY)
        get_filename_component(csound_lib_dir "${CSOUND_LIBRARY}" DIRECTORY)
        target_link_options(vst3_plugins PRIVATE "-Wl,-rpath,${csound_lib_dir}")
    endif()
elseif(SMTG_WIN)
    target_compile_options(vst3_plugins PRIVATE /Zc:__cplusplus)
    set_target_properties(vst3_plugins PROPERTIES PREFIX "" SUFFIX ".dll")
endif()

add_custom_target(clean_dist
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CSOUND_VST3_OPCODE_DIST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CSOUND_VST3_OPCODE_DIST_DIR}/bin"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CSOUND_VST3_OPCODE_DIST_DIR}/examples"
    COMMENT "Creating clean dist directory"
)

add_custom_target(stage_dist ALL
    DEPENDS clean_dist vst3_plugins
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:vst3_plugins>" "${CSOUND_VST3_OPCODE_DIST_DIR}/bin/$<TARGET_FILE_NAME:vst3_plugins>"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/README.md" "${CSOUND_VST3_OPCODE_DIST_DIR}/README.md"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/LICENSE" "${CSOUND_VST3_OPCODE_DIST_DIR}/LICENSE"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/examples" "${CSOUND_VST3_OPCODE_DIST_DIR}/examples"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/README.md" "${CSOUND_VST3_OPCODE_DIST_DIR}/vst3-opcodes-README.md"
    COMMENT "Staging README.md, LICENSE, examples, and build artifacts in dist"
)

if(APPLE)
    add_custom_target(sign_dist
        DEPENDS stage_dist
        COMMAND /bin/bash
            "${CMAKE_CURRENT_LIST_DIR}/cmake/sign-macos-dist.bash"
            "${CSOUND_VST3_OPCODE_DIST_DIR}/bin"
            "${CSOUND_VST3_OPCODE_SIGN}"
            "${CSOUND_VST3_OPCODE_CODESIGN_IDENTITY}"
        VERBATIM
        COMMENT "Signing macOS executable artifacts in dist/bin"
    )

    add_custom_target(archive_dist
        DEPENDS sign_dist
        COMMAND "${CMAKE_COMMAND}" -E tar cfv "${PROJECT_BINARY_DIR}/${CSOUND_VST3_OPCODE_ARCHIVE_NAME}" --format=zip -- dist
        WORKING_DIRECTORY "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}"
        COMMENT "Creating macOS dist archive"
    )

    add_custom_target(notarize_dist
        DEPENDS archive_dist
        COMMAND /bin/bash
            "${CMAKE_CURRENT_LIST_DIR}/cmake/notarize-macos-dist.bash"
            "${CSOUND_VST3_OPCODE_ARCHIVE}"
            "${CSOUND_VST3_OPCODE_NOTARIZE}"
            "${CSOUND_VST3_OPCODE_NOTARY_KEY}"
            "${CSOUND_VST3_OPCODE_NOTARY_KEY_ID}"
            "${CSOUND_VST3_OPCODE_NOTARY_ISSUER}"
        VERBATIM
        COMMENT "Notarizing macOS dist archive"
    )
else()
    add_custom_target(sign_dist
        DEPENDS stage_dist
        COMMAND "${CMAKE_COMMAND}" -E echo "No platform signing step configured for ${CMAKE_SYSTEM_NAME}."
    )

    add_custom_target(archive_dist
        DEPENDS sign_dist
        COMMAND "${CMAKE_COMMAND}" -E tar cfv "${PROJECT_BINARY_DIR}/${CSOUND_VST3_OPCODE_ARCHIVE_NAME}" --format=zip -- dist
        WORKING_DIRECTORY "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}"
        COMMENT "Creating dist archive"
    )
endif()
