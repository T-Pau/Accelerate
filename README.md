**Warning:** This program is still in the early stages of development. It contains bugs and features that aren't completely implemented yet. Also, details may still change. 

# Accelerate

Accelerate is a flexible cross-assembler running on modern systems targeting multiple 8- and 16-bit computers. Support for additional CPU types, memory models, and output formats can easily be added via configuration files.

Programs are divided into small objects like a single subroutine or variables. These objects are automatically arranged in memory according to provided constraints like alingment or placement within a certain memory region. This allows to fulfil the hardware requirements of the target machine while automatically using the remaining memory optimally. 

## Building Accelerate

Accelerate is written in C++17 and should run on any modern platform. It also comes with an extensive test suite.

To build it, you need a C++17 compiler and [cmake](https://cmake.org). To run the tests, you need [Python](https://www.python.org) and [nihtest](https://github.com/nih-at/nihtest).
