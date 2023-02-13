# File Types

## CPU Definition

## Target/Platform Definition

## Source

## ParsedValue

## Module

# Components

## Module

- exported symbols
- exported constants
- imported symbols
- used modules
- units

Allow re-exporting symbols/constants of used modules

## File

## Section

## Range

## Scope

Visibility of a symbol: unit, file?, module, global

## Constant

A constant set in the assembler source:

```
base_address = $1000
```

## Object

A constant or unit

## Unit

This is the basic unit of the linker. 

- containing section
- relative range of used bytes
- content (expressions for data of used bytes)