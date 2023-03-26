if(NOT DEFINED ENV{CLAID_PATH}) 
  message(FATAL_ERROR "Environment variable CLAID_PATH not set. Did you properly install CLAID using one of the installer scripts?")
endif()

macro(CLAID_Include claid_package)

  # if(NOT DEFINED ${claid_package})
  #   message(FATAL_ERROR "Error, CLAID_Include called without providing argument specifying which package to include.")
  # endif()
  file(TO_CMAKE_PATH $ENV{CLAID_PATH} CLAID_PATH)
    message("PATH ${CLAID_PATH}" )
  set(CLAID_PACKAGE_PATH ${CLAID_PATH}/packages/${claid_package})
  if(NOT EXISTS "${CLAID_PACKAGE_PATH}")
    message(FATAL_ERROR "CLAID_Include: cannot include package ${claid_package}, it does not exist under $ENV{CLAID_PATH}/packages/\n Did you install the package?")
  endif()

  include(${CLAID_PACKAGE_PATH}/Require.cmake)
  message("Included package ${claid_package}.")

  endmacro()