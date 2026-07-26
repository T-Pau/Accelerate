# Ideas for Later

<!-- --8<-- [start:summary] -->
This article collects ideas for improvements. 
<!-- --8<-- [end:summary] -->

## Functional Improvements

These are ideas that improve existing functionality, or add new functionality.

### Argument Restrictions for Macros and Functions

Allowing to restrict arguments to certain types, value ranges, or encodings would allow for better error messages and further evaluation of their bodies.

## Simplification Improvements

These are ideas that simplify expressions and bodies, thus making libraries more compact.

### Inline ArgumentConstant Only Used Once

If an ArgumentConstant is only used once, we can evaluate the reference to its expression. This way, the scope defining it can remove it and maybe itself.


## Optimizations

These are ideas that improve performance, either in terms of speed or memory usage.

### Pre-create CPU Invocation Bodies

If we can group the sets of address modes that can be matched by the same node list, we can pre-create the CPU invocation bodies for them, and store them in the instruction.  We would thus not have to recreate them for every invocation.

### Optional Cloning

Implement `needs_cloning()`, defaulting to `true`. If `false`, reuse existing Expression/Body.

Implement `children_need_cloning()` that uses `traverse()` to check if any of the children need cloning. Can be used to implement `needs_cloning()`, but is not the default because it would default to `false` (since `traverse()` defaults to no children).
