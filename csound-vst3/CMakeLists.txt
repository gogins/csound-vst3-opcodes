# This assumes that the user has built and installed Csound on Linux.
list(APPEND CMAKE_MODULE_PATH /usr/local/share/cmake/Csound)
find_package(Csound REQUIRED)
if(ANDROID)
    return()
endif()
include(SMTG_AddSubDirectories)
smtg_add_subdirectories()
