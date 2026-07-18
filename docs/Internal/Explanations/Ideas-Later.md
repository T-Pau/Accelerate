# Ideas for Later

<!-- --8<-- [start:summary] -->
This article collects ideas for improvements. 
<!-- --8<-- [end:summary] -->

## Functional Improvements

### Argument Restrictions for Macros and Functions

Allowing to restrict arguments to certain types, value ranges, or encodings would allow for better error messages and further evaluation of their bodies.

## Optimizations

### Pre-create CPU Invocation Bodies

If we can group the sets of address modes that can be matched by the same node list, we can pre-create the CPU invocation bodies for them, and store them in the instruction.  We would thus not have to recreate them for every invocation.
