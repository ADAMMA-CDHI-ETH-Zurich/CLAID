#pragma once
// Why do we have our own byte definition?
// We could ofc use uint8_t as byte. However, it might interfere with the definition of char (character).
// Because on some plattforms and depending on compilers, sometimes "char" is signed (i.e., signed char), on others it might be unsigned (i.e., unsigned char).
// That means that in our RecursiveReflector, we might have the problem that it cannot distinguish "char" from uint8_t, if char is defined as "unsigned char".
// However, we want to be able to distinguish them both, as serializers might treat a byte (i.e., 8 bit value) differently from a character.
// For example, take the JavaNativeGetter and JavaNativeSetter. They would map byte to java.lang.Byte class and char to java.lang.Character class.
// Also, java.lang.Character is 16 bits in Java, while a character in C++ is always 8 bit.
#ifndef byte
enum class byte : unsigned char
{

};
#endif