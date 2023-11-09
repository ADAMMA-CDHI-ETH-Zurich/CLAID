#!/bin/bash

set -x

# TODO: remove this file once
#     bazel test :all
# works reliably.


# Working tests
CC=clang bazel test  --test_timeout=10 --cache_test_results=no :capi_test
CC=clang bazel test  --test_timeout=10 --cache_test_results=no :module_table_test
CC=clang bazel test  --test_timeout=10 --cache_test_results=no :router_test
CC=clang bazel test  --test_timeout=10 --cache_test_results=no :routing_tree_test
CC=clang bazel test  --test_timeout=10 --cache_test_results=no :mutator_test
CC=clang bazel test  --test_timeout=10 --cache_test_results=no :remote_dispatching_test

# Workin but fails at the end because the shutdown doesn't work.
CC=clang bazel test --test_output=all --test_timeout=10 --cache_test_results=no :local_dispatching_test
