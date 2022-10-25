# This is the same as CLAIDConfig.cmake.
# Can be used on build environments that do exclude CMAKE_PREFIX_PATH as place to look for packages (e.g., 
# cross compiling environments such as in android studio). In that case, this path needs
# to be added to CMAKE_MODULE_PATH manually.
message("FoundCLAID")


if(UNIX)
    # for Linux, BSD, Solaris, Minix, Apple
    set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")
    include(${CMAKE_CURRENT_LIST_DIR}${OS_SLASH}CLAID_Include.cmake)
endif()

if(WIN32)
    # for Windows operating system in general
    set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}\\cmake\\Modules\\")
    include(${CMAKE_CURRENT_LIST_DIR}${OS_SLASH}CLAID_Include.cmake)
endif()



