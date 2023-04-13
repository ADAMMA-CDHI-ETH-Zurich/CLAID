import os

def get_claid_path():
    CLAID_PATH = ""
    if "CLAID_PATH" in os.environ:
        CLAID_PATH = os.environ.get("CLAID_PATH")
    else:
        raise Exception("Error, cannot parse package list file. Environment variable CLAID_PATH is not set.\n"
                        "Did you install python before?")
    return CLAID_PATH

def parse_package_list():
    CLAID_PATH = get_claid_path()

    package_list_path = CLAID_PATH + "/package_list.txt"
    
    if(not os.path.isfile(package_list_path)):
        raise Exception("Error, cannot parse package list file. File {} does not exist.".format(package_list_path))
    
    file = open(package_list_path, 'r')
    Lines = file.readlines()
        
    packages = dict()

    for line in Lines:
        line = line.replace("\n", "")
        package_name, repo_path, branch = line.split(";")
        packages[package_name] = (repo_path, branch)

    return packages