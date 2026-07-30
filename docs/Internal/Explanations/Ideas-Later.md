# Ideas for Later

<!-- --8<-- [start:summary] -->
This article collects ideas for improvements. 
<!-- --8<-- [end:summary] -->

## Functional Improvements

These are ideas that improve existing functionality, or add new functionality.

### Explicit Unsigned Encoding

For `:n` encodings, allow signed and unsigned values that fit into `n` bytes. Use `:+n` to only allow unsigned values. 

This would allow `lda #-2` to be encoded as `lda #$fe` as expected, instead of being an error.


### Argument Restrictions for Macros and Functions

Allowing to restrict arguments to certain types, value ranges, or encodings would allow for better error messages and further evaluation of their bodies. Also allow specifying the return type of functions.

This would allow for better error messages, and also allow for further evaluation of their definitions and uses.


## Simplification Improvements

These are ideas that simplify expressions and bodies, thus making libraries more compact.


### Move `.pc` Labels into Instruction's Body

Only the instruction needs the label for `.pc`. Having it in the instruction's body makes it an ArgumentConstant, which can be inlined and removed from the scope, Also, it no longer needs `.label_0` names.


### Rearrange BinaryExpressions to Evaluate More Known Values

`(($1234-(test_bcc+$02))-$02)` can be simplified to `($1234-test_bcc)`.

Unless we come up with a clever way to automatically find possible simplifications, it's probably not worth implementing it all by hand. But we can implement a few common ones, like relative addressing calculations as in the above.


## Optimizations

These are ideas that improve performance, either in terms of speed or memory usage.


### Pre-create CPU Invocation Bodies

If we can group the sets of address modes that can be matched by the same node list, we can pre-create the CPU invocation bodies for them, and store them in the instruction.  We would thus not have to recreate them for every invocation.


### Optional Cloning

Implement `needs_cloning()`, defaulting to `true`. If `false`, reuse existing Expression/Body.

Implement `children_need_cloning()` that uses `traverse()` to check if any of the children need cloning. Can be used to implement `needs_cloning()`, but is not the default because it would default to `false` (since `traverse()` defaults to no children).
