# main.py
import argparse

from claid import CLAID

def hello_world(args):
    print("Instantiating python")
    claid = CLAID()
    claid.hello_world()

    print("Hello world!")

def install(args):
    print('Installing...')

def main():
    parser = argparse.ArgumentParser(description='Command-line tool for claid')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Create parser for the 'hello' command
    hello_parser = subparsers.add_parser('hello_world', help='Greet someone')

    # Create parser for the 'install' command
    install_parser = subparsers.add_parser('install', help='Install something')

    args = parser.parse_args()

    # Call the appropriate function based on the command
    if args.command == 'hello_world':
        hello_world(args)
    elif args.command == 'install':
        install(args)
    else:
        print('Unknown command')

if __name__ == '__main__':
    main()
