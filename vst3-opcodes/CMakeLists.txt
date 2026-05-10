cmake_minimum_required(VERSION 3.21)

project(vst3_opcodes
    VERSION 2.0.0
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
option(CSOUND_AC_ENABLE_CODESIGN "Sign Mach-O artifacts in dist when credentials are available." OFF)
option(CSOUND_AC_ENABLE_NOTARIZATION "Notarize the macOS distribution archive when credentials are available." OFF)

set(CSOUND_VST3_OPCODE_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}"
    CACHE PATH "Directory containing the vst3-opcodes sources."
)

set(CSOUND_VST3_OPCODE_REPOSITORY_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/.."
    CACHE PATH "Repository root containing README.md and examples/."
)

set(CSOUND_VST3_OPCODE_DIST_DIR
    "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/dist"
    CACHE PATH "Distribution staging and install directory."
)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" CSOUND_VST3_OPCODE_SYSTEM_NAME_LOWER)

set(CSOUND_VST3_OPCODE_ARCHIVE_NAME
    "csound-vst3-opcodes-${PROJECT_VERSION}-${CSOUND_VST3_OPCODE_SYSTEM_NAME_LOWER}.zip"
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
message(STATUS "CSOUND_VST3_OPCODE_SOURCE_DIR: ${CSOUND_VST3_OPCODE_SOURCE_DIR}")
message(STATUS "CSOUND_VST3_OPCODE_REPOSITORY_ROOT: ${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}")
message(STATUS "CSOUND_VST3_OPCODE_DIST_DIR: ${CSOUND_VST3_OPCODE_DIST_DIR}")
message(STATUS "CSOUND_VST3_OPCODE_ARCHIVE_NAME: ${CSOUND_VST3_OPCODE_ARCHIVE_NAME}")
message(STATUS "CSOUND_VST3_OPCODE_ARCHIVE: ${CSOUND_VST3_OPCODE_ARCHIVE}")
message(STATUS "CSOUND_AC_ENABLE_CODESIGN: ${CSOUND_AC_ENABLE_CODESIGN}")
message(STATUS "CSOUND_AC_ENABLE_NOTARIZATION: ${CSOUND_AC_ENABLE_NOTARIZATION}")

message(STATUS "Refreshing shared files from csound-ac repository head...")

set(csound_ac_refresh_dir "${CMAKE_BINARY_DIR}/csound-ac-refresh")
file(MAKE_DIRECTORY "${csound_ac_refresh_dir}")
file(MAKE_DIRECTORY "${CSOUND_VST3_OPCODE_SOURCE_DIR}/cmake")

file(DOWNLOAD
    "https://raw.githubusercontent.com/gogins-dev/csound-ac/refs/heads/master/cmake/FindCsoundHomeFirst.cmake"
    "${csound_ac_refresh_dir}/FindCsoundHomeFirst.cmake"
    STATUS find_csound_download_status
    SHOW_PROGRESS
)

list(GET find_csound_download_status 0 find_csound_download_code)

if(find_csound_download_code EQUAL 0)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${csound_ac_refresh_dir}/FindCsoundHomeFirst.cmake"
            "${CSOUND_VST3_OPCODE_SOURCE_DIR}/cmake/FindCsoundHomeFirst.cmake"
    )
else()
    list(GET find_csound_download_status 1 find_csound_download_message)
    message(WARNING
        "Failed to refresh FindCsoundHomeFirst.cmake from csound-ac head: "
        "${find_csound_download_message}. Using local copy."
    )
endif()

file(DOWNLOAD
    "https://raw.githubusercontent.com/gogins-dev/csound-ac/refs/heads/master/CsoundAC/OpcodeBaseAC.hpp"
    "${csound_ac_refresh_dir}/OpcodeBaseAC.hpp"
    STATUS opcode_base_download_status
    SHOW_PROGRESS
)

list(GET opcode_base_download_status 0 opcode_base_download_code)

if(opcode_base_download_code EQUAL 0)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${csound_ac_refresh_dir}/OpcodeBaseAC.hpp"
            "${CSOUND_VST3_OPCODE_SOURCE_DIR}/OpcodeBaseAC.hpp"
    )
else()
    list(GET opcode_base_download_status 1 opcode_base_download_message)
    message(WARNING
        "Failed to refresh OpcodeBaseAC.hpp from csound-ac head: "
        "${opcode_base_download_message}. Using local copy."
    )
endif()

file(DOWNLOAD
    "https://raw.githubusercontent.com/gogins-dev/csound-ac/refs/heads/master/external/codesign-check.bash"
    "${csound_ac_refresh_dir}/codesign-check.bash"
    STATUS codesign_check_download_status
    SHOW_PROGRESS
)

list(GET codesign_check_download_status 0 codesign_check_download_code)

if(codesign_check_download_code EQUAL 0)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${csound_ac_refresh_dir}/codesign-check.bash"
            "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/codesign-check.bash"
    )
    execute_process(
        COMMAND /bin/chmod +x
            "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/codesign-check.bash"
    )
else()
    list(GET codesign_check_download_status 1 codesign_check_download_message)
    message(WARNING
        "Failed to refresh codesign-check.bash from csound-ac head: "
        "${codesign_check_download_message}. Using local copy if present."
    )
endif()

include("${CSOUND_VST3_OPCODE_SOURCE_DIR}/cmake/FindCsoundHomeFirst.cmake")

if(NOT CSOUND_FOUND)
    message(FATAL_ERROR "Csound was not found.")
endif()

message(STATUS "Using Csound executable:    ${CSOUND_EXECUTABLE}")
message(STATUS "Using Csound library:       ${CSOUND_LIBRARY}")
message(STATUS "Using Csound include dirs:  ${CSOUND_INCLUDE_DIRS}")
message(STATUS "Using Csound framework dir: ${CSOUND_FRAMEWORK_DIR}")
message(STATUS "Using Csound version:       ${CSOUND_VERSION}")

set(_csound_resolved_include_dirs "")
foreach(_csound_include_dir IN LISTS CSOUND_INCLUDE_DIRS)
    if(EXISTS "${_csound_include_dir}/csound.h")
        list(APPEND _csound_resolved_include_dirs "${_csound_include_dir}")
    elseif(EXISTS "${_csound_include_dir}/csound/csound.h")
        list(APPEND _csound_resolved_include_dirs "${_csound_include_dir}/csound")
    else()
        list(APPEND _csound_resolved_include_dirs "${_csound_include_dir}")
    endif()
endforeach()

if(_csound_resolved_include_dirs)
    list(REMOVE_DUPLICATES _csound_resolved_include_dirs)
    set(CSOUND_INCLUDE_DIRS "${_csound_resolved_include_dirs}")
    list(GET CSOUND_INCLUDE_DIRS 0 CSOUND_INCLUDE_DIR)
endif()

message(STATUS "Resolved Csound include dirs: ${CSOUND_INCLUDE_DIRS}")

set(vst3sdk_SOURCE_DIR
    "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/vst3sdk"
    CACHE PATH "Steinberg VST3 SDK source directory."
)

add_subdirectory("${vst3sdk_SOURCE_DIR}" "${PROJECT_BINARY_DIR}/vst3sdk")
smtg_enable_vst3_sdk()

if(SMTG_WIN)
    set(SMTG_USE_STDATOMIC_H OFF CACHE BOOL "" FORCE)
endif()

set(vst3_plugins_sources
    "${CSOUND_VST3_OPCODE_SOURCE_DIR}/vst3-opcodes.cpp"
    "${CSOUND_VST3_OPCODE_SOURCE_DIR}/OpcodeBaseAC.hpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/memorystream.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/connectionproxy.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/eventlist.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/hostclasses.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/parameterchanges.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/pluginterfacesupport.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/plugprovider.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/processdata.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/utility/stringconvert.cpp"
    "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/vstinitiids.cpp"
)

set(vst3_plugins_platform_libs)

if(SMTG_MAC)
    list(APPEND vst3_plugins_sources
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_mac.mm"
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_mac.mm"
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_mac.mm"
    )
    list(APPEND vst3_plugins_platform_libs "-framework Cocoa" "-framework Foundation")
elseif(SMTG_LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
    list(APPEND vst3_plugins_sources
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_linux.cpp"
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_linux.cpp"
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_linux.cpp"
    )
elseif(SMTG_WIN)
    list(APPEND vst3_plugins_sources
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/hosting/module_win32.cpp"
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/systemclipboard_win32.cpp"
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/threadchecker_win32.cpp"
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
        "${CSOUND_VST3_OPCODE_SOURCE_DIR}"
        "${CSOUND_INCLUDE_DIRS}"
)

if(SMTG_MAC)
    set_target_properties(vst3_plugins PROPERTIES
        PREFIX "lib"
        SUFFIX ".dylib"
        BUILD_WITH_INSTALL_RPATH ON
        INSTALL_RPATH "@loader_path;@loader_path/../lib;/Library/Frameworks;/usr/local/lib"
    )

    target_compile_options(vst3_plugins PRIVATE -fobjc-arc)

    target_link_options(vst3_plugins PRIVATE
        "-Wl,-rpath,@loader_path"
        "-Wl,-rpath,@loader_path/../lib"
        "-Wl,-rpath,/Library/Frameworks"
        "-Wl,-rpath,/usr/local/lib"
    )

    if(CSOUND_FRAMEWORK_DIR)
        get_filename_component(csound_framework_parent "${CSOUND_FRAMEWORK_DIR}" DIRECTORY)
        target_link_options(vst3_plugins PRIVATE
            "-F${csound_framework_parent}"
            "-Wl,-rpath,${csound_framework_parent}"
        )
    endif()
elseif(SMTG_LINUX)
    set_target_properties(vst3_plugins PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"
    )
    if(CSOUND_LIBRARY)
        get_filename_component(csound_lib_dir "${CSOUND_LIBRARY}" DIRECTORY)
        target_link_options(vst3_plugins PRIVATE "-Wl,-rpath,${csound_lib_dir}")
    endif()
elseif(SMTG_WIN)
    target_compile_options(vst3_plugins PRIVATE /Zc:__cplusplus)
    set_target_properties(vst3_plugins PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )
endif()

install(TARGETS vst3_plugins
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)

install(FILES
    "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/README.md"
    "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/LICENSE"
    DESTINATION .
)

install(DIRECTORY
    "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/examples"
    DESTINATION .
)

add_custom_target(clean_dist
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CSOUND_VST3_OPCODE_DIST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CSOUND_VST3_OPCODE_DIST_DIR}/bin"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CSOUND_VST3_OPCODE_DIST_DIR}/examples"
    COMMENT "Creating clean dist directory"
)

add_custom_target(stage_dist
    DEPENDS clean_dist vst3_plugins
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:vst3_plugins>"
        "${CSOUND_VST3_OPCODE_DIST_DIR}/bin/$<TARGET_FILE_NAME:vst3_plugins>"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/README.md"
        "${CSOUND_VST3_OPCODE_DIST_DIR}/README.md"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/LICENSE"
        "${CSOUND_VST3_OPCODE_DIST_DIR}/LICENSE"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/examples"
        "${CSOUND_VST3_OPCODE_DIST_DIR}/examples"
    COMMENT "Staging README.md, LICENSE, examples, and build artifacts in dist"
)

if(APPLE)
    add_custom_target(sign_dist
        DEPENDS stage_dist
        COMMAND /bin/bash
            "${CSOUND_VST3_OPCODE_SOURCE_DIR}/cmake/sign-macos-dist.bash"
            "${CSOUND_VST3_OPCODE_DIST_DIR}"
            "$<IF:$<BOOL:${CSOUND_AC_ENABLE_CODESIGN}>,ON,OFF>"
        VERBATIM
        COMMENT "Signing macOS Mach-O artifacts in dist when enabled"
    )

    add_custom_target(archive_dist
        DEPENDS sign_dist
        COMMAND "${CMAKE_COMMAND}" -E tar cfv "${CSOUND_VST3_OPCODE_ARCHIVE}" --format=zip -- dist
        WORKING_DIRECTORY "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}"
        COMMENT "Creating macOS dist archive"
    )

    if(CSOUND_AC_ENABLE_CODESIGN AND CSOUND_AC_ENABLE_NOTARIZATION)
        add_custom_target(notarize_dist
            DEPENDS archive_dist
            COMMAND /bin/bash
                "${CSOUND_VST3_OPCODE_SOURCE_DIR}/cmake/notarize-macos-dist.bash"
                "${CSOUND_VST3_OPCODE_ARCHIVE}"
                ON
            COMMAND /bin/bash
                "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}/codesign-check.bash"
                "${CSOUND_VST3_OPCODE_ARCHIVE}"
            VERBATIM
            COMMENT "Notarizing and validating macOS dist archive when enabled"
        )
    else()
        add_custom_target(notarize_dist
            DEPENDS archive_dist
            COMMAND /bin/bash
                "${CSOUND_VST3_OPCODE_SOURCE_DIR}/cmake/notarize-macos-dist.bash"
                "${CSOUND_VST3_OPCODE_ARCHIVE}"
                OFF
            VERBATIM
            COMMENT "Notarization and codesign-check.bash validation are disabled"
        )
    endif()

    add_custom_target(release_dist
        DEPENDS notarize_dist
        COMMENT "Building complete release archive"
    )
else()
    add_custom_target(sign_dist
        DEPENDS stage_dist
        COMMAND "${CMAKE_COMMAND}" -E echo "No platform signing step configured for ${CMAKE_SYSTEM_NAME}."
    )

    add_custom_target(archive_dist
        DEPENDS sign_dist
        COMMAND "${CMAKE_COMMAND}" -E tar cfv "${CSOUND_VST3_OPCODE_ARCHIVE}" --format=zip -- dist
        WORKING_DIRECTORY "${CSOUND_VST3_OPCODE_REPOSITORY_ROOT}"
        COMMENT "Creating dist archive"
    )

    add_custom_target(release_dist
        DEPENDS archive_dist
        COMMENT "Building complete release archive"
    )
endif()
