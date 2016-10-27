#!/bin/bash

compiler=${1}
lexer=${2}
in_file=${3}

compiler_out=$("${compiler}" --lextest "${in_file}")
lexer_out=$(${lexer} "${in_file}")

echo "${compiler}"

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

exit 0
