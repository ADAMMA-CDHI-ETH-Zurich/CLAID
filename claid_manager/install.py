from git import Repo
import common

def install_package(package_name, *args):
    print("Installing package", package_name)
    packages = common.parse_package_list()

    if package_name == "":
        raise Exception("Error, cannot install package. Please specify a package name, e.g.: claid install ModuleAPI")

    if package_name not in packages:
        raise Exception("Error, cannot install package {}. The package is unknown to CLAID.".format(package_name))
    
    repo_path, branch = packages[package_name]

    CLAID_PATH = common.get_claid_path()

    repo = Repo.clone_from(repo_path, CLAID_PATH + "/packages/" + package_name, branch=branch)
    repo.submodule_update(recursive=True)

    print("Successfully installed package", package_name)