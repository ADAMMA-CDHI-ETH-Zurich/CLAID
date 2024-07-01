import os
import shutil
from set_version_in_all_packages import *
import time
current_file_path = os.path.dirname(os.path.abspath(__file__))
output_folder = f"{current_file_path}/build_packages"

version = load_claid_version()

sign_maven = False
sign_maven_mail = ""

def create_output_folder():

    print(f"Built packages will be stored under {output_folder}")

    if os.path.exists(output_folder):
        print(f"Cleaning output folder")
        try:
            shutil.rmtree(output_folder)
        except Exception as e:
            print("Error: " + str(e))
            exit(1)

    try:
        print("Creating output folder")
        os.makedirs(output_folder)
    except Exception as e:
        print("Error: " + str(e))
        exit(1)        


def make_android_package():
    print("Compiling android (aar) package")
    make_path = f"{current_file_path}/dispatch/android"
    os.chdir(make_path)
    os.system("make aar_package")

    aar_path = f"{current_file_path}/dispatch/android/aar/claid/claid/build/outputs/aar/claid-release.aar"
    package_output_path = f"{output_folder}/android/"

    try:
        os.makedirs(package_output_path)
    except Exception as e:
        print("Error: " + str(e))
        exit(1)  

    print(f"Copying {aar_path} to {package_output_path}/claid-release.aar")
    shutil.copyfile(aar_path, package_output_path + "/claid-release.aar")

    print(f"Copying {aar_path} to {package_output_path}/claid-debug.aar")
    shutil.copyfile(aar_path, package_output_path + "/claid-debug.aar")

    shutil.copy(f"{current_file_path}/CHANGELOG.md", package_output_path)


    package_android_maven()

def package_android_maven():
    print("Creating android maven release package")
    make_path = f"{current_file_path}/dispatch/android"
    os.chdir(make_path)
    os.system("make maven_publish")

    maven_path = os.path.expanduser(f"~/.m2/repository/ch/claid/claid/{version}")
    maven_output_path = f"{output_folder}/android/maven/ch/claid/claid/{version}"

    try:
        os.makedirs(maven_output_path)
    except Exception as e:
        print("Error: " + str(e))
        exit(1)  

    print(f"Copying {maven_path} to {maven_output_path}")
    shutil.copytree(maven_path, maven_output_path, dirs_exist_ok=True)

def make_flutter_package():
    print("Creating flutter (pub) package")

    flutter_path = f"{current_file_path}/dispatch/dart/claid"
    flutter_output_path = f"{output_folder}/flutter/"

    try:
        os.makedirs(flutter_output_path)
    except Exception as e:
        print("Error: " + str(e))
        exit(1)  

    os.chdir(flutter_path)
    os.system("make flutter_package")

    print(f"Copying {flutter_path} to {flutter_output_path}")
    shutil.copytree(flutter_path, flutter_output_path, dirs_exist_ok=True)

    shutil.copy(f"{current_file_path}/CHANGELOG.md", flutter_output_path)

def make_python_package():
    print("Creating python (pip) package")

    python_path = f"{current_file_path}/dispatch/python/claid"
    python_output_path = f"{output_folder}/python/"

    try:
        os.makedirs(python_output_path)
    except Exception as e:
        print("Error: " + str(e))
        exit(1)  

    os.chdir(python_path)
    os.system("make pip_package")

    print(f"Moving {python_path}/pip_package to {python_output_path}")
    shutil.move(os.path.join(python_path, "pip_package"), python_output_path)
    shutil.copy(f"{current_file_path}/CHANGELOG.md", python_output_path)

choice = ""
while(choice != "y" and choice != "n"):
    choice = input(f"The current CLAID version is set to {version}, proceed? [y|n]")

if(choice == "n"):
    print("Please change the CLAID version in the file CLAID_VERSION.txt")
    exit(0)

print(f"Continuing to create android (aar), flutter (pub) and python (pip) packages for CLAID version {version}")
time.sleep(1)


set_version_in_android_package(version)
set_version_in_flutter_package(version)
set_version_in_python_package(version)

create_output_folder()

time.sleep(1)
make_android_package()
time.sleep(1)
make_flutter_package()
time.sleep(1)
make_python_package()