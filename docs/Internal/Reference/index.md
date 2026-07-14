# Internal Reference

<!-- --8<-- [start:summary] -->
This section contains reference documentation for the classes and functions used to implement Accelerate.
<!-- --8<-- [end:summary] -->

It is intended to developers who want to contribute to the project. It provides documentation of the classes and how to use them. For rationale behind their design and how they work together, see the [Internal Explanations](../Explanations/index.md) section.

Private methods and members are not documented here, since they are only relevant when working on that class, which usually require more detail than the documentation provides. They are documented in the source code itself, so IDEs should provide documentation popups for them.

Classes used from T'Pau C++ Kernal are documented in its [reference section](https://tpau-cpp-kernal.tpau.group/Reference/).

The table of contents in the sidebar lists the classes in alphabetical order. 

This index page groups them into categories. Within a category, the base classes are listed first, the others follow in alphabetical order.

The categories are: [CPU Definition](#cpu-definition), [Entities](#entities), [Expressions](#expressions), [Body Elements](#body-elements), [Memory](#memory), [Encoders](#encoders), [Parsers](#parsers), [Component Management](#component-management), [Utility Classes](#utility-classes), and [Unsorted](#unsorted).


## CPU Definition

These classes represent the CPU definition, which defines the syntax of the assembly instructions and how they are encoded. They are created from a CPU definition file and used to parse and encode assembly instructions.

### [CPU](CPU.md)
::: CPU.@brief

### [AddressingMode](AddressingMode.md)
::: AddressingMode.@brief

### [AddressingModeMatcher](AddressingModeMatcher.md)
::: AddressingModeMatcher.@brief

### [ArgumentType](ArgumentType.md)
::: ArgumentType.@brief

### [Instruction](Instruction.md)
::: Instruction.@brief

### [InstructionInvocation](InstructionInvocation.md)
::: InstructionInvocation.@brief


## Entities

These classes represent the components of a program: objects, constants, macros, and functions. They are created from an assembler source file and used to evaluate the program.

### [Entity](Entity.md)
::: Entity.@brief

### [Callable](Callable.md)
::: Callable.@brief

### [Function](Function.md)
::: Function.@brief

### [Macro](Macro.md)
::: Macro.@brief

### [Object](Object.md)
::: Object.@brief


## Expressions

These classes represent expressions, which are used in various places in a program. They are created from an expression and used to evaluate it.

### [Expression](Expression.md)
::: Expression.@brief

### [ExpressionNode](ExpressionNode.md)
::: ExpressionNode.@brief

### [BaseExpression](BaseExpression.md)
::: BaseExpression.@brief

### [BinaryExpression](BinaryExpression.md)
::: BinaryExpression.@brief

### [ConstantExpression](ConstantExpression.md)
::: ConstantExpression.@brief

### [DefinedExpression](DefinedExpression.md)
::: DefinedExpression.@brief

### [ExistsExpression](ExistsExpression.md)
::: ExistsExpression.@brief

### [FillExpression](FillExpression.md)
::: FillExpression.@brief

### [FunctionExpression](FunctionExpression.md)
::: FunctionExpression.@brief

### [InRangeExpression](InRangeExpression.md)
::: InRangeExpression.@brief

### [LabelOffsetExpression](LabelOffsetExpression.md)
::: LabelOffsetExpression.@brief

### [MinMaxExpression](MinMaxExpression.md)
::: MinMaxExpression.@brief

### [ObjectExpression](ObjectExpression.md)
::: ObjectExpression.@brief

### [ObjectNameExpression](ObjectNameExpression.md)
::: ObjectNameExpression.@brief

### [SizeofExpression](SizeofExpression.md)
::: SizeofExpression.@brief

### [UnaryExpression](UnaryExpression.md)
::: UnaryExpression.@brief

### [ValueExpression](ValueExpression.md)
::: ValueExpression.@brief

### [VariableExpression](VariableExpression.md)
::: VariableExpression.@brief

### [VoidExpression](VoidExpression.md)
::: VoidExpression.@brief


## Body Elements

These classes represent body elements, which are used to define the contents of objects and macros. They are created from a body and used to evaluate it.

### [Body](Body.md)
::: Body.@brief

### [BodyElement](BodyElement.md)
::: BodyElement.@brief

### [AssignmentBody](AssignmentBody.md)
::: AssignmentBody.@brief

### [BlockBody](BlockBody.md)
::: BlockBody.@brief

### [ChecksumBody](ChecksumBody.md)
::: ChecksumBody.@brief

### [DataBody](DataBody.md)
::: DataBody.@brief

### [EmptyBody](EmptyBody.md)
::: EmptyBody.@brief

### [ErrorBody](ErrorBody.md)
::: ErrorBody.@brief

### [IfBody](IfBody.md)
::: IfBody.@brief

### [LabelBody](LabelBody.md)
::: LabelBody.@brief

### [MacroBody](MacroBody.md)
::: MacroBody.@brief

### [MemoryBody](MemoryBody.md)
::: MemoryBody.@brief

### [RepeatBody](RepeatBody.md)
::: RepeatBody.@brief

### [ScopeBody](ScopeBody.md)
::: ScopeBody.@brief


## Memory

These classes manage the target machine's memory of the program, including allocation map and memory content.

### [Address](Address.md)
::: Address.@brief

### [Memory](Memory.md)
::: Memory.@brief

### [MemoryMap](MemoryMap.md)
::: MemoryMap.@brief


## Encoders

These classes represent encoders, which are used to encode values into bytes.

### [Encoder](Encoder.md)
::: Encoder.@brief

### [BaseEncoder](BaseEncoder.md)
::: BaseEncoder.@brief

### [BinaryEncoder](BinaryEncoder.md)
::: BinaryEncoder.@brief

### [IntegerEncoder](IntegerEncoder.md)
::: IntegerEncoder.@brief

### [StringEncoder](StringEncoder.md)
::: StringEncoder.@brief


## Parsers

These classes implement the various parsers.

### [Assembler](Assembler.md)
::: Assembler.@brief

### [BodyParser](BodyParser.md)
::: BodyParser.@brief

### [CPUParser](CPUParser.md)
::: CPUParser.@brief

### [FileParser](FileParser.md)
::: FileParser.@brief

### [FileTokenizer](FileTokenizer.md)
::: FileTokenizer.@brief

### [ObjectFileParser](ObjectFileParser.md)
::: ObjectFileParser.@brief

### [SequenceTokenizer](SequenceTokenizer.md)
::: SequenceTokenizer.@brief

### [Token](Token.md)
::: Token.@brief

### [TokenGroup](TokenGroup.md)
::: TokenGroup.@brief

### [TokenNode](TokenNode.md)
::: TokenNode.@brief

### [Tokenizer](Tokenizer.md)
::: Tokenizer.@brief


## Component Management

These classes manage the components of a program, including CPU definitions, targets, and libraries, parsing them from files and avoiding creating duplicates for the same component.

### [Getter](Getter.md)
::: Getter.@brief

### [CPUGetter](CPUGetter.md)
::: CPUGetter.@brief

### [LibraryGetter](LibraryGetter.md)
::: LibraryGetter.@brief

### [TargetGetter](TargetGetter.md)
::: TargetGetter.@brief



## Utility Classes

These classes provide various utility functions and algorithms used throughout the program.

### [Checksum](Checksum.md)
::: Checksum.@brief

### [ChecksumAlgorithm](ChecksumAlgorithm.md)
::: ChecksumAlgorithm.@brief

### [ChecksumAlgorithmXor](ChecksumAlgorithmXor.md)
::: ChecksumAlgorithmXor.@brief

### [ChecksumComputation](ChecksumComputation.md)
::: ChecksumComputation.@brief


## Unsorted

These classes are not yet sorted into the above categories.

!!! TODO
    Sort the classes into categories, creating new categories as needed.

### [Constant](Constant.md)
::: Constant.@brief

### [EvaluationContext](EvaluationContext.md)
::: EvaluationContext.@brief

### [EvaluationResult](EvaluationResult.md)
::: EvaluationResult.@brief

### [ExpressionParser](ExpressionParser.md)
::: ExpressionParser.@brief

### [LibraryLinker](LibraryLinker.md)
::: LibraryLinker.@brief

### [Linker](Linker.md)
::: Linker.@brief

### [Node](Node.md)
::: Node.@brief

### [ObjectFile](ObjectFile.md)
::: ObjectFile.@brief

### [Output](Output.md)
::: Output.@brief

### [ProgramLinker](ProgramLinker.md)
::: ProgramLinker.@brief

### [Range](Range.md)
::: Range.@brief

### [Scope](Scope.md)
::: Scope.@brief

### [SizeRange](SizeRange.md)
::: SizeRange.@brief

### [StructuredArray](StructuredArray.md)
::: StructuredArray.@brief

### [StructuredBody](StructuredBody.md)
::: StructuredBody.@brief

### [StructuredDictionary](StructuredDictionary.md)
::: StructuredDictionary.@brief

### [StructuredScalar](StructuredScalar.md)
::: StructuredScalar.@brief

### [StructuredValue](StructuredValue.md)
::: StructuredValue.@brief

### [Target](Target.md)
::: Target.@brief

### [UnnamedLabelList](UnnamedLabelList.md)
::: UnnamedLabelList.@brief

### [Unresolved](Unresolved.md)
::: Unresolved.@brief

### [Visibility](Visibility.md)

