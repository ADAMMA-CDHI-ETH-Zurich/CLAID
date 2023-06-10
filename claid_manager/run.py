import os
import subprocess

def extract_excutable_name(string):
    
    identifier = "add_executable"
    index = len(identifier) + 1

    # Remove Whitespaces and newlines
    while index < len(string):
        if string[index] not in ['\n', '\t', ' ']:
            break
        index += 1

    executable_name = ""
    while index < len(string):
        executable_name += string[index]
        index += 1

        character = string[index]
        if(character in ['\n', ' ', ')', ',']):
            break

    print(executable_name)
    return executable_name
        

def run_application(application_name, arguments):


    command = list()
    command.append("./{}".format(application_name))
      
    for arg in arguments:
        command.append(arg)

    print(command)

    try:
        make_process = subprocess.Popen(command, stderr=subprocess.STDOUT)
        if make_process.wait() != 0:
            print("claid run failed")
    except:
        pass

def run(*args):
    
    if(len(args) > 0):
        if(args[0] == "application"):
            print("bla")

            if(len(args) > 1):
                run_application(args[1], args[2:])
                return


    if(not os.path.isfile("./CMakeLists.txt")):
        print("claid run failed; there is no CMakeLists.txst in the current directory, hence there seems to not be a CLAID application to run here.\n"
              "Please keep in mind, claid build only works for C++ and Java applications for now.\n"
              "For Android, iOS and WearOS, please open the projects separately in Android Studio or XCode.")
        return
     
    with open('CMakeLists.txt', 'r') as file:
        data = file.read()
        

        index = data.find("add_executable")

        executable_names = list()

        while index != -1:
            
            substring = data[index:]
            executable_name = extract_excutable_name(substring)
            executable_names.append(executable_name)

            index = data.find("add_executable", index + 1)


    if(len(executable_names) == 0):
        print("claid run failed; cannot find name of executable in CMakeLists.txt file."
              "Please note, that claid run currently only works for CLAID C++ projects.")
    elif(len(executable_names) == 1):
        run_application(executable_names[0], args)
    else:
        print("claid run failed; found multiple possible applications to run: ", executable_name, 
              "Please specify which one to run using \"claid run --application application_name [parameter]\".")

