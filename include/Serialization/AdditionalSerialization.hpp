#pragma once

// Add every additional De/Serializers or registration helpers 
// (in addition to the standard ones such as XMLDeSerializers and BinaryDeSerializers, 
// registration to ClassFactory), that you need here.


#if defined(__ANDROID__) || defined(__JAVA_WRAPPERS__)
    #include "JavaWrapper/JavaWrapperMaster.hpp"
#endif

#if defined(__ANDROID__) || defined(__JAVA_WRAPPERS__)
    #define DECLARE_JAVA_DE_SERIALIZERS(className) \
        DECLARE_JAVA_WRAPPER(className) 
#else
    #define DECLARE_JAVA_DE_SERIALIZERS(className) 
#endif 


#if defined(__ANDROID__) || defined(__JAVA_WRAPPERS__)
    #define REGISTER_JAVA_DE_SERIALIZERS(className) \
        REGISTER_JAVA_WRAPPER(className) 
#else
    #define REGISTER_JAVA_DE_SERIALIZERS(className) 
#endif 

#ifdef __PYTHON_WRAPPERS__
    #include "PythonWrapper/PythonWrapperMaster.hpp"
#endif


#ifdef __PYTHON_WRAPPERS__
    #define DECLARE_PYTHON_DE_SERIALIZERS(className) DECLARE_PYTHON_WRAPPER(className)
    #define REGISTER_PYTHON_DE_SERIALIZERS(className) REGISTER_PYTHON_WRAPPER(className)
#else
    #define DECLARE_PYTHON_DE_SERIALIZERS(className)
    #define REGISTER_PYTHON_DE_SERIALIZERS(className)
#endif



#define REGISTER_ADDITIONAL_DE_SERIALIZERS(className) \
    REGISTER_JAVA_DE_SERIALIZERS(className)\
    REGISTER_PYTHON_DE_SERIALIZERS(className)
