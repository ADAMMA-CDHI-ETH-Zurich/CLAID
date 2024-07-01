###########################################################################
# Copyright (C) 2023 ETH Zurich
# CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
# Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
# Centre for Digital Health Interventions (c4dhi.org)
# 
# Authors: Patrick Langer
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#         http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

# main.py
import argparse
import os
from claid_cli_utils.create_package import create_package_prompt
from claid_cli_utils.check_protoc import *

import platform
def hello_world(args):
    from claid import CLAID

    print("Instantiating PyCLAID")
    claid = CLAID()
    claid.hello_world()

    print("Hello world!")

def device_info(args):
    # Get the operating system name
    os_name = platform.system()

    # Get the operating system version
    os_version = platform.version()

    # Get the machine architecture (e.g., 'x86_64', 'arm64')
    architecture = platform.machine()

    # Get additional information (e.g., 'Windows-10-10.0.19041-SP0', 'Linux-5.4.0-91-generic-x86_64')
    additional_info = platform.platform()

    # Print the information
    print(f"Operating System: {os_name}")
    print(f"Version: {os_version}")
    print(f"Architecture: {architecture}")
    print(f"Additional Info: {additional_info}")

def check_sdk(args):
    print("Verifying CLAID SDK...")
    path = ""
    try:
        path = os.environ["CLAID_SDK_HOME"]
    except:
        print("CLAID SDK not found! Environment variable \"CLAID_SDK_HOME\" is not set.")
        exit(1)

    print(f"Found CLAID_SDK_HOME pointing to {path}, checking files...")

    if not os.path.isdir(path):
        print(f"Invalid CLAID SDK or path! The folder {path} is not a directory")
        exit(1)

    if not os.path.isfile(f"{path}/bin/android/claid-debug.aar"):
        print(f"Invalid CLAID SDK! Could not find android libraries under {path}")
        exit(1)

    print("Found valid CLAID SDK!")

def check_buildtools(args):
    print("Checking if protobuf compiler works for dart")
    # Example usage
    is_protoc_working, message = check_protoc_for_dart()

    if not is_protoc_working:
        print("Error! Protobuf compiler not working for dart: " + message)
        exit(1)

    print("Protobuf compiler works!")

    print("Checking if Android Studio tools are installed")
    android_home = ""
    try:
        android_home = os.environ["ANDROID_HOME"]
    except:
        print("Android Studio tools not found! Environment variable ANDROID_HOME is not set.")
        exit(1)

    print("All build tools set! Ready to build CLAID packages.")


def main():
    parser = argparse.ArgumentParser(description='Command-line tool for claid')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Create parser for the 'hello' command
    hello_parser = subparsers.add_parser('hello_world', help='Test the CLAID middleware on your device.')
    device_info_parser = subparsers.add_parser('device_info', help='Get device info (use this to report errors to the CLAID team).')

    check_sdk_parser = subparsers.add_parser('create_package', help='Create a new CLAID package')

    check_sdk_parser = subparsers.add_parser('check_sdk', help='Check if the CLAID SDK is installed')

    check_buildtools_parser = subparsers.add_parser('check_buildtools', help='Check if the build tools (Android Studio, Protobuf, Flutter) are installed')


    args = parser.parse_args()

    # Call the appropriate function based on the command
    if args.command == 'hello_world':
        hello_world(args)
    elif args.command == 'device_info':
        device_info(args)
    elif args.command == 'create_package':
        create_package_prompt(args)
    elif args.command == 'check_sdk':
        check_sdk(args)
    elif args.command == 'check_buildtools':
        check_buildtools(args)
    else:
        print('Unknown command')

if __name__ == '__main__':
    main()
