#!/bin/bash

compiler=${1}
in_file=${2}

out_name=$(mktemp --tmpdir=. -u)

compiler_out=$("${compiler}" --compile-firm -O "${in_file}" -o $out_name 2>&1)
compiler_retval=$?




# if the return code not 0, "error" must be in the output
if [[ ${compiler_retval} -ne 0 ]]; then
  echo "ERROR: Compiler returned ${compiler_retval}"

  echo "Output:"
  echo "${compiler_out}"

  rm -f $out_name

  exit 1
fi
a_out=$($out_name)

rm -f $out_name

if [[ $? -ne 0 ]]; then
  echo "ERROR: program crashed:"
  echo $a_out
  exit 1
fi

unopt_out_name=$(mktemp --tmpdir=. -u)

noopt_compiler_out=$("${compiler}" --compile-firm "${in_file}" -o $unopt_out_name 2>&1)
noopt_compiler_retval=$?
if [[ ${compiler_retval} -ne 0 ]]; then
  echo "ERROR: Compiler (unoptimized) returned ${compiler_retval}"
  rm -f $unopt_out_name
  exit 1
fi

unopt_a_out=$($unopt_out_name)

rm -f $unopt_out_name

if [[ $? -ne 0 ]]; then
  echo "ERROR: program (unopt) crashed:"
  echo $unopt_a_out
  exit 1
fi

if [[ ${a_out} != ${unopt_a_out} ]]; then
  echo "ERROR: Program output's differ!"
  echo $(diff <(echo ${a_out}) <(echo ${unopt_a_out}))
  exit 1
fi

exit 0
