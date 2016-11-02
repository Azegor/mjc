#!/bin/bash

compiler=${1}
in_file=${2}

compiler_out=$("${compiler}" --parsetest "${in_file}" 2>&1)
compiler_retval=$?


# if the return code not 0, "error" must be in the output
if [[ ${compiler_retval} -ne 0 ]]; then
  
  if [[ ${compiler_out} != *"error"* ]]; then
    echo "ERROR: Compiler return value is != 0 but output doesn't contain 'error'"
    echo "Output:"
    echo "${compiler_out}"
  fi

  echo "ERROR: Compiler returned ${compiler_retval} but ${in_file} is supposed to be valid"

  exit 1
fi

exit 0
