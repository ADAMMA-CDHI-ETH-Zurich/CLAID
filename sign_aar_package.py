from set_version_in_all_packages import *
import shutil

current_file_path = os.path.dirname(os.path.abspath(__file__))
output_folder = f"{current_file_path}/build_packages"

version = load_claid_version()

sign_maven_mail = ""


    
def sign_maven_package():
    print("Signing maven package")
    maven_output_path = f"{output_folder}/android/maven/ch/claid/claid/{version}"

    print(f"Copying {current_file_path}/sign_maven.sh to {maven_output_path}/sign_maven.sh")
    shutil.copy(f"{current_file_path}/sign_maven.sh", f"{maven_output_path}/sign_maven.sh")

    replace_dict = {"$gpg_mail" : sign_maven_mail, "$claid_version" : version}
    replace_in_file(f"{maven_output_path}/sign_maven.sh", replace_dict)

    os.chdir(f"{maven_output_path}")
    os.system("sh ./sign_maven.sh")

    os.remove(f"{maven_output_path}/sign_maven.sh")
    os.chdir(f"{output_folder}/android/maven")
    os.system(f"zip {output_folder}/android/maven_release.zip ch/claid/claid/{version}/*")

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





sign_maven_mail = input("In order to publish the CLAID android package to maven central, it needs to be signed with a gpg key associated with an e-mail.\n"
                        + "Which e-mail do you want to use? Make sure you created a valid gpg key! Enter the mail to use: ")
sign_maven_package()