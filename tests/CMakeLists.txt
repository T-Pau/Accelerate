# cspell: words XFAIL endforeach

set(XFAIL_TESTS
    # Won't fix before merge
    cpu-4510.test # long branches need new argument restriction type
    cpu-45gs02.test # long branches need new argument restriction type
    long-branch-short.test # long branches need new argument restriction type

    # Not implemented yet
    hello-world-zx-spectrum-next.test # function call
    library-function.test # function call
    macro-named-label.test # labels in macros
    macro-named-label-fixed.test # labels in macros
    macro-named-label-half-fixed.test # labels in macros
    macro-unnamed-label.test # labels in macros
    macro-unnamed-label-half-fixed.test # labels in macros
    macro-unnamed-label-fixed.test # labels in macros
    macro-nested-branch.test # labels in macros
    hello-world-zx-spectrum-tap.test # labels in macros, function call, checksum

    # Crashes

    # Should investigate
    repeat.test # inner body can't find variable
    hello-world-cerberus-2100-6502.test # labels evaluated wrong
    label-assignment.test # .private label can't be added to entity, needs to be added to module.
    label-named.test # label got lost
    label-unnamed-loop.test # undefined variable
    label-unnamed.test # labels not evaluated
    error-unclosed-object.test # missing location in error message, follow-on error messages
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
