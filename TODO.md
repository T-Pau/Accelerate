## Bugs

- Fix serializing partially resolved object files (compile mode). 
- Don't use full file name for .import in object file. 
- .segment not reported as unknown directive while assembling
- columns are off when source contains TAB.
- Serialize which library object was resolved in (needed to refer to private objects).
- Test/fix labels (named and unnamed) in macros.
- Resolve macros in library they're defined in. Add placeholders for arguments in local environment.


## Missing Features

- Clean
- Allow pinning object to fixed address in map file. (`.pin name address`)
- Evaluate program/library constants in output elements.
- Add optional priorities to sections.
- Allow specifying libraries in map file.
- Support for text. (How to specify encoding?)
- Support for integer as ASCII (e. g. for start sys).
- Check for objects in unknown sections. 


## Optimizations

- Don't serialize signed encoding when value is known to fit.
- Simplify X + N + M and similar.


## Cleanup

- Don't rename CPU notation arguments. (Currently fails for 45gs02 CPU.)


## Older Items

- Fix Tokenizer:at_end to honor ungot token.
- Parsing objects: end on directive.
