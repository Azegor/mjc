#!/bin/bash

compiler=${1}
shift
arguments=$@

compiler_out=$(${compiler} ${arguments} 2>&1)
compiler_retval=$?


if [[ "${compiler_retval}" -eq 0 ]]; then
  echo "ERROR: Compiler returned ${compiler_retval} but '${compiler} ${arguments}' is supposed to be invalid"
  exit 1
fi

exit 0
