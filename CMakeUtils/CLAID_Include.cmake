if(NOT DEFINED ENV{CLAID_PATH}) 
  message(FATAL_ERROR "Environment variable CLAID_PATH not set. Did you properly install CLAID using one of the installer scripts?")
endif()

if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
    set(CLAID_LINK_LIBRARIES pthread)
    set(OS_SLASH /)
endif()

if(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
    set(CLAID_LINK_LIBRARIES pthread)
    set(OS_SLASH /)
endif()

if(WIN32)
    # for Windows operating system in general
    set(CLAID_LINK_LIBRARIES wsock32 ws2_32)
    set(OS_SLASH \\)
endif()


macro(CLAID_Include claid_package)
  # if(NOT DEFINED ${claid_package})
  #   message(FATAL_ERROR "Error, CLAID_Include called without providing argument specifying which package to include.")
  # endif()
  set(CLAID_PACKAGE_PATH $ENV{CLAID_PATH}${OS_SLASH}packages${OS_SLASH}${claid_package})
  if(NOT EXISTS "${CLAID_PACKAGE_PATH}")
    message(FATAL_ERROR "CLAID_Include: cannot include package ${claid_package}, it does not exist under $ENV{CLAID_PATH}${OS_SLASH}packages${OS_SLASH}\n Did you install the package?")
  endif()

  include_directories(${CLAID_PACKAGE_PATH}${OS_SLASH}include)

  file(GLOB_RECURSE CURRENT_SOURCES
          "${CLAID_PACKAGE_PATH}${OS_SLASH}src${OS_SLASH}*.cpp"
          ) 

  set(CLAID_SOURCES ${CLAID_SOURCES} ${CURRENT_SOURCES})
  message("Included package ${claid_package}.")

  endmacro()