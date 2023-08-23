import os
import shutil
import pathlib
from glob import glob

def resolve_path(path):
    destination_path = path

    if(path == "here"):
        destination_path = os.getcwd()

    return destination_path

def get_source_path():
    return pathlib.Path(__file__).parent.resolve()
    

def create_application_folder(path):  
    path = resolve_path(path)

    try:
        os.makedirs(os.path.join(path, "src"))
        os.makedirs(os.path.join(path, "include"))
    except:
        pass
def create(type, path, *args):

    if(path == ""):
        raise Exception("Error, no path specified for create. Please specify a path where to create an application or package."
                        "You can also use \"here\", to create it in the current directory, e.g.: claid create application here.")

    # Why not use template repositories to just clone the files 
    # from a public github etc?
    # I want this to work offline. In firewall restricted environments, this could be a pain otherwise.
    # Especially if you have an embedded device at a company, which can not access the web.
    # Trust me, I've been there.
    # Yes, of course, in order to use the claid_manager (or CLAID) in general, you have to have
    # access to the web in the first place. But for this, you could also clone the repo externally and copy it to the
    # embedded device. As soon as claid is installed on a device, you should be able to use it offline.
    if(type == "application"):
        create_application(path)
    elif(type == "java_application"):
        create_java_application(path)
    elif(type == "android_application"):
        create_android_application(path)
    elif(type == "wearos_application"):
        create_wearos_application(path)
    else:
        raise Exception("Keyword {} is unknown for claid create. Options are: [application, java_application].".format(type))

    print("Created application \"{}\" successfully.".format(path))

def create_application(path):
    create_application_folder(path)
    copy_files_from_template_folder(path, "Cpp")

    print("Application \"{}\" created successfully.".format(path))

def create_java_application(path):
    create_application_folder(path)
    copy_files_from_template_folder(path, "Java")
    
    print("Application \"{}\" created successfully.".format(path))

def create_android_application(path):
    create_application_folder(path)
    copy_files_from_template_folder(path, "AndroidCLAID")

def create_wearos_application(path):
    create_application_folder(path)
    copy_files_from_template_folder(path, "WearOSCLAID")

def copy_files_from_template_folder(path, application_type):
    path = resolve_path(path)

    source_path = get_source_path()
    application_source_path = os.path.join(source_path, "ApplicationTypes", application_type)

    files = [os.path.relpath(file, start=application_source_path)
             for x in os.walk(application_source_path) for file in glob(os.path.join(x[0], '*')) 
             if os.path.isfile(file)]


    for file in files:
        file_source_path = os.path.join(application_source_path, file)
        file_destination_path = os.path.join(path, file)

        try:
            os.makedirs(pathlib.Path(file_destination_path).parent)
        except:
            pass

        shutil.copy(file_source_path, file_destination_path)