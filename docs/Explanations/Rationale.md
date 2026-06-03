# Rationale

While developing programs for multiple 8-bit computers, existing cross assemblers did not fit my needs in some key regards: 

- They usually only target one CPU family, and all have slightly different syntax.
- They don’t allow precise control where objects are placed in memory when necessary, while efficiently using the remaining memory automatically.
- Adding a new CPU variant requires understanding the inner workings of the assembler, changing and recompiling the assembler, and either getting the change accepted upstream or maintaining a fork.
- Not all assemblers support automatic dependency tracking of included files and used libraries, which makes integrating them into a build system much harder.

To address these issues, Accelerate was created.

CPU definitions are text files, so it is easy to add a new CPU family without altering the assembler itself. Definitions for variants can include the definition of the main CPU model and only specify the differences.

The output file format is also defined in a text file, allowing to add new platforms and their requirements. File formats that further encode the binary data (like tape or disk images) need a separate program to do the final conversion.

The program itself is divided into small objects, one for each function, variable, or table. This allows the assembler more flexibility in placing them in memory. For each object, the constraints imposed by the platform can be specified, for example a fixed address, address alignment, or placement in a specific memory region. Gaps left by placing them are filled with other objects.

This also allows libraries to be more efficient, since only the objects actually used will be included in the program.

