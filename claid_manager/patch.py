


import shutil
import os

def recursive_overwrite(dst, src, ignore=None):
    for src_dir, dirs, files in os.walk(src):
        dst_dir = src_dir.replace(src, dst, 1)
    if not os.path.exists(dst_dir):
        os.makedirs(dst_dir)
    for file_ in files:
        src_file = os.path.join(src_dir, file_)
        dst_file = os.path.join(dst_dir, file_)
        if os.path.exists(dst_file):
            try:
                os.remove(dst_file)
            except PermissionError as exc:
                continue

        shutil.copy(src_file, dst_dir)




def patch(*args):

    if(len(args) != 2):
        print("Error in claid patch: Unsupported number of arguments")
        print("Usage: claid patch project_directory patch_directory")
        exit(0)

    
    dst = args[0]
    patch_srcs = args[1]

    if not os.path.isdir(dst):
        print("Error in claid patch: Destination directory \"{}\" does not exit".format(dst))
        exit(0)

    if not os.path.isdir(patch_srcs):
        print("Error in claid patch: Patch directory \"{}\" does not exit".format(patch_srcs))
        exit(0)

    recursive_overwrite(dst, patch_srcs)


    