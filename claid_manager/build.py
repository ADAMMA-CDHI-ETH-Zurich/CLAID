import subprocess
import os

def build(*args):
    
    if(not os.path.isfile("./CMakeLists.txt")):
        print("claid build failed; there is no CMakeLists.txst in the current directory.\n"
              "Please keep in mind, claid build only works for C++ and Java applications for now.\n"
              "For Android, iOS and WearOS, please open the projects separately in Android Studio or XCode.")
        return


    cmake_process = subprocess.Popen(["cmake", "."], stderr=subprocess.STDOUT)
    if cmake_process.wait() != 0:
        print("claid build failed")

    make_process = subprocess.Popen(["cmake", "--build", ".", "-j"], stderr=subprocess.STDOUT)
    if make_process.wait() != 0:
        print("claid build failed")
    
