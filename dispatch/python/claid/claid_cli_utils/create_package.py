import os
import shutil
from datetime import datetime

def package_name_is_valid(package_name: str):
    return package_name != ""

def namespace_is_valid(namespace_name: str):
    return namespace_name != ""

def output_path_is_valid(output_path: str):
    return output_path != ""

def replace_in_file(file_path: str, replacements: dict):
    try:
        # Read the content of the file
        print(f"Patching file'{file_path}'.")
        with open(file_path, 'r') as file:
            content = file.read()

        # Replace occurrences in the content
        for key, value in replacements.items():
            content = content.replace(key, value)

        # Write the modified content back to the file
        with open(file_path, 'w') as file:
            file.write(content)

        print(f"Patched file'{file_path}'.")

    except FileNotFoundError:
        print(f"File '{file_path}' not found.")
        exit(0)
    except Exception as e:
        print(f"An error occurred: {str(e)}")
        exit(0)

def create_package(package_name: str, namespace: str, output_path: str):
    
    if(output_path.startswith("./")):
        output_path = os.getcwd() + output_path[2:]

    if(os.path.exists(output_path)):
        if(not os.path.isdir(output_path)):
            print(f"Error! Path {output_path} exists but is not a directory. Cannot create package here.")
            return
    else:
        try:
            os.makedirs(output_path)
        except e as Exception:
            print(f"An error occurred: {str(e)}")
            exit(0)
        if(not os.path.exists(output_path)):
            print(f"Error! Failed to create folder {output_path}. Cannot create package here.")
            exit(0)

    complete_path = os.path.join(output_path, package_name)
    if(os.path.exists(complete_path)):
        print("Error, cannot create package. Package already exists under \"" + complete_path + "\"")
        exit(0)
    
    os.system(f"cd {output_path} && git clone https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAIDPackageTemplate.git {package_name}")
    
    #### Create symlinks
    print("Creating symlinks")
    os.system(f"cd {output_path}/{package_name}/packaging/flutter/claid_package/android/src/main && ln -s ../../../../../../src/android/java java")
    os.system(f"cd {output_path}/{package_name}/packaging/flutter/claid_package/android/src/main && ln -s ../../../../../../src/android/AndroidManifest.xml AndroidManifest.xml")
    os.system(f"cd {output_path}/{package_name}/packaging/flutter/claid_package/android/src/main && ln -s ../../../../../../src/cpp cpp")
    os.system(f"cd {output_path}/{package_name}/packaging/flutter/claid_package/android/src/main && ln -s ../../../../../../datatypes proto")
    os.system(f"cd {output_path}/{package_name}/packaging/flutter/claid_package/ && ln -s ../../../src/dart lib")
    os.system(f"cd {output_path}/{package_name}/packaging/flutter/claid_package/ && ln -s ../../../assets assets")

    os.system(f"cd {output_path}/{package_name}/packaging/android/claid_package/claid/src/main && ln -s ../../../../../../src/android/java java")
    os.system(f"cd {output_path}/{package_name}/packaging/android/claid_package/claid/src/main && ln -s ../../../../../../src/android/AndroidManifest.xml AndroidManifest.xml")
    os.system(f"cd {output_path}/{package_name}/packaging/android/claid_package/claid/src/main && ln -s ../../../../../../src/cpp cpp")
    os.system(f"cd {output_path}/{package_name}/packaging/android/claid_package/claid/src/main && ln -s ../../../../../../datatypes proto")
    os.system(f"cd {output_path}/{package_name}/packaging/android/claid_package/claid/src/main && ln -s ../../../../../../assets assets")
    

    files = ["packaging/android/claid_package/settings.gradle",
             "packaging/android/claid_package/claid/build.gradle",
             "packaging/flutter/claid_package/android/settings.gradle",
             "packaging/flutter/claid_package/android/build.gradle",
             "packaging/flutter/claid_package/pubspec.yaml",
            "src/cpp/CMakeLists.txt",
             "Makefile"]
    
    patches = {"${package_name}" : package_name, "${namespace}" : namespace}

    for file in files:
        path = os.path.join(output_path, package_name, file)

        replace_in_file(path, patches)
    
    package_folders = namespace.split(".")

    package_folder_path = f"{output_path}/{package_name}/packaging/android/claid_package/claid/src/main/java"
    for folder in package_folders:
        package_folder_path += "/" + folder 
        try:
            print("Creating folder " + package_folder_path)
            os.makedirs(package_folder_path)
        except e as Exception:
            print(f"An error occurred: {str(e)}")
            exit(0)

    ### Writing java package main
    java_package_main_file = f"{package_folder_path}/{package_name}.java"
    write_java_package_main(java_package_main_file, package_name, namespace)
    ### Done writing java package main

    package_file_name = namespace + "." + package_name
    package_file_name = package_file_name.replace(".", "_")
    package_file_name += ".claidpackage"
    print("Creating " + f"{output_path}/{package_name}/packaging/android/claid_package/claid/src/main/claid_package_assets/{package_file_name}")
    android_claidpackage_file_path = f"{output_path}/{package_name}/packaging/android/claid_package/claid/src/main/claid_package_assets/{package_file_name}"

    with open(android_claidpackage_file_path, "w") as file:
        file.write(f"{namespace}.{package_name}")


    print("Creating " + f"{output_path}/{package_name}/packaging/flutter/claid_package/android/src/main/claid_package_assets/{package_file_name}")
    flutter_claidpackage_file_path = f"{output_path}/{package_name}/packaging/flutter/claid_package/android/src/main/claid_package_assets/{package_file_name}"

    with open(flutter_claidpackage_file_path, "w") as file:
        file.write(f"{namespace}.{package_name}")

    ### Writing dart package main
    dart_package_main_file = f"{output_path}/{package_name}/packaging/flutter/claid_package/lib/{package_name}.dart"
    write_dart_package_main(dart_package_main_file, package_name, namespace)
    ### Done writing dart package main

    proto_data_type_path = f"{output_path}/{package_name}/datatypes/{package_name}_datatypes.proto"
    write_proto_data_type(proto_data_type_path, package_name, namespace)

    try:
        shutil.move(f"{output_path}/{package_name}/packaging/android/claid_package/claid", f"{output_path}/{package_name}/packaging/android/claid_package/{package_name}")
    except Exception as e:
        print(f"Error when moving folder: {str(e)}")

    try: 
        shutil.rmtree(f"{output_path}/{package_name}/.git")
    except Exception as e:
        print(f"Error when deleting folder: {str(e)}")


    print(f"Package {package_name} created successfully!")

def write_java_package_main(path, package_name, namespace):
    java_code = """package $namespace;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_android.Package.CLAIDPackageAnnotation;
import adamma.c4dhi.claid_android.Package.CLAIDPackage;

@CLAIDPackageAnnotation (
    authors = {"Your name"},
    date = "$date",
    description = "Package description",
    version = "0.1"
)
public class $package_name extends CLAIDPackage
{
    public void register()
    {
        Logger.logInfo("Loading $package_name");
        loadNativeComponent("$package_name");
    }
}
"""
    # Get current date
    current_date = datetime.now()
    formatted_date = current_date.strftime("%dth %B %Y")

    java_code = java_code.replace("$package_name", package_name)
    java_code = java_code.replace("$namespace", namespace)
    java_code = java_code.replace("$date", formatted_date)

    with open(path, 'w') as file:
        file.write(java_code)

def write_dart_package_main(path, package_name, namespace):
    dart_code = """import 'package:claid/package/CLAIDPackage.dart';
import 'package:claid/package/CLAIDPackageAnnotation.dart';
import 'package:claid/ui/CLAIDView.dart';


@CLAIDPackageAnnotation(
    authors: ['Your name'],
    date: '$date',
    description: 'Package description.',
    version: '0.1',
)
class $package_name extends CLAIDPackage
{
    @override
    void register()
    {

    }
}
"""
    # Get current date
    current_date = datetime.now()
    formatted_date = current_date.strftime("%dth %B %Y")

    dart_code = dart_code.replace("$package_name", package_name)
    dart_code = dart_code.replace("$namespace", namespace)
    dart_code = dart_code.replace("$date", formatted_date)
    with open(path, 'w') as file:
        file.write(dart_code)

def write_proto_data_type(path, package_name, namespace):
    proto_code = """syntax="proto3";
package $package_name;
option java_package = "$namespace";
option java_outer_classname = "$package_name_datatypes";
option java_multiple_files = true;


"""
    proto_code = proto_code.replace("$package_name", package_name)
    proto_code = proto_code.replace("$namespace", namespace)
    with open(path, 'w') as file:
        file.write(proto_code)

def create_package_prompt(args):
    package_name = ""
    namespace = ""
    output_path = ""
    
    while(not package_name_is_valid(package_name)):
        package_name = input("Please enter a name for your package (e.g., \"claid_smartwatch_package\"): ")
        if(not package_name_is_valid(package_name)):
            print("Invalid package name!")
    

    while(not namespace_is_valid(namespace)):
        namespace = input("Please enter a namespace for your package (e.g., \"ch.claid.smartwatch_package\"): ")
        if(not namespace_is_valid(namespace)):
            print("Invalid namespace!")
    
    while(not output_path_is_valid(output_path)):
        output_path = input("Please enter the path where you want to create the package. If you want to create it in the current folder, use \"./\": ")
        if(not output_path_is_valid(output_path)):
            print("Invalid output path!")
    
    confirm = ""
    
    while(confirm != "y" and confirm != "n"):
        confirm = input(f"Creating package with name \"{package_name}\" and namespace \"{namespace}\" in path \"{output_path}\"? [y|n]")

    if(confirm == "y"):
        create_package(package_name, namespace, output_path)