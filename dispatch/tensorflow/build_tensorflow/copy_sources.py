import os
import shutil

def copy_files_with_extensions(source_folder, destination_folder, allowed_extensions):
    for root, dirs, files in os.walk(source_folder):
        for file in files:
            if any(file.endswith(ext) for ext in allowed_extensions):
                source_path = os.path.join(root, file)
                destination_path = os.path.join(destination_folder, os.path.relpath(source_path, source_folder))
                
                # Ensure the destination directory exists
                os.makedirs(os.path.dirname(destination_path), exist_ok=True)
                
                # Copy the file
                print(source_path, destination_path)
                shutil.copy2(source_path, destination_path)

# Example usage:
source_folder = '../../../../claid_external/tensorflow'
destination_folder = 'tfinc'
allowed_extensions = [".h", ".c", ".hh", ".hpp"]

copy_files_with_extensions(source_folder, destination_folder, allowed_extensions)
