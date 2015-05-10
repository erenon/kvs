#!/bin/bash

set -e

bin_dir="$( cd "$( dirname "$0" )" && pwd )"
CXX=${CXX:="g++"}
test_dir="$bin_dir/build/$CXX/release/test"

find $test_dir -type f -not -name "*.so" -print0 | while IFS= read -r -d $'\0' testprog; do
  echo "run $testprog"
  $testprog
done
