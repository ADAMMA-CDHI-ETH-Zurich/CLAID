message("FoundCLAID")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}\\cmake\\Modules\\")
include(${CMAKE_CURRENT_LIST_DIR}\\CLAID_Include.cmake)

if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
    set(CLAID_LINK_LIBRARIES pthread)
endif()

if(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
    set(CLAID_LINK_LIBRARIES pthread)
endif()

if(WIN32)
    # for Windows operating system in general
    set(CLAID_LINK_LIBRARIES wsock32 ws2_32)
endif()