#!/bin/bash

compiler=${1}
in_file=${2}

input_file="${2}.in"
output_file="${2}.out"




out_name=$(mktemp --tmpdir=. -u)

compiler_out=$("${compiler}" "${in_file}" -o $out_name 2>&1)
compiler_retval=$?


#echo $out_name
#echo $input_file
#echo $output_file

if [[ ${compiler_retval} -ne 0 ]]; then
  echo "ERROR: Compiler returned ${compiler_retval}"

  echo "Output:"
  echo "${compiler_out}"

  rm -f $out_name

  exit 1
fi


if [[ -a ${input_file} ]]; then
  a_out=$(cat $input_file | $out_name)
else
  a_out=$($out_name)
fi

rm -f $out_name

if [[ $? -ne 0 ]]; then
  echo "ERROR: program crashed:"
  echo $a_out
  exit 1
fi

if [[ -a ${output_file} ]]; then
  wanted_output=$(cat "${output_file}")
  if [[ ${a_out} != ${wanted_output} ]]; then
    echo ${wanted_output}
    echo "ERROR: Program output not wanted output!"
    echo $(diff <(echo ${a_out}) <(echo ${wanted}))
    exit 1
  fi
fi

exit 0
