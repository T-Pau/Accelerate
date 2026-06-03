---
title: Accelerate
---
## What is Accelerate?

Accelerate is a flexible cross-assembler running on modern systems targeting multiple 8- and 16-bit computers. Support for additional CPU types, memory models, and output formats can easily be added via configuration files.

Programs are divided into small objects like a single subroutine or variable. These objects are automatically arranged in memory according to provided constraints like alignment or placement within a certain memory region. This allows to fulfill the hardware requirements of the target machine while automatically using the remaining memory optimally.

## Warning

**This program is still in the early stages of development. It contains bugs and features that aren't completely implemented yet. Also, details may still change in backwards incompatible ways.**

## Why Use Accelerate?

Accelerate is especially useful if you develop for multiple CPU architectures, since it provides a uniform environment.

If your program has a complicated memory layout, Accelerate helps fulfilling them without having to do it all manually.

## Getting Started

First, [build and install](https://github.com/T-Pau/Accelerate/blob/main/INSTALL.md) Accelerate.

Then, assemble your program:

<code>xlr8 -o <em>program</em> -t <em>target</em> <em>sources ...</em></code>

## Reporting Problems

It's an old adage that it's never the compiler's fault, but since Accelerate is still young, bugs and misfeatures are likely. If you found a problem, please [create an issue on GitHub](https://github.com/T-Pau/Accelerate/issues/new/choose) or let us know at [accelerate@tpau.group](mailto:accelerate@tpau.group).

Also let us know if the documentation is incomplete, inaccurate, or hard to understand.
