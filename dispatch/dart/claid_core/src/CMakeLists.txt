add_library(claid_core
        # Sets the library as a shared library.
        SHARED
        claid_core.cpp

        )

target_link_libraries(claid_core ${CMAKE_CURRENT_LIST_DIR}/../blobs/libclaid_capi_linux.so)
