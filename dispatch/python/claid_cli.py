# main.py
import argparse

from claid import CLAID
import platform
def hello_world(args):
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

def install(args):
    print('Installing...')

def main():
    parser = argparse.ArgumentParser(description='Command-line tool for claid')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Create parser for the 'hello' command
    hello_parser = subparsers.add_parser('hello_world', help='Test the CLAID middleware on your device.')
    device_info_parser = subparsers.add_parser('device_info', help='Get device info (use this to report errors to the CLAID team).')

    # Create parser for the 'install' command
    install_parser = subparsers.add_parser('install', help='Install something')

    args = parser.parse_args()

    # Call the appropriate function based on the command
    if args.command == 'hello_world':
        hello_world(args)
    elif args.command == 'device_info':
        device_info(args)
    elif args.command == 'install':
        install(args)
    else:
        print('Unknown command')

if __name__ == '__main__':
    main()
