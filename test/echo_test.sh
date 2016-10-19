#!/bin/bash

# tests that <compiler> --echo <file> outputs the contents of <file>
# uses itself as the <file>

compiler=${1}

compiler_out=$("${compiler}" --echo "${0}")
cat_out=$(cat "${0}")

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
