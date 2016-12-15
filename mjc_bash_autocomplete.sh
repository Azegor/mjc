# _mjc()
# {
#   _mjc_commands=$(./mjc --help)
#
#   local cur prev
#   COMPREPLY=()
#   cur="${COMP_WORDS[COMP_CWORD]}"
#   COMPREPLY=( $(compgen -W "${_script_commands}" -- ${cur}) )
#
#   return 0
# }
# complete -o nospace -F _mjc ./mjc

complete_words='-h --help --echo --input-file --lextest --lexfuzz --parsetest --parsefuzz --print-ast --dot-ast --check --fuzz-check --dot-attr-ast --firm-graph --compile-firm -S --output-assembly --no-verify -O --optimize -c --compile -o --output'
complete -o nospace -W "${complete_words}" -o bashdefault -o default ./mjc
