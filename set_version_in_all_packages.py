import re
import os

current_file_path = os.path.dirname(os.path.abspath(__file__))


def set_version_in_android_package(version):
    print(f"\nSetting CLAID version to {version} in Android (aar) package")
    android_package_path = f"{current_file_path}/dispatch/android"
    build_file = f"{android_package_path}/aar/claid/claid/build.gradle"

    replace_version_in_file(build_file, 'version = \'\d.\d.\d\'', f"version = '{version}'")

    replace_version_in_file(build_file, 'versionName "\d.\d.\d"', f"versionName \"{version}\"")

def set_version_in_flutter_package(version):
    print(f"\nSetting CLAID version to {version} in Flutter (pub) package")
    flutter_package_path = f"{current_file_path}/dispatch/dart"
    
    build_file = f"{flutter_package_path}/claid/android/build.gradle"
    replace_version_in_file(build_file, 'version \'\d.\d.\d\'', f"version '{version}'")

    pubspec_file = f"{flutter_package_path}/claid/pubspec.yaml"
    replace_version_in_file(pubspec_file, 'version: \d.\d.\d', f"version: {version}")


def set_version_in_python_package(version):
    print(f"\nSetting CLAID version to {version} in Python (pip) package")
    python_package_path = f"{current_file_path}/dispatch/python"
    setup_file = f"{python_package_path}/claid/setup.py"

    replace_version_in_file(setup_file, 'version=\'\d.\d.\d\'', f"version='{version}'")



def replace_version_in_file(file_path, regex_pattern, new_text):
    print("Modifying file " + file_path)
    # Define the regex pattern to match the version string with any content
    version_pattern = re.compile(regex_pattern)

    # Read the contents of the file
    with open(file_path, 'r') as file:
        content = file.read()

    # Replace the old version with the new version
    new_content = version_pattern.sub(new_text, content)

    # Write the modified content back to the file
    with open(file_path, 'w') as file:
        file.write(new_content)

def load_claid_version():
    version_file_path = f"{current_file_path}/CLAID_VERSION.txt"

    if not os.path.exists(version_file_path) or not os.path.isfile(version_file_path):
        print(f"Cannot change CLAID version in files! File {version_file_path} does not exist.")
        exit(1)

    # Define the regex pattern to match a version string in the format \d.\d.\d
    version_pattern = re.compile(r'^\d+\.\d+\.\d+$')

    # Read the contents of the file
    with open(version_file_path, 'r') as file:
        lines = file.readlines()

    # Verify that the file contains only one line
    num_lines = len(lines)
    if num_lines != 1:
        raise ValueError("Invalid CLAID version in file \"{version_file_path}\". Expected file to have 1 line but found {num_lines}")

    # Extract the version string and strip any surrounding whitespace
    version_string = lines[0].strip()

    # Verify that the version string matches the pattern \d.\d.\d
    if not version_pattern.match(version_string):
        raise ValueError("The version string is not in the correct format (major.minor.patch).")

    # Split the version string into major, minor, and patch components
    major, minor, patch = version_string.split('.')

    print(f"Loaded CLAID version {version_string}")
    print(f"\tmajor: {major}")
    print(f"\tminor: {minor}")
    print(f"\tpatch: {patch}")

    return version_string

