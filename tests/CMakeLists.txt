# cspell: words XFAIL endforeach

set(XFAIL_TESTS
    # Won't fix before merge
    cpu-4510.test # long branches need new argument restriction type
    cpu-45gs02.test # long branches need new argument restriction type
    long-branch-short.test # long branches need new argument restriction type

    # Will fix before merge
    hello-world-zx-spectrum-next.test # function call
    # hello-world-zx-spectrum-tap.test # function call, checksum

    error-unclosed-object.test
)

file(GLOB TESTS ${CMAKE_CURRENT_SOURCE_DIR}/*.test)
foreach(FULL_CASE IN LISTS TESTS)
    get_filename_component(CASE ${FULL_CASE} NAME)
    add_test(NAME ${CASE} COMMAND nihtest ${CASE})
    set_tests_properties(${CASE} PROPERTIES SKIP_RETURN_CODE 77)
    #  set_tests_properties(${CASE} PROPERTIES ENVIRONMENT "PATH=${path}")
endforeach()

foreach(CASE ${XFAIL_TESTS})
    set_tests_properties(${CASE} PROPERTIES WILL_FAIL TRUE)
endforeach()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/nihtest.conf.in ${CMAKE_CURRENT_BINARY_DIR}/nihtest.conf)
