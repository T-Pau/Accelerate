## Bugs

- Record object reference while evaluating, otherwise reference is lost for fixed address objects.

## Missing Features

- Allow pinning object to fixed address in map file. (`.pin name address`)
- Evaluate program/library constants in output elements.
- Add optional priorities to sections.
- Allow specifying libraries in map file.
- Support for text. (How to specify encoding?)
- Support for integer as ASCII (e. g. for start sys).
- Check for objects in unknown sections. 

## Older Items

- Fix Tokenizer:at_end to honor ungot token.
- Location: support multi-line locations.
- Parsing objects: end on directive.
