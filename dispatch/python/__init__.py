import sys
import os
current_file_path = os.path.abspath(__file__)

# Get the directory containing the currently executed Python file
current_directory = os.path.dirname(current_file_path)

sys.path.append(current_directory)
sys.path.append(current_directory + "/dispatch/python")
sys.path.append(current_directory + "/claid/dispatch/python/module")

from claid.CLAID import CLAID