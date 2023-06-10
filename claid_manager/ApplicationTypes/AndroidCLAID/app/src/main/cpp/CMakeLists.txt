cmake_minimum_required(VERSION 3.4.1)  # for example
set (CMAKE_CXX_STANDARD 17)
message("Bla $ENV{CMAKE_PREFIX_PATH}")

string(REPLACE ":" ";" CMAKE_PREFIX_PATH_TMP "$ENV{CMAKE_PREFIX_PATH}")
# This step is required, as android studio has configured cmake environment to ignore CMAKE_PREFIX_PATH
list(APPEND CMAKE_MODULE_PATH $ENV{CLAID_CMAKE_MODULE_PATH})

find_package(CLAID REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/CLAIDPackages.cmake)



file(WRITE ${CMAKE_CURRENT_LIST_DIR}/CLAIDJavaDirectories.txt)
foreach(java_directory ${CLAID_JAVA_DIRECTORIES})
    file(APPEND ${CMAKE_CURRENT_LIST_DIR}/CLAIDJavaDirectories.txt "${java_directory}\n")
endforeach()

add_library( JavaCLAID

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        JavaCLAID.cpp
        ${CLAID_SOURCES})

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library( # Sets the name of the path variable.
        android-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        android )
set(ABI ${ANDROID_ABI})

target_link_libraries( # Specifies the target library.
        JavaCLAID
        # Links the target library to the log library
        # included in the NDK.

        ${CLAID_LINK_LIBRARIES})


jbind11_deploy_android_override(JavaCLAID ${CMAKE_CURRENT_LIST_DIR}/JavaCLAID)

target_link_libraries( # Specifies the target library.
        JavaCLAID
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${android-lib})