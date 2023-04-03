@echo off
SET mypath=%~dp0
echo "Installing CLAID into %mypath:~0,-1%"
setx CLAID_PATH "%mypath:~0,-1%"
setx CLAID_CMAKE_MODULE_PATH "%mypath:~0,-1%"\CMakeUtils\
setx CMAKE_PREFIX_PATH "%mypath:~0,-1%"\CMakeUtils\
setx /M PATH "%PATH%;<your-new-path>%mypath:~0,-1%"\claid
echo "CLAID installed successfully."
pause