## Missing Features

- Evaluate program/library constants in output elements.
- Add $ to special variables in output elements. (Needs support in tokenizer.)
- Differentiate between signed and unsigned size constraints (`:-1` for signed byte).
- Add optional priorities to sections.
- Allow specifying libraries in map file.
- Support for text. (How to specify encoding?)
- Support for integer as ASCII (e. g. for start sys).
- Specify CPU in map file, rename map to target, specify in assembler as well.
- Check for objects in unknown sections. 

## Older Items

- Fix Tokenizer:at_end to honor ungot token.
- Location: support multi-line locations.
- Parsing objects: end on directive.
