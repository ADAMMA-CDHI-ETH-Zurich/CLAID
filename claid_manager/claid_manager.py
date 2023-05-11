import argparse
from git import Repo
from install import install_package
from create import create
from build import build 
 
args = ""


def init():
    print("claid manager init")
    parser = argparse.ArgumentParser()
    parser.add_argument("command")
    parser.add_argument("arg1", nargs="?", default="", type=str)
    parser.add_argument("arg2", nargs="?", default="", type=str)
    parser.add_argument("arg3", nargs="?", default="", type=str)

    args = parser.parse_args()

    print(args.command)
    map = [("list_packages", list_packages),
           ("install", install_package),
           ("create", create),
           ("build", build)
           ]

    for entry in map:
        name, func = entry

        if(name == args.command):
            func(args.arg1, args.arg2, args.arg3)

def list_packages(arg1, *args):
    print("listing packages")

