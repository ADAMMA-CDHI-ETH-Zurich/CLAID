# This is the same as CLAIDConfig.cmake.
# Can be used on build environments that do exclude CMAKE_PREFIX_PATH as place to look for packages (e.g., 
# cross compiling environments such as in android studio). In that case, this path needs
# to be added to CMAKE_MODULE_PATH manually.
message("FoundCLAID")

include(${CMAKE_CURRENT_LIST_DIR}\\CLAID_Include.cmake)
