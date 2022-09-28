#pragma once

#define Reflection(Class, code)\ 
    template<typename Reflector>\
    void reflect(Reflector& reflector)\
    {\
        typedef Class THIS_CLASS;\
        code\
    }\

#define r(mem) reflector.member(#mem, mem, ""); reflector.pointer_to_member(#mem, &THIS_CLASS::mem)