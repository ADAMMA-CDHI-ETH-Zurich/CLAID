## Bazel

For background on Bazel see 

https://bazel.build/

The best option to install bazel is a bazelisk 

https://github.com/bazelbuild/bazelisk

which will always download the correct Bazel version to use.

## Protobuf and gRPC 

For details about Protobuf anf gRPC see: 

https://protobuf.dev/
https://grpc.io/

## BUILDING packages

To build a package current you want to make sure you have a compiler 
fitting compiler installed. 

Linux:  both gcc and clang work. To build with clang you currently 
have to run. 

```bash
$ CC=clang bazel build :claid_dispatcher 
```

where '''claid_dispatcher''' is a target defined in the current
BUILD.bazel file in the current directory. 

Note: The exact compiler to use requires more work. Bazel allows 
to control this by configuring a toolchain. See:


https://bazel.build/extending/toolchains

## Testing 

Currently we are using the 'googletest' testing framework. 
See: http://google.github.io/googletest/
This could easily be switched out for a different framework. 

To (build and) run a test target execute: 

bazel test :local_dispatching_test 

This assumes you are in the directory that defines that test target. 
