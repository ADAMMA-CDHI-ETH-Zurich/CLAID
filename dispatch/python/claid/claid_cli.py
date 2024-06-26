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

from claid_cli_utils.create_package import create_package_prompt

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



def main():
    parser = argparse.ArgumentParser(description='Command-line tool for claid')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Create parser for the 'hello' command
    hello_parser = subparsers.add_parser('hello_world', help='Test the CLAID middleware on your device.')
    device_info_parser = subparsers.add_parser('device_info', help='Get device info (use this to report errors to the CLAID team).')

    # Create parser for the 'install' command
    install_parser = subparsers.add_parser('create_package', help='Create a new CLAID package')

    args = parser.parse_args()

    # Call the appropriate function based on the command
    if args.command == 'hello_world':
        hello_world(args)
    elif args.command == 'device_info':
        device_info(args)
    elif args.command == 'create_package':
        create_package_prompt(args)
    else:
        print('Unknown command')

if __name__ == '__main__':
    main()
