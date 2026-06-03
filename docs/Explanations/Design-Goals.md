---
layout: page
title: Design Goals
---
These are the design goals of Accelerate, in order of importance:

## 1. Be correct and consistent.

Correct programs should be translated correctly, while errors in programs should be detected and reported, not silently lead to incorrect or undefined behavior.

Accelerate supports reproducible builds: if given the same input files, it will create byte-wise identical results. This means that if a program is built from the same sources on two different computers, the resulting program will be byte-wise identical, which makes testing and creating distributions easier. (This does not hold across different versions of Accelerate.)


## 2. Allow extending its functionality easily.

Accelerate is a generic assembler without built-in knowledge of any CPU architecture, memory layout, or output file format. All these aspects are defined in easily readable configuration files.

This allows adding support for new target computers without modifying Accelerate itself. 

A definition file can extend another, thus allowing to adapt existing platform support to your needs with a minimum of effort.


## 3. Allow precise control over the produced binary when needed, without requiring it when not.

You can specify the exact address where to place an object, or you can specify a memory region and alignment (i. e. the address must be divisible by a certain number). Accelerate will make sure the requirements are met, while placing the other elements as space-efficiently as possible.


## 4. Meet modern expectations of a programming tool.

Accelerate is written in C++20 and should work on all modern platforms. It is a command line tool that follows the usual Unix conventions.

Accelerate integrates well with development and build environments: If it encounters an error, it will not produce output files and exit with a non-zero status. Also, it supports gcc-style dependency tracking.



## 5. Be easy to use.

The language implemented by Accelerate tries to be easy to use: consistent, easy to remember, and readable.
