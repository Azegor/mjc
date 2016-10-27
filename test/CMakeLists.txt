add_test(NAME echo
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/echo_test.sh" $<TARGET_FILE:mjc>)
add_test(NAME lextest
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/lex_test.sh" $<TARGET_FILE:mjc> "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java" "${CMAKE_CURRENT_SOURCE_DIR}/Prog1.java.lex")
