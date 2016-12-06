#!/bin/bash

compiler=${1}
in_file=${2}

compiler_out=$("${compiler}" --compile-firm -O "${in_file}" 2>&1)
compiler_retval=$?


# if the return code not 0, "error" must be in the output
if [[ ${compiler_retval} -ne 0 ]]; then
  echo "ERROR: Compiler returned ${compiler_retval}"

  echo "Output:"
  echo "${compiler_out}"

  exit 1
fi

exit 0
