if(NOT HAVE_GETOPT_LONG)
    list(APPEND COMPATIBILITY getopt_long.cc)
endif()


SET(assembler_SOURCES
        Assembler.cc
        Commandline.cc
        Exception.cc
        FileReader.cc
        Token.cc
        Tokenizer.cc
        Util.cc
        ${COMPATIBILITY}
)

foreach(PROGRAM assembler)
    ADD_EXECUTABLE(${PROGRAM} ${${PROGRAM}_SOURCES})
    target_include_directories(${PROGRAM} PRIVATE ${PROJECT_BINARY_DIR})
    INSTALL(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
