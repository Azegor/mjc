#!/bin/bash

compiler=${1}
binary_dir=${2}
src_dir=${3}
in_file=${4}

echo "Binary dir: ${binary_dir}"
echo "src dir: ${src_dir}"
echo "Compiler: ${compiler}"
echo "In file: ${in_file}"

name=${in_file##*/}
out_dir=${binary_dir}/firm_output_${name}
mkdir "${out_dir}"
pushd "${out_dir}" >/dev/null

# This will just dump a file called "test.s"
compiler_out=$("${compiler}" --firm-graph --gen-code "${in_file}" 2>&1)
compiler_retval=$?

gcc -static test.s -o _test_ -L"${binary_dir}/../" -lruntime

popd >/dev/null

if [[ ${compiler_retval} -ne 0 ]]; then
  echo "ERROR: Compiler returned ${compiler_retval} but ${in_file} is supposed to be valid"
  echo "Output:"
  echo "${compiler_out}"

  exit 1
fi

wanted_output=$(cat "${in_file}.output")
actual_output=$("${out_dir}/_test_")

echo "wanted output: ${wanted_output}"
echo "actual output: ${actual_output}"

if [[ "${actual_output}" = "" ]]; then
  echo "ERROR: Output is empty"

  exit 1
fi

if [[ "${actual_output}" != "${wanted_output}" ]]; then
  echo "ERROR: Wanted and actual output do not match".

  echo "Wanted: '${wanted_output}'"
  echo "Actual: '${actual_output}'"

  exit 1
fi

exit 0
