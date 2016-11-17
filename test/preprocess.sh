#!/bin/bash


die() {
    echo "${@}" >&2
    exit 1
}

_get_imported_files() {
    local in_file=${1}
    local libdir=${2}

    [[ -f ${in_file} ]] || die "file not found: ${in_file}"

    local imp
    for imp in $(grep -o '^import .*;' "${in_file}" | sed -e 's#import \(.*\);#\1#g'); do
        local impfile=${imp//.//}.java
        [[ -f ${libdir}/${impfile} ]] || die "import file not found for ${imp}"
        echo "${libdir}/${impfile}"
    done
}

_get_all_imports() {
    local in_file=${1}
    local libdir=${2}

    local -A imports
    imports["${in_file}"]=0

    while true; do
        local processed=0
        local imp
        for imp in "${!imports[@]}"; do
            # file already processed
            [[ ${imports[${imp}]} -gt 0 ]] && continue
           
            local file
            for file in $(_get_imported_files "${imp}" "${libdir}"); do
                imports["${file}"]=$((imports["${file}"]+0))
            done

            processed=$((processed+1))
            imports["${imp}"]=$((imports["${imp}"]+1))
        done

        if [[ ${processed} -eq 0 ]]; then
            echo "${!imports[@]}"
            return
        fi
    done
}

_rewrite_to_mj() {
    in_file=${1}

    sed -e '/^package .*;/ d' \
        -e '/^import .*;/ d' \
        -e 's#public class#class#g' \
        "${in_file}"
}

_main() {
    local in_file=${1}
    local libdir=${2}
    local out_file=${3}

    [[ -f ${in_file} ]] || die "input file not found"
    [[ -d ${libdir} ]] || die "lib dir doesn't exist"
    rm -f "${out_file}" || die "couldn't remove output file"

    local file
    for file in $(_get_all_imports "${in_file}" "${libdir}"); do
        _rewrite_to_mj "${file}" >> "${out_file}"
        echo '' >> "${out_file}"
    done
}

_main "${@}"
