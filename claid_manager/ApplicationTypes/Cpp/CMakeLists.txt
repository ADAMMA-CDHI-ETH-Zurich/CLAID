set (CMAKE_CXX_STANDARD 14)

# This allows you to use CLAID_Include,
# that can be used to include CLAID packages

find_package(CLAID REQUIRED)
# Include desired packages.
include(${CMAKE_CURRENT_LIST_DIR}/CLAIDPackageList.txt)

# Scanning for all .cpp files in the src directory
file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    )

# Add "include" as include directory
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

## The executable will be compiled from the project source files and
## the source files from the included CLAID packages.
add_executable(MyApplication
    ${PROJECT_SOURCES}
    ${CLAID_SOURCES})

target_link_libraries(MyApplication ${CLAID_LINK_LIBRARIES})


if(MSVC)
  add_definitions(/MP)
endif()
