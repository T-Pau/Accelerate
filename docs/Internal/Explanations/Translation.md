# Translating a Program or Library

<!-- --8<-- [start:summary] -->
This article describes the steps that Accelerate takes to translate a program from source code to output or how it translates a library. 
<!-- --8<-- [end:summary] -->

It provides an overview of the translation process, and for each step explains what happens, what is guaranteed to be known after it, and what errors can occur.

To report as many errors as possible, Accelerate will continue translating after an error is encountered. The part causing the error will be erroneous. Operations on these will propagate the errors as appropriate, but will not cause additional error messages.

!!! warning
    Bringing the implementation in line with this description is currently underway.

!!! question
    Enter names into the environment during parsing, or in a separate pass? They need to be entered before [Step 2: Resolve names](#2-resolve-names).

!!! warning
    I'm unclear on how to handle if bodies. Should I enter the names even if I'm unsure the part will actually be translated? Otherwise, the resolution won't work. Maybe a good middle ground is to discard parts that are known not to be translated prior to the resolution step. Is that good enough? It won't work for `.exists()`. The other option is to have two resolution passes.

!!! note
    [Step 2: Resolve names](#2-resolve-names) needs to collect all referenced constants, macros, and functions, so they can be evaluated before evaluating this entity. 
    
    Ensuring the right order is done in the ObjectFile evaluation methods, not the Body/Expression evaluation methods. This centralizes the processing, and keeps the Body/Expression evaluation methods simpler.

!!! note
    Unresolved entities are no longer collected, they are reported during [Step 2: Resolve names](#2-resolve-names).

# 1. Parse all Entities

All source files are parsed and all libraries are loaded.

This results in a collection of objects, constants, macros, and functions, arranged in nested scopes.

Duplicate names in the same scope will result in an error. Duplicate names in different scopes are allowed, and will be resolved to the closest scope.

After this step, all entities are known.


# 2. Resolve Names

This step resolves all names in bodies and expressions in their respective scopes. It is implemented by the `resolve()` methods of entities, body parts, and expressions.

Only entities in the program or library being translated are evaluated; names in loaded libraries are already resolved.

The following actions are performed during this pass:

* Resolve unnamed labels.
* Resolve arguments in macro and function definitions. These resolve to placeholders that will be filled in for each call when evaluating calls in step 4.
* Resolve labels and variables in bodies and expressions.

Macro and function bodies are not copied to their call sites during this pass; they are simplified in the next step first, so the simplification doesn't need to be done for each call.

Values of constants and variables are not propagated during this pass, even if they are known. This makes tracking used objects easier.

Each entity records the names of all objects it uses.

After this pass, all names are resolved to a specific entity. Unresolved names will result in an error.


# 3. Evaluate Expressions, Pass 1

This pass replaces all variable expressions with their values, if known, and compute all expressions with known values. It will also propagate refined value ranges of expressions. It is implemented by the `evaluated()` methods of entities, body parts, and expressions.

This pass will be able to remove the object address from label offset expressions within the same object, resulting in a range expression (if the size of the preceding body elements is not known) or a value (if the size is known).

Variables will be evaluated before they are used, so all known information will be propagated. Cyclic uses will be detected and result in an error.


# 4. Expand Macro and Function Calls

Macro and function bodies are copied to their call sites, and their arguments are substituted. Then, expressions in their bodies are simplified (like in step 3). This is implemented by the `expand_calls()` methods of macros and functions.

Calls in macros and functions are evaluated before the macro or function is used, so only simplified bodies are copied. Circular calls will be detected and result in an error.

After this pass, the sizes of all body elements must be known. Ambiguous sizes will result in an error.


# 5. Place Used Objects in Memory (Program Only)

This pass collects all used objects, starting with the target's `.output` entity and objects marked explicitly as used. Then recursively all objects referenced by these objects are collected, until no new objects are found.

It will then place all used objects in memory according to their constraints, and assign memory addresses to them. If some constraints cannot be fulfilled, an error will be reported.

After this pass, the address of all used objects is known.


# 6. Evaluate Expressions, Pass 2 (Program Only)

This pass is only done when translating a program, not a library. It is implemented by the `evaluated()` methods of entities, body parts, and expressions.

Since all object addresses are now known, all expressions can now be fully computed.

After this pass, all expressions are fully computed, and the byte contents of all object bodies are known. 

If some expressions cannot be evaluated, an error will be reported.


# 7. Copy Used Objects to Memory (Program Only)

This pass copies the byte contents of all used objects to their assigned memory addresses in the output file. This is implemented by the `encode()` methods of body parts.


# 8. Generate Output

For programs, the output file is generated according to the target's output format. 

For libraries, all entities are written to the library file. The omission of unused entities is not done for libraries, since what is unused depends on the program using the library. Libraries are simplified as much as possible without knowing the addresses of objects or the values of imported variables.
