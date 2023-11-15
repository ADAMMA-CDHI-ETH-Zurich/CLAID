cmake_minimum_required(VERSION 3.18)

add_library(claid_core
        # Sets the library as a shared library.
        SHARED
        claid_core.cpp

        )


if(ANDROID)

add_library(libandroid_claid SHARED IMPORTED)
set_target_properties(libandroid_claid PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/../blobs/android/${ANDROID_ABI}/libclaid_capi_android.so
        )

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


        target_link_libraries(claid_core libandroid_claid ${log-lib}
        ${android-lib})

elseif(UNIX)
        target_link_libraries(claid_core ${CMAKE_CURRENT_LIST_DIR}/../blobs/libclaid_capi_linux.so)

endif()

