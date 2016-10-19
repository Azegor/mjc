add_test(NAME echo
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/echo_test.sh" $<TARGET_FILE:mjc>)
