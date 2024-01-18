## Soon

- specify padding character in string encoding (`:string(<n>, '<c>')`)
- parse signed non-native integer encoding (`:-big_endian[(<n>)]`, `:little_endian(-<n>)`)
- verify that integer encoding size <= 8
- Allow non-constant expressions for range in `.binary_file`.
- Character constants, optionally with encoding (`'a':screen`)
- Track constant usage, include used constants in symbol table.
- Optionally omit private symbols of libraries from symbol talbe. 
- Move location from BaseExpression to Expression, update centrally
- Include library name in symbol table.


## Tests

- `.if .defined` in body
- `.use`, `.uses`, `.pin`, `.used`
- `.define` and `.undefine` are ignored if preprocessor is skipping.
- non-native integer encodings
- range for `.binary_file`


## Documentation

- preprocessor
- target additions (objects in target, only used from `.output` included in program)
- non-native integer encodings
- range for `.binary_file`
- in-body `.if` syntax
- `.repeat`, `.fill`


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
