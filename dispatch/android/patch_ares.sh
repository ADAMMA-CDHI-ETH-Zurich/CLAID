#!/bin/bash

# Define the file to be modified
FILE="../../bazel-CLAID/external/c-ares+/ares_getnameinfo.c"


# Determine the OS and choose the appropriate sed command
if [[ "$(uname)" == "Darwin" ]]; then
    # macOS (BSD sed)
    sed -i '' '/#include "ares_setup.h"/a\
#undef HAVE_GETSERVBYPORT_R
' "$FILE"
else
    # GNU sed on Linux
    sed -i '/#include "ares_setup.h"/a #undef HAVE_GETSERVBYPORT_R' "$FILE"
fi

echo "File '$FILE' has been updated."
