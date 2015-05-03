#!/bin/bash

set -e

bin_dir="$( cd "$( dirname "$0" )" && pwd )"
CXX=${CXX:="g++"}
test_dir="$bin_dir/build/$CXX/release/test"

find $test_dir -type f -print0 | while IFS= read -r -d $'\0' testprog; do
  $testprog
done
