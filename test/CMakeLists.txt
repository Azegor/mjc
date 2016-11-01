add_test(NAME echo
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/echo_test.sh" $<TARGET_FILE:mjc>)
add_test(NAME lextest
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/lex_test.sh" $<TARGET_FILE:mjc> "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java" "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java.lex")



# Compile lexer using flex
if(NOT EXISTS "/usr/bin/flex")
  message(FATAL_ERROR "flex not found")
endif()

SET_SOURCE_FILES_PROPERTIES(minijava.c GENERATED)
add_executable(mj_lexer minijava.c)
target_link_libraries(mj_lexer fl)
ADD_CUSTOM_COMMAND(
  COMMAND flex
  ARGS -ominijava.c
       ${CMAKE_CURRENT_SOURCE_DIR}/minijava.l
  DEPENDS minijava.l
  OUTPUT minijava.c)

file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/lextest/*.java")
foreach(file ${lexer_input_files})
  MESSAGE(STATUS "Lexer test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Lex_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/lexer_diff.sh" $<TARGET_FILE:mjc> $<TARGET_FILE:mj_lexer> "${file}"
           DEPENDS mj_lexer)
endforeach()
