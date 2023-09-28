from git import Repo
import common
import os

def is_git_repo_link(package_name):

    return package_name.endswith(".git")

def clone_package_from_git(git_path, branch, output_path):
    repo = Repo.clone_from(git_path, output_path, branch=branch, multi_options=["--recurse-submodules"])
    repo.submodule_update(recursive=True)

def package_exists(package_name):
    CLAID_PATH = common.get_claid_path()
    output_path = CLAID_PATH + "/packages/" + package_name

    return os.path.isdir(output_path)

def install_package_from_package_name(package_name):
    
    packages = common.parse_package_list()

    if package_name == "":
        raise Exception("Error, cannot install package. Please specify a package name, e.g.: claid install ModuleAPI")

    if package_name not in packages:
        raise Exception("Error, cannot install package {}. The package is unknown to CLAID.".format(package_name))
    

    if(package_exists(package_name)):
        print("Skipping package {}, because it already is installed.".format(package_name))
        exit(0)

    git_path, branch = packages[package_name]
    
    CLAID_PATH = common.get_claid_path()
    output_path = CLAID_PATH + "/packages/" + package_name

    print("Installing package ", package_name)

    clone_package_from_git(git_path, branch, output_path)

    print("Successfully installed package", package_name)

def get_package_name_from_git_link(git_link):

    # Fix windows path delimiters..
    git_link = git_link.replace("\\", "/")
    name = git_link.split("/")[-1]
    name = name.replace(".git", "")

    return name
    

def install_package_from_git_link(git_link):

    package_name = get_package_name_from_git_link(git_link)

    if(package_exists(package_name)):
        print("Skipping package {}, because it already is installed.", package_name)
        exit(0)

    CLAID_PATH = common.get_claid_path()
    output_path = CLAID_PATH + "/packages/" + package_name
    branch = "main"
    
    print("Installing package ", package_name)

    clone_package_from_git(git_link, branch, output_path)
    
    print("Successfully installed package", package_name)


def install_package(*package_names):

    for package_name in package_names:
        if(is_git_repo_link(package_name)):
            install_package_from_git_link(package_name)
        else:
            install_package_from_package_name(package_name)
