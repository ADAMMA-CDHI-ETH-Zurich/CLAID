
OUTPUT_LIB = bazel-bin/dispatch/core/libclaid_capi.so
TARGET_LIB = dispatch/dart/claid_core/blobs/libclaid_capi_linux.so

.PHONY: all
all: build

.PHONY: clean
clean:
	rm -f blobs/*.so

# Build everything and run a series of tests.
.PHONY: test
test: build
	flutter test ./test/

# Run the tests, gather coverage and generate a report.
.PHONY: coverage
coverage: build
	mkdir -p coverage
	flutter test --coverage ./test/
	rm -rf coverage/output/*
	mkdir -p coverage/output
	genhtml -o coverage/output/ coverage/lcov.info

# Build the middleware and generate the dart code.
.PHONY: build
build: build_middleware generate

# Build the middleware via bazel.
.PHONY: build_middleware
build_middleware:
	cd ../../..; rm -f $(TARGET_LIB)
	CC=clang bazel build //dispatch/core:claid_capi
	cd ../../..; cp $(OUTPUT_LIB) $(TARGET_LIB)

.PHONY: generate
generate: generate_dart generate_test_dart generate_ffi

.PHONY: generate_dart
generate_dart:
	mkdir -p lib/generated
	protoc -I../../proto/ --dart_out=grpc:lib/generated \
		../../proto/claidservice.proto \
		google/protobuf/timestamp.proto \
		google/protobuf/empty.proto

.PHONY: generate_test_dart
generate_test_dart:
	mkdir -p test/generated
	protoc -I./test/ --dart_out=grpc:test/generated \
		./test/testmessages.proto

.PHONY: generate_ffi
generate_ffi:
	dart run ffigen --config ffigen.yaml

#	protoc -I../../proto/ --dart_out=grpc:lib/generated ../../proto/claidservice.proto
