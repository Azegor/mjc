#!/bin/bash

compiler=${1}
in_file=${2}

compiler_out1=$("${compiler}" --print-ast "${in_file}" 2>&1)
compiler_retval1=$?
compiler_out2=$("${compiler}" --print-ast <(echo "${compiler_out1}") 2>&1)
compiler_retval2=$?


# if the return code not 0, "error" must be in the output
if [[ ${compiler_retval1} -ne 0 ]]; then

  if [[ ${compiler_out1} != *"error"* ]]; then
    echo "ERROR: Compiler (pass 1) return value is != 0 but output doesn't contain 'error'"
    echo "Output:"
    echo "${compiler_out1}"
  fi

  echo "ERROR: Compiler (pass 1) returned ${compiler_retval1} but ${in_file} is supposed to be valid"

  exit 1
fi
if [[ ${compiler_retval2} -ne 0 ]]; then

  if [[ ${compiler_out2} != *"error"* ]]; then
    echo "ERROR: Compiler (pass 2) return value is != 0 but output doesn't contain 'error'"
    echo "Output:"
    echo "${compiler_out2}"
  fi

  echo "ERROR: Compiler (pass 2) returned ${compiler_retval2} but output of first run is supposed to be valid"
  echo "Output:"
  echo "${compiler_out2}"
  echo "Output of first pass was:"
  echo "========================"
  echo "${compiler_out1}"
  echo "========================"

  exit 1
fi

if [[ "${compiler_out1}" != "${compiler_out2}" ]]; then
  diff=$(diff <(echo "${compiler_out1}") <(echo "${compiler_out2}"))
  echo "ERROR: Output of first and second pass of the Compiler are not identical"
  echo "First Pass:"
  echo "================================="
  echo "${compiler_out1}"
  echo "================================="
  echo
  echo "Second Pass:"
  echo "================================="
  echo "${compiler_out2}"
  echo "================================="
  echo "Diff:"
  echo "================================="
  echo "${diff}"
  echo "================================="
  echo "Return values: pass1=${compiler_retval1}, pass2=${compiler_retval2}"
  exit 1
fi

exit 0
