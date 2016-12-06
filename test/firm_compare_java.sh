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
mkdir -p "${out_dir}"
pushd "${out_dir}" >/dev/null

# This will just dump a file called "test.s"
compiler_out=$("${compiler}" --compile-firm "${in_file}" 2>&1)
compiler_retval=$?

# gcc -static test.s -o a.out -L"${binary_dir}/../" -lruntime

popd >/dev/null

if [[ ${compiler_retval} -ne 0 ]]; then
  echo "ERROR: Compiler returned ${compiler_retval} but ${in_file} is supposed to be valid"
  echo "Output:"
  echo "${compiler_out}"

  exit 1
fi

filename=$(basename "${in_file}")
class_name="${filename%.*}"
class_path=$(dirname "${in_file}")
javac ${in_file}
if [[ $? -ne 0 ]]; then
    echo "Error while compiling with javac"
    exit 1
fi
wanted_output=$(java -cp ${class_path} ${class_name})
java_retval=$?
rm -f "${class_path}/${class_name}"
if [[ ${java_retval} -ne 0 ]]; then
    echo "Error while executing with"
    exit 1
fi


actual_output=$("${out_dir}/a.out")
if [[ $? -ne 0 ]]; then
    echo "Error while executing binary"
    exit 1
fi

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
