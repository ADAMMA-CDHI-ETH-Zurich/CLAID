message("FoundCLAID")

if(UNIX)
    # for Linux, BSD, Solaris, Minix, Apple
    include(${CMAKE_CURRENT_LIST_DIR}/CLAID_Include.cmake)
endif()

if(WIN32)
    # for Windows operating system in general
    include(${CMAKE_CURRENT_LIST_DIR}\\CLAID_Include.cmake)
endif()