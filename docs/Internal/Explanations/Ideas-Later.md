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

### Optional Cloning

Implement `needs_cloning()`, defaulting to `true`. If `false`, reuse existing Expression/Body.

Implement `children_need_cloning()` that uses `traverse()` to check if any of the children need cloning. Can be used to implement `needs_cloning()`, but is not the default because it would default to `false` (since `traverse()` defaults to no children).

### Eliminating Scopes

Implement `will_evaluate()` in VariableExpression that returns `true` if it will evaluate to the entity it refers to. This will return `true` if the entity has a value or if it is has `SCOPE` visibility and its value is a Constant or Object or it is only referred to once. ScopeBody will use this to determine if it can eliminate itself. This will get rid of instruction argument scopes for instructions with a single variant and resolved constraints.
