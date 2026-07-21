# cspell: words BINDIR endforeach

add_library(xlr8-library STATIC
        Body/AssignmentBody.cc
        Body/BlockBody.cc
        Body/Body.cc
        Body/BodyElement.cc
        Body/ChecksumBody.cc
        Body/DataBody.cc
        Body/EmptyBody.cc
        Body/ErrorBody.cc
        Body/IfBody.cc
        Body/LabelBody.cc
        Body/MacroBody.cc
        Body/MemoryBody.cc
        Body/RepeatBody.cc
        Body/ScopeBody.cc

        Entity/Callable.cc
        Entity/Constant.cc
        Entity/Entity.cc
        Entity/Function.cc
        Entity/Macro.cc
        Entity/Object.cc
        Entity/Output.cc

        Expression/ArgumentExpression.cc
        Expression/BaseExpression.cc
        Expression/BinaryExpression.cc
        Expression/ConstantExpression.cc
        Expression/CurrentObjectExpression.cc
        Expression/DefinedExpression.cc
        Expression/EntityExpression.cc
        Expression/ExistsExpression.cc
        Expression/Expression.cc
        Expression/FillExpression.cc
        Expression/FunctionExpression.cc
        Expression/InRangeExpression.cc
        Expression/LabelOffsetExpression.cc
        Expression/MinMaxExpression.cc
        Expression/ObjectExpression.cc
        Expression/ObjectNameExpression.cc
        Expression/SizeofExpression.cc
        Expression/UnaryExpression.cc
        Expression/ValueExpression.cc
        Expression/VariableExpression.cc
        Expression/VoidExpression.cc

        Address.cc
        AddressingMode.cc
        AddressingModeMatcher.cc
        ArgumentType.cc
        Assembler.cc
        Base.cc
        BaseEncoder.cc
        BinaryEncoder.cc
        BodyParser.cc
        Checksum.cc
        ChecksumAlgorithm.cc
        ChecksumAlgorithmXor.cc
        ChecksumComputation.cc
        CPU.cc
        CPUGetter.cc
        CPUParser.cc
        Encoder.cc
        EvaluationContext.cc
        EvaluationOrder.cc
        EvaluationResult.cc
        ExpressionNode.cc
        ExpressionParser.cc
        FileParser.cc
        FileTokenizer.cc
        HexStreamEncoder.cc
        HexStringDecoder.cc
        Instruction.cc
        InstructionInvocation.cc
        IntegerEncoder.cc
        LibraryGetter.cc
        LibraryLinker.cc
        Linker.cc
        Memory.cc
        MemoryMap.cc
        Module.cc
        Node.cc
        ObjectFile.cc
        ObjectFileParser.cc
        ProgramLinker.cc
        Range.cc
        Scope.cc
        SequenceTokenizer.cc
        SizeRange.cc
        StringEncoder.cc
        StringEncoding.cc
        StructuredValue.cc
        Target.cc
        TargetGetter.cc
        Token.cc
        TokenGroup.cc
        Tokenizer.cc
        TokenNode.cc
        UnnamedLabelList.cc
        Unresolved.cc
        UsedEntities.cc
        Visibility.cc
)

target_include_directories(xlr8-library PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(xlr8-library tpau-cpp-kernal::tpau-cpp-kernal)

foreach(PROGRAM xlr8)
    ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.cc)
    target_include_directories(${PROGRAM} PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${PROGRAM} xlr8-library)
    target_link_libraries(${PROGRAM} tpau-cpp-kernal::tpau-cpp-kernal)
    INSTALL(TARGETS ${PROGRAM} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
