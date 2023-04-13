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

    os.makedirs(os.path.join(path, "src"))
    os.makedirs(os.path.join(path, "include"))

def create(type, path, *args):

    print(type,  path)
    if(path == ""):
        raise Exception("Error, no path specified for create. Please specify a path where to create an application or package."
                        "You can also use \"here\", to create it in the current directory, e.g.: claid create application here.")

    if(type == "application"):
        create_application(path)
    elif(type == "java_application"):
        create_java_application(path)
    else:
        raise Exception("Keyword {} is unknown for claid create. Options are: [application, java_application].".format(type))


def create_application(path):
    create_application_folder(path)

def create_java_application(path):
    create_application_folder(path)
    
    
    path = resolve_path(path)

    source_path = get_source_path()
    application_source_path = os.path.join(source_path, "ApplicationTypes", "Java")

    files = [os.path.relpath(file, start=application_source_path)
             for x in os.walk(application_source_path) for file in glob(os.path.join(x[0], '*')) 
             if os.path.isfile(file)]


    for file in files:
        print(file, source_path, application_source_path)
        file_source_path = os.path.join(application_source_path, file)
        file_destination_path = os.path.join(path, file)

        try:
            os.makedirs(pathlib.Path(file_destination_path).parent)
        except:
            pass

        shutil.copy(file_source_path, file_destination_path)



        


