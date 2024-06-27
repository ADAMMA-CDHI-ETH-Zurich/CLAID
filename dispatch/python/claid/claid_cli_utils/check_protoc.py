import subprocess
import tempfile
import os

def check_protoc_for_dart():
    try:
        # Check if protoc is installed
        result = subprocess.run(['protoc', '--version'], capture_output=True, text=True)
        if result.returncode != 0:
            return False, "protoc is not installed"

        # Create a temporary directory to store the .proto and generated files
        with tempfile.TemporaryDirectory() as temp_dir:
            proto_file_path = os.path.join(temp_dir, 'test.proto')
            dart_out_dir = os.path.join(temp_dir, 'dart_out')
            os.mkdir(dart_out_dir)

            # Create a simple .proto file
            proto_content = '''
            syntax = "proto3";
            package test;
            message TestMessage {
                string message = 1;
            }
            '''
            with open(proto_file_path, 'w') as proto_file:
                proto_file.write(proto_content)

            # Run protoc to generate Dart files
            result = subprocess.run(['protoc', '--dart_out=.',
                                      f"-I{temp_dir}",
                                      proto_file_path], cwd=dart_out_dir, capture_output=True, text=True)
            if result.returncode != 0:
                return False, f"protoc failed for Dart with error: {result.stderr}"

            # Check if the Dart file was generated
            dart_files = os.listdir(dart_out_dir)
            if not any(file.endswith('.dart') for file in dart_files):
                return False, "protoc did not generate Dart files"

        return True, "protoc for Dart works"
    except Exception as e:
        return False, str(e)

