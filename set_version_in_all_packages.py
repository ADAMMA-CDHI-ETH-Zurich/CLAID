import re
import os

current_file_path = os.path.abspath(__file__)


def set_version_in_android_package(version):
    android_package_path = f"{current_file_path}/dispatch/android"

    #version = '0.6'
    replace_version_in_file("", r'version=\'[^"]*\'', f"version='{version}'")

    #"versionName \"0.6\""
    first_pattern = r'version="[^"]*"'
    first_pattern = r'version="[^"]*"'

def set_version_in_flutter_package():
    pass

def set_version_in_python_package():
    pass


def replace_version_in_file(file_path, regex_pattern, new_text):
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