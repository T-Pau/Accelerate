if(NOT HAVE_GETOPT_LONG)
    list(APPEND COMPATIBILITY getopt_long.cc)
endif()


SET(assembler_SOURCES
        AddressingMode.cc
        Assembler.cc
        Commandline.cc
        CPU.cc
        CPUParser.cc
        Exception.cc
        ExpressionListNode.cc
        ExpressionNode.cc
        FileReader.cc
        Instruction.cc
        IntegerNode.cc
        Node.cc
        SymbolTable.cc
        Tokenizer.cc
        Util.cc
        ${COMPATIBILITY}
)

foreach(PROGRAM assembler)
    ADD_EXECUTABLE(${PROGRAM} ${${PROGRAM}_SOURCES})
    target_include_directories(${PROGRAM} PRIVATE ${PROJECT_BINARY_DIR})
    INSTALL(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
