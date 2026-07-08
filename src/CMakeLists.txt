# cspell: words BINDIR endforeach

add_library(xlr8-library STATIC
        Address.cc
        AddressingMode.cc
        AddressingModeMatcher.cc
        ArgumentType.cc
        Assembler.cc
        AssignmentBody.cc
        BaseEncoder.cc
        BaseExpression.cc
        BinaryEncoder.cc
        BinaryExpression.cc
        BlockBody.cc
        Body.cc
        BodyElement.cc
        BodyParser.cc
        CPU.cc
        CPUGetter.cc
        CPUParser.cc
        Callable.cc
        Checksum.cc
        ChecksumBody.cc
        ChecksumAlgorithm.cc
        ChecksumAlgorithmXor.cc
        ChecksumComputation.cc
        DataBody.cc
        DefinedExpression.cc
        EmptyBody.cc
        Encoder.cc
        Entity.cc
        Environment.cc
        ErrorBody.cc
        EvaluationContext.cc
        EvaluationResult.cc
        ExistsExpression.cc
        Expression.cc
        ExpressionNode.cc
        ExpressionParser.cc
        FileParser.cc
        FileTokenizer.cc
        FillExpression.cc
        Function.cc
        FunctionExpression.cc
        HexStreamEncoder.cc
        HexStringDecoder.cc
        IfBody.cc
        InRangeExpression.cc
        Instruction.cc
        InstructionEncoder.cc
        IntegerEncoder.cc
        LabelBody.cc
        LabelExpression.cc
        LibraryGetter.cc
        Linker.cc
        Macro.cc
        MacroBody.cc
        Memory.cc
        MemoryBody.cc
        MemoryMap.cc
        MinMaxExpression.cc
        Node.cc
        Object.cc
        ObjectExpression.cc
        ObjectFile.cc
        ObjectFileParser.cc
        ObjectNameExpression.cc
        StructuredValue.cc
        Range.cc
        RepeatBody.cc
        ScopeBody.cc
        SequenceTokenizer.cc
        SizeofExpression.cc
        SizeRange.cc
        StringEncoder.cc
        StringEncoding.cc
        Target.cc
        TargetGetter.cc
        Token.cc
        TokenGroup.cc
        TokenNode.cc
        Tokenizer.cc
        UnaryExpression.cc
        UnnamedLabelList.cc
        Unresolved.cc
        ValueExpression.cc
        VariableExpression.cc
        Visibility.cc
        VoidExpression.cc
        Output.cc
        ProgramLinker.cc
        LibraryLinker.cc
        UsedEntities.cc
)

target_include_directories(xlr8-library PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(xlr8-library tpau-cpp-kernal::tpau-cpp-kernal)

foreach(PROGRAM xlr8)
    ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.cc)
    target_include_directories(${PROGRAM} PRIVATE ${PROJECT_BINARY_DIR})
    target_link_libraries(${PROGRAM} xlr8-library)
    target_link_libraries(${PROGRAM} tpau-cpp-kernal::tpau-cpp-kernal)
    INSTALL(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
