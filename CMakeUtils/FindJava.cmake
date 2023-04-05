
macro(CLAID_FIND_JAVA)
        find_package(Java 18 REQUIRED Development)
        include(UseJava)

        ## NOTE remove these JAVA*INCLUDE lines when this is fixed https://gitlab.kitware.com/cmake/cmake/-/issues/23364
        set(JAVA_INCLUDE_PATH "$ENV{JAVA_HOME}/include")
        find_path(JAVA_INCLUDE_PATH2 NAMES jni_md.h jniport.h
                PATHS ${JAVA_INCLUDE_PATH}
                ${JAVA_INCLUDE_PATH}/darwin
                ${JAVA_INCLUDE_PATH}/win32
                ${JAVA_INCLUDE_PATH}/linux
                )

        set(JAVA_AWT_INCLUDE_PATH "$ENV{JAVA_HOME}/include")
        find_package(JNI REQUIRED)
        include_directories(${JNI_INCLUDE_DIRS})
endmacro()