.PHONY: all
all: generate_dart

.PHONY: generate_dart
generate_dart:
	mkdir -p lib/generated
	protoc -I../../proto/ --dart_out=grpc:lib/generated \
		../../proto/claidservice.proto \
		google/protobuf/timestamp.proto

#	protoc -I../../proto/ --dart_out=grpc:lib/generated ../../proto/claidservice.proto
