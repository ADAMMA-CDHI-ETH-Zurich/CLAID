from git import Repo
import common
import os
from install import install_package

import shutil


def is_git_repo_link(package_name):

    return package_name.endswith(".git")


def package_exists(package_name):
    CLAID_PATH = common.get_claid_path()
    output_path = CLAID_PATH + "/packages/" + package_name

    return os.path.isdir(output_path)

def update_package_from_package_name(package_name):
    
    packages = common.parse_package_list()

    CLAID_PATH = common.get_claid_path()
    output_path = CLAID_PATH + "/packages/" + package_name


    if(package_exists(package_name)):
        print("Uninstalling package {} ({}).".format(package_name, output_path))
        try:
            shutil.rmtree( output_path)
        except:
            print("Failed to uninstall package {}: Failed to delete folder {}.".format(package_name, output_path))
            return

    install_package(package_name)

def get_package_name_from_git_link(git_link):

    # Fix windows path delimiters..
    git_link = git_link.replace("\\", "/")
    name = git_link.split("/")[-1]
    name = name.replace(".git", "")

    return name
    

def update_package_from_git_link(git_link):

    package_name = get_package_name_from_git_link(git_link)

    CLAID_PATH = common.get_claid_path()
    output_path = CLAID_PATH + "/packages/" + package_name

    if(package_exists(package_name)):
        print("Uninstalling package {} ({}).".format(package_name, output_path))
        try:
            shutil.rmtree( output_path)
        except:
            print("Failed to uninstall package {}: Failed to delete folder {}.".format(package_name, output_path))
            return

    
    install_package(git_link)



def update_package(*package_names):

    for package_name in package_names:
        if(is_git_repo_link(package_name)):
            update_package_from_git_link(package_name)
        else:
            update_package_from_package_name(package_name)
