if(NOT HAVE_GETOPT_LONG)
    list(APPEND COMPATIBILITY getopt_long.cc)
endif()


SET(xlr8-as_SOURCES
        AddressingMode.cc
        AddressingModeMatcher.cc
        ArgumentType.cc
        Assembler.cc
        AssemblerMain.cc
        CPU.cc
        CPUParser.cc
        Commandline.cc
        Environment.cc
        Exception.cc
        ExpressionList.cc
        ExpressionNode.cc
        ExpressionParser.cc
        FileParser.cc
        FileReader.cc
        Instruction.cc
        Int.cc
        Location.cc
        MemoryMap.cc
        Node.cc
        Object.cc
        ObjectFile.cc
        ObjectFileParser.cc
        ParseException.cc
        ParsedValue.cc
        Path.cc
        Section.cc
        SymbolTable.cc
        Token.cc
        TokenGroup.cc
        TokenNode.cc
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
