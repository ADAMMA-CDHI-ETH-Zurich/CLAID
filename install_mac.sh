CURRENT_PATH=$(pwd)
echo ${CURRENT_PATH}

echo "export CLAID_PATH=${CURRENT_PATH}" > ~/.claidrc
echo "export CMAKE_PREFIX_PATH=\${CMAKE_PREFIX_PATH}:${CURRENT_PATH}/CMakeUtils/" >> ~/.claidrc
echo "export CLAID_CMAKE_MODULE_PATH=${CURRENT_PATH}/CMakeUtils/" >> ~/.claidrc
echo "export PATH=\${PATH}:${CURRENT_PATH}/claid_manager/" >> ~/.claidrc



if [[ -z "${CLAID_IN_ZSHRC}" ]]; then
    # Make sure ~/.claidrc get's sourced on startup.
    echo "export CLAID_IN_ZSHRC=1" >> ~/.zshrc
    echo ". ~/.claidrc" >> ~/.zshrc
else
    # CLAID_INSTALLED is already set, that means there already should be a ". ~/.claidrc" in the ~/.zshrc.
    # Thus, we do not need to set it again. This might happen if the user moved the CLAID directory and wants to install again 
    # When uninstalling CLAID, we still keep ". ~/.claidrc" in the ~/.zshrc, but just remove it's contents.
    echo " "

fi
source ~/.claidrc
chmod u+x claid/claid

echo "Installed CLAID successfully (do not forget to source ~/.zshrc or restart your terminal in order to use CLAID)."