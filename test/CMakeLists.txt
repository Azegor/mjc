MESSAGE(STATUS "Adding tests")

add_test(NAME echo
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/echo_test.sh" $<TARGET_FILE:mjc>)
add_test(NAME lextest
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/lex_test.sh" $<TARGET_FILE:mjc> "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java" "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java.lex")

# Valid coverage tests
set(Count 0)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/coverage/valid_arguments.txt" invalid_arguments)
foreach(arg ${invalid_arguments})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Coverage valid test command: mjc ${arg}")
  add_test(NAME "Coverage_valid_command_${Count}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/coverage_valid.sh" $<TARGET_FILE:mjc> "${arg}"
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()
MESSAGE(STATUS "  Added ${Count} valid coverage tests")

# Invalid coverage tests
set(Count 0)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/coverage/invalid_arguments.txt" invalid_arguments)
foreach(arg ${invalid_arguments})
  math(EXPR Count "${Count}+1")
#   MESSAGE(STATUS "Coverage invalid test command: mjc ${arg}")
  add_test(NAME "Coverage_invalid_command_${Count}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/coverage_invalid.sh" $<TARGET_FILE:mjc> "${arg}"
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR Count "${Count} + 1")
endforeach()
MESSAGE(STATUS "  Added ${Count} invalid coverage tests")

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

set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/lextest/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Lexer test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Lex_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/lexer_diff.sh" $<TARGET_FILE:mjc> $<TARGET_FILE:mj_lexer> "${file}"
           DEPENDS mj_lexer)
endforeach()
MESSAGE(STATUS "  Added ${Count} lexer tests")

# parse valid programs
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/parsetest/valid/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Parser test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Parse_valid_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/parser_valid.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} valid parser tests")

#generated programs
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/parsetest/generated/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Parser test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Parse_valid_generated_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/parser_valid.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} generated valid parser tests")

# parse invalid programs
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/parsetest/invalid/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Parser test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Parse_invalid_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/parser_invalid.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} invalid parser tests")

# ast idempotency test
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/parsetest/valid/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Parser test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "AST_idempotent_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/ast_idempotency.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} ast idempotency tests")
