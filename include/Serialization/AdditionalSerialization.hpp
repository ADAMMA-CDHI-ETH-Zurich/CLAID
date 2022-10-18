#pragma once

// Add every additional De/Serializers or registration helpers (compared to the standard ones such as XMLDeSerializers and BinaryDeSerializers, registration to ClassFactory),
// that you need here.


#ifdef __ANDROID__
    #include "JavaWrapper/JavaNativeSetter.hpp"
    #include "JavaWrapper/JavaNativeGetter.hpp"
    #include "JavaWrapper/WrapperMaster.hpp"
#endif

#ifdef __ANDROID__ 
    #define DECLARE_JAVA_DE_SERIALIZERS(className) \
        DECLARE_JAVA_WRAPPER(className) \
        DECLARE_POLYMORPHIC_REFLECTOR(className, claid::JavaWrapper::JavaNativeSetter, JavaNativeSetter)\
        DECLARE_POLYMORPHIC_REFLECTOR(className, claid::JavaWrapper::JavaNativeGetter, JavaNativeGetter)
#else
    #define DECLARE_JAVA_DE_SERIALIZERS(className) 
#endif 


#ifdef __ANDROID__ 
    #define REGISTER_JAVA_DE_SERIALIZERS(className) \
        REGISTER_JAVA_WRAPPER(className) \
        REGISTER_POLYMORPHIC_REFLECTOR(className, claid::JavaWrapper::JavaNativeSetter, JavaNativeSetter)\
        REGISTER_POLYMORPHIC_REFLECTOR(className, claid::JavaWrapper::JavaNativeGetter, JavaNativeGetter)
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


#define DECLARE_ADDITIONAL_DE_SERIALIZERS(className) \
    DECLARE_JAVA_DE_SERIALIZERS(className)\
    DECLARE_PYTHON_DE_SERIALIZERS(className)



#define REGISTER_ADDITIONAL_DE_SERIALIZERS(className) \
    REGISTER_JAVA_DE_SERIALIZERS(className)\
    REGISTER_PYTHON_DE_SERIALIZERS(className)
