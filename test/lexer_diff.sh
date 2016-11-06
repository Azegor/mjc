#!/bin/bash

compiler=${1}
lexer=${2}
in_file=${3}

compiler_out=$("${compiler}" --lextest "${in_file}" 2>&1)
compiler_retval=$?
lexer_out=$(${lexer} "${in_file}")
lexer_retval=$?


# if the return code not 0, "error" must be in the output
if [[ ${compiler_retval} -ne 0 ]]; then
  err=false
  if [[ ${compiler_out} != *"error"* ]]; then
    echo "ERROR: Compiler return value is != 0 but output doesn't contain 'error'"
    echo "Output:"
    echo "${compiler_out}"
    err=true
  fi

  if [[ ${lexer_retval} -eq 0 ]]; then
    echo "ERROR: Compiler returned ${compiler_retval} but Lexer returned ${lexer_retval}"
    err=true
  fi

  if "$err" = true; then
    exit 1
  fi
fi

if [[ ${lexer_retval} -ne 0 ]]; then
  err=false
  if [[ ${lexer_out} != *"error"* ]]; then
    echo "ERROR: Lexer return value is != 0 but output doesn't contain 'error'"
    echo "Output:"
    echo "${lexer_out}"
    err=true
  fi

  if [[ ${compiler_retval} -eq 0 ]]; then
    echo "ERROR: Lexer returned ${lexer_retval} but Compiler ${compiler_retval}"
    err=true
  fi

  if "$err" = true; then
    exit 1;
  fi
fi

# Strip profiling file output from first line if present.
# e.g.  "LLVM Profile Note: Set profile file path to "default.profraw" via default setting." 
compiler_out=${compiler_out#LLVM Profile Note*.
}

# Only compare outputs if both return values were 0
if [ "${compiler_retval}" -eq 0 ] && [ "${lexer_retval}" -eq 0 ]; then
  if [[ "${compiler_out}" != "${lexer_out}" ]]; then
      diff=$(diff <(echo "${compiler_out}") <(echo "${lexer_out}"))
      echo "================================="
      echo "Compiler Output:"
      echo "${compiler_out}"
      echo "================================="
      echo "Lexer Output:"
      echo "${lexer_out}"
      echo "================================="
      echo "Diff:"
      echo "${diff}"
      exit 1
  fi
fi

exit 0
