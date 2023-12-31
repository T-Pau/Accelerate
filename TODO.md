## Soon

- Move location from BaseExpression to Expression, update centrally
- Use braces to scope `.if` in objects.
- Treat all source files as one library, only import what's used in target.
- Add `.pin <name> <address>` to target to specify address of object, mark it as used.
- Add `.uses <object>`, `.used` modifiers to objects.
- Add `.use <object>` directive.
- Allow objects, constants in target.
- Character constants, optionally with encoding (`'a':screen`)
- Add `.repeat` in objects.

## StringEncoding

- fix target specified in source file (CPU punctuation doesn't get added, assemlby instructions not recognized)
- allow adding library in target description
- fix `.import` in assemlby source file

## Bugs

- Don't use full file name for `.import` in object file. 
- `.segment` not reported as unknown directive while assembling
- columns are off when source contains TAB.
- Serialize which library object was resolved in (needed to refer to private objects). (still needed?)
- Test/fix labels (named and unnamed) in macros.
- Resolve macros in library they're defined in. Add placeholders for arguments in local environment.


## Missing Features

- Evaluate program/library constants in output elements.
- Add optional priorities to sections.


## Optimizations

- Don't serialize signed encoding when value is known to fit.
- Simplify X + N + M and similar.


## Cleanup

- Don't rename CPU notation arguments. (Currently fails for 45gs02 CPU.)


## Older Items

- Fix Tokenizer:at_end to honor ungot token.
- Parsing objects: end on directive.
