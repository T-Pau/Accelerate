# Introduction

Accelerate is a cross-assembler that runs on modern computers. It translates assembly language programs for retro computers into binaries that can run on those computers. It supports a variety of computers and CPU families, and adding support for more is relatively easy and can be done without changing and recompiling the assembler itself.

All source, definition, and intermediary files are text files, although the latter are not meant to be edited by hand.

The program is organized into [**source files**](../Reference/Assembler-Source-File.md) and **libraries**, which contain [**objects**](../Reference/Assembler-Source-File.md#objects), [**constants**](../Reference/Assembler-Source-File.md#constants), [**macros**](../Reference/Assembler-Source-File.md#macros), and [**functions**](../Reference/Assembler-Source-File.md#functions). The assembly process is directed by a  [**target definition**](../Reference/Target-Definition.md), which references a [**CPU definition**](../Reference/CPU-Definition.md). 
