cross_compile_ios()
{
   architecture=$1
   echo $architecture
   path=libs/ios/${architecture}
   mkdir -p ${path}

   cd tensorflow
   bazel build --copt=-Ofast --apple_bitcode=embedded --config=v2 --verbose_failures --config=ios_${architecture} --distinct_host_configuration //tensorflow/lite:libtensorflowlite.so
   
   rm  -f ../${path}/libtensorflowlite.so
   cp bazel-bin/tensorflow/lite/libtensorflowlite.so ../${path}/libtensorflowlite.so
   cd ..


   echo "Warning: library /${path}/libtensorflowlite.so for iOS was built, but not signed."
   echo "Therefore, you very likely will get an error when trying to use it on iOS"
   echo "Please run codesign -s codesign -s your_identity@something.com ${path}/libtensorflowlite.so manually." 
   echo "Use security find-identity -v -p codesigning to find available identities."
   # codesign -s your_identity@something.com libtensorflowlite.so

}

# TODO: Add monkey patch automatically.
echo "build for iOS not supported out of the box, requires small change in tensorflow lite BUILD config."
exit 1


. ./verify_hash.sh

mkdir libs
cross_compile_ios "arm64"

# Does not work with tensorflow lite C++ API.
#cross_compile_ios "armv7"

# It seems, however, that only arm64 is relevant anyways for iOS:
    # arm64 is the current 64-bit ARM CPU architecture, as used since the iPhone 5S and later (6, 6S, SE and 7), the iPad Air, Air 2 and Pro, with the A7 and later chips.
    # armv7s (a.k.a. Swift, not to be confused with the language of the same name), being used in Apple's A6 and A6X chips on iPhone 5, iPhone 5C and iPad 4.
    # armv7, an older variation of the 32-bit ARM CPU, as used in the A5 and earlier.

