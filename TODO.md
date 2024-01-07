## Soon

- Move location from BaseExpression to Expression, update centrally
- Character constants, optionally with encoding (`'a':screen`)


## Tests

- `.if .defined` in body
- `.use`, `.uses`, `.pin`, `.used`
- `.define` and `.undefine` are ignored if preprocessor is skipping.
- hex encoding: `.binary_include`, `{{`/`}}` literal in source and object


## StringEncoding

- add support for `\\` escapes


## Bugs

- fix target specified in source file (CPU punctuation doesn't get added, assemlby instructions not recognized)
- fix `.import` in assemlby source file
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

- evaluate ScopeBody to inner body when scope no longer needed.
- Don't serialize signed encoding when value is known to fit.
- Simplify X + N + M and similar.


## Cleanup

- Don't rename CPU notation arguments. (Currently fails for 45gs02 CPU.)


## Older Items

- Fix Tokenizer:at_end to honor ungot token.
- Parsing objects: end on directive.
