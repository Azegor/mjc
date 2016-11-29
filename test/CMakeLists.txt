MESSAGE(STATUS "Adding tests")

# http://stackoverflow.com/questions/16158091/cmake-setting-an-environmental-variable-for-ctest-or-otherwise-getting-failed
if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check
        ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process --quiet --parallel $(JOBS)
            --build-config "$<CONFIGURATION>"
        && printf "\\033[1m\\033[32m::: All tests passed!\\033[39m\\033[0m\\n"
        || ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process --rerun-failed --output-on-failure
            --build-config "$<CONFIGURATION>"
        VERBATIM
    )
else()
    add_custom_target(check
        ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process --quiet --parallel $(JOBS)
        && printf "\\033[1m\\033[32m::: All tests passed!\\033[39m\\033[0m\\n"
        || ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process --rerun-failed --output-on-failure --parallel $(JOBS)
        VERBATIM
    )
endif()

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

# check valid programs semantically
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/sematest/valid/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Parser test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Semantics_valid_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/semantic_valid.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} valid semantic tests")

# check invalid programs semantically
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/sematest/invalid/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Parser test file: ${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Semantics_invalid_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/semantic_invalid.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} invalid semantic tests")

# check valid firm programs
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/firmtest/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Firm_${filename}"
           COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/firm_valid.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} libfirm tests")

# libfirm codegen + output tests
set(Count 0)
file(GLOB lexer_input_files "${CMAKE_CURRENT_SOURCE_DIR}/firmtest/output/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Firm_Output_${filename}"
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/firm_output.sh" $<TARGET_FILE:mjc> "${CMAKE_CURRENT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/src/" "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} libfirm output tests")

set(Count 0)
file(GLOB lexer_input_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/exec/*.java")
file(GLOB_RECURSE stdlib_files "${CMAKE_CURRENT_SOURCE_DIR}/testlib/*.java")
foreach(file ${lexer_input_files})
  math(EXPR Count "${Count} + 1")
#   MESSAGE(STATUS "Parser test file: ${file}")
get_filename_component(dirname "${file}" PATH)
  add_custom_command(
    PRE_BUILD
    COMMAND "mkdir"
    ARGS "-p" "${dirname}"
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/preprocess.sh"
    ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${file}" "${CMAKE_CURRENT_SOURCE_DIR}/testlib" "${file}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
    DEPENDS ${stdlib_files}
    OUTPUT "${file}"
  )
  # HACK: can't add dependencies to tests, sp just always preprocess them in `make all`
  add_custom_target("generated_${Count}" ALL DEPENDS "${file}")
  get_filename_component(filename "${file}" NAME)
  add_test(NAME "Semantics_exec_valid_${filename}"
      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/semantic_valid.sh" $<TARGET_FILE:mjc> "${file}")
endforeach()
MESSAGE(STATUS "  Added ${Count} executable tests")

