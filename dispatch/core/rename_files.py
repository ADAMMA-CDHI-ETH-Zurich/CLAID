import os

def rename_files(folder_path):
    for foldername, subfolders, filenames in os.walk(folder_path):
        for filename in filenames:
            if filename.endswith(".cpp"):
                old_path = os.path.join(foldername, filename)
                new_filename = filename[:-4] + ".cc"
                new_path = os.path.join(foldername, new_filename)
                
                os.rename(old_path, new_path)
                print(f'Renamed: {old_path} to {new_path}')

# Replace 'your_folder_path' with the path to the root folder containing the files
folder_path = './'
rename_files(folder_path)
