## Bugs

- Don't use full file name for .import in object file. 
- Record object reference while evaluating, otherwise reference is lost for fixed address objects.
- Isolate expanded macro body.
- .segment not reported as unknown directive while assembling
- columns are off when source contains TAB.


## Missing Features

- Allow pinning object to fixed address in map file. (`.pin name address`)
- Evaluate program/library constants in output elements.
- Add optional priorities to sections.
- Allow specifying libraries in map file.
- Support for text. (How to specify encoding?)
- Support for integer as ASCII (e. g. for start sys).
- Check for objects in unknown sections. 


## Optimizations

- Remove unnamed label body element when no longer needed.
- Don't serialize signed encoding when value is known to fit.
- Simplify X + N + M and similar.


## Cleanup

- Don't rename CPU notation arguments. (Currently fails for 45gs02 CPU.)


## Older Items

- Fix Tokenizer:at_end to honor ungot token.
- Parsing objects: end on directive.
