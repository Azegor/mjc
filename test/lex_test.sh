#!/bin/bash

# tests that <compiler> --lextest $2 outputs the contents of $3

compiler=${1}
java_file=${2}
lexer_output=${3}

compiler_out=$("${compiler}" --lextest "${java_file}")
cat_out=$(cat "${lexer_output}")

if [[ "${compiler_out}" != "${cat_out}" ]]; then
    echo "================================="
    echo "Compiler Output:"
    echo "${compiler_out}"
    echo "================================="
    echo "cat Output:"
    echo "${cat_out}"
    exit 1
fi

exit 0
