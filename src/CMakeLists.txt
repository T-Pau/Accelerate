if(NOT HAVE_GETOPT_LONG)
    list(APPEND COMPATIBILITY getopt_long.cc)
endif()

add_library(xlr8-library STATIC
        Address.cc
        AddressingMode.cc
        AddressingModeMatcher.cc
        ArgumentType.cc
        Assembler.cc
        AssignmentBody.cc
        BaseExpression.cc
        BinaryExpression.cc
        BlockBody.cc
        Body.cc
        BodyElement.cc
        BodyParser.cc
        CPU.cc
        CPUGetter.cc
        CPUParser.cc
        Callable.cc
        Command.cc
        Commandline.cc
        DataBody.cc
        EmptyBody.cc
        Encoding.cc
        Entity.cc
        Environment.cc
        ErrorBody.cc
        EvaluationContext.cc
        EvaluationResult.cc
        Exception.cc
        Expression.cc
        ExpressionNode.cc
        ExpressionParser.cc
        FileParser.cc
        FileReader.cc
        FileTokenizer.cc
        Function.cc
        FunctionExpression.cc
        IfBody.cc
        InRangeExpression.cc
        Instruction.cc
        InstructionEncoder.cc
        Int.cc
        Label.cc
        LabelBody.cc
        LabelExpression.cc
        LibraryGetter.cc
        Linker.cc
        Location.cc
        Macro.cc
        MacroBody.cc
        Memory.cc
        MemoryBody.cc
        MemoryMap.cc
        Node.cc
        Object.cc
        ObjectExpression.cc
        ObjectFile.cc
        ObjectFileParser.cc
        ObjectNameExpression.cc
        ParseException.cc
        ParsedValue.cc
        Path.cc
        Range.cc
        SequenceTokenizer.cc
        SizeRange.cc
        Symbol.cc
        Target.cc
        TargetGetter.cc
        TargetParser.cc
        Token.cc
        TokenGroup.cc
        TokenNode.cc
        Tokenizer.cc
        UnaryExpression.cc
        Util.cc
        Value.cc
        ValueExpression.cc
        VariableExpression.cc
        Visibility.cc
        VoidExpression.cc
        ${COMPATIBILITY}
)

target_include_directories(xlr8-library PRIVATE ${PROJECT_BINARY_DIR})

foreach(PROGRAM xlr8)
    ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.cc)
    target_include_directories(${PROGRAM} PRIVATE ${PROJECT_BINARY_DIR})
    target_link_libraries(${PROGRAM} xlr8-library)
    INSTALL(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
