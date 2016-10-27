add_test(NAME echo
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/echo_test.sh" $<TARGET_FILE:mjc>)
add_test(NAME lextest
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/lex_test.sh" $<TARGET_FILE:mjc> "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java" "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java.lex")



# Compile lexer using flex
SET_SOURCE_FILES_PROPERTIES(minijava.c GENERATED)
add_executable(mj_lexer minijava.c)
target_link_libraries(mj_lexer fl)
ADD_CUSTOM_COMMAND(
  COMMAND flex
  ARGS -ominijava.c
       ${CMAKE_CURRENT_SOURCE_DIR}/minijava.l
  OUTPUT minijava.c)
