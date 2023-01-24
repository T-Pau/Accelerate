if(NOT HAVE_GETOPT_LONG)
    list(APPEND COMPATIBILITY getopt_long.cc)
endif()


SET(xlr8-as_SOURCES
        AddressingMode.cc
        AddressingModeMatcher.cc
        ArgumentType.cc
        Assembler.cc
        AssemblerMain.cc
        AssemblerObject.cc
        Commandline.cc
        CPU.cc
        CPUParser.cc
        Environment.cc
        Exception.cc
        ExpressionListNode.cc
        ExpressionNode.cc
        ExpressionParser.cc
        FileReader.cc
        Instruction.cc
        TokenNode.cc
        Location.cc
        Node.cc
        Object.cc
        ParseException.cc
        Path.cc
        MemoryMap.cc
        Section.cc
        Symbol.cc
        SymbolTable.cc
        Token.cc
        TokenGroup.cc
        Tokenizer.cc
        TokenizerFile.cc
        TokenizerSequence.cc
        Util.cc
        ${COMPATIBILITY}
)

foreach(PROGRAM xlr8-as)
    ADD_EXECUTABLE(${PROGRAM} ${${PROGRAM}_SOURCES})
    target_include_directories(${PROGRAM} PRIVATE ${PROJECT_BINARY_DIR})
    INSTALL(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
