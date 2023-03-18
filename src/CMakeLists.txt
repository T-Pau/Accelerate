if(NOT HAVE_GETOPT_LONG)
    list(APPEND COMPATIBILITY getopt_long.cc)
endif()

add_library(xlr8-library STATIC
        AddressingMode.cc
        AddressingModeMatcher.cc
        ArgumentType.cc
        Assembler.cc
        BinaryExpression.cc
        CPU.cc
        CPUParser.cc
        Command.cc
        Commandline.cc
        Environment.cc
        Exception.cc
        ExpressionList.cc
        Expression.cc
        ExpressionNode.cc
        ExpressionParser.cc
        FileParser.cc
        FileReader.cc
        Instruction.cc
        Int.cc
        IntegerExpression.cc
        Linker.cc
        ObjectExpression.cc
        Location.cc
        Memory.cc
        MemoryMap.cc
        Node.cc
        Object.cc
        ObjectFile.cc
        ObjectFileParser.cc
        OutputElement.cc
        ParseException.cc
        ParsedValue.cc
        Path.cc
        Range.cc
        SymbolTable.cc
        Target.cc
        TargetParser.cc
        Token.cc
        TokenGroup.cc
        TokenNode.cc
        Tokenizer.cc
        TokenizerFile.cc
        TokenizerSequence.cc
        UnaryExpression.cc
        VariableExpression.cc
        Util.cc
        ${COMPATIBILITY}
)

target_include_directories(xlr8-library PRIVATE ${PROJECT_BINARY_DIR})

foreach(PROGRAM xlr8 xlr8-ar)
    ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.cc)
    target_include_directories(${PROGRAM} PRIVATE ${PROJECT_BINARY_DIR})
    target_link_libraries(${PROGRAM} xlr8-library)
    INSTALL(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
