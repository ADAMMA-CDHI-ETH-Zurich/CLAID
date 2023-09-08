# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(claid_core_library VERSION 0.0.1 LANGUAGES C)

add_library(claid_core SHARED
  "claid_core.c"
)

set_target_properties(claid_core PROPERTIES
  PUBLIC_HEADER claid_core.h
  OUTPUT_NAME "claid_core"
)

target_compile_definitions(claid_core PUBLIC DART_SHARED_LIB)
