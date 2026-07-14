# Coding Conventions

<!-- --8<-- [start:summary] -->
This article describes the coding conventions Accelerate uses. Following these conventions makes the code base feel like a consistent whole.
<!-- --8<-- [end:summary] -->

!!! note
    These are guidelines, not hard rules. The objective is code that's efficient, easy to read, understand, and maintain. If a guideline gets in the way, use your judgment, but don't disregard it without good reason.

!!! Todo
    This should be moved to [T'Pau C++ Kernal](https://tpau-cpp-kernal.tpau.group/).


## Using C Features

Where available, prefer C++ features over C features.

- Use automatic resource management with the RAII (Resource Acquisition Is Initialization) idiom. 

    - If an API does not support automatic resource management, wrap it in a class that adds it, thus keeping manual resource management localized.

    - Avoid `new` and `delete`.

- Use `#include<cstdint>` instead of `#include<stdint.h>`.

- Use `std::limits<uint32_t>::max()` instead of `UINT32_MAX`.

- Use constants instead of `#define` for constant values. 

- Use streams instead of `printf()` and `scanf()`.

- Use `std::format()` instead of `sprintf()`.

Use the preprocessor sparingly, and contain its use.


## Passing Complex Types

If the called function takes ownership of the argument, pass it by value and use `std::move()` to move it into the function, otherwise pass it by `const` reference.

For shared pointers, if the called function takes (co-)ownership of the argument, pass it by value and use `std::move()` to move it into the function, otherwise pass the raw pointer.

`std::string` is a special case: if the called function doesn't take ownership of the string, doesn't use it for key lookup in `std::unordered_map`, and doesn't require its C-string representation, use `std::string_view` instead of `const std::string&`.


## Platform-Specific Code

 If there is no native C++ API for a feature, and calling it from C++ is not straightforward, or there are multiple platform-specific APIs for the same feature, wrap it in a common API.
  
Use `cmake` to check for the availability of the different platform-specific APIs, and add defines to `config.h`.  Prefer this to checking for the platform in the code, as it is more robust. 

In the implementation of the common API, use `#ifdef` to select the platform-specific implementation. This contains the platform-specific code and use of the preprocessor in one place, keeping the rest of the code base clean.

Also wrap APIs that don't support automatic resource management. Contain the manual resource management in the wrapper class.


## Names

Use descriptive names and avoid abbreviations. Most IDEs have auto-completion, so saving a few keystrokes is not worth the loss of clarity. 

- Use PascalCase (e. g. `MyFile.cc`) for file names. Name files after the main class in the file. If there is no main class, use a name describing its contents. If the file does not contain a class and a lowercase name fits better, use kebab-case (e. g. `my-file.cc`).

- Use PascalCase for types (e. g. `MyClass`).

- Use snake_case (e. g. `my_function`) for functions and variables.

- Use UPPERCASE (e. g. `MY_ENUM_VALUE`) for enum values and preprocessor defines.


## Include Files

Make sure all include files are self-contained, meaning they can be included without requiring other include files being included first. To ensure this, include it first in the corresponding `.cc` file.

Use the following order of includes in a file:

1. The corresponding header file.
2. C++ standard library headers.
3. Third-party library headers.
4. Project headers.

Use angle brackets for system and third-party library headers, and double quotes for project headers. Separate the different groups of includes with a blank line.

Wrap the copyright notice and content of the include file in an include guard. Guard against circular dependencies and multiple inclusion of the same file:

```cpp
#ifdef IN_XLR8_MY_FILE_H
#error "circular include file dependency detected"
#endif
#define IN_XLR8_MY_FILE_H
#ifndef HAD_XLR8_MY_FILE_H
#define HAD_XLR8_MY_FILE_H

// Copyright Notice and License information

// ... contents of the include file ...

#endif // HAD_XLR8_MY_FILE_H
#undef IN_XLR8_MY_FILE_H
```


## Exposing Members Read-Only

If a member of a class should be readable by other classes, but writable only by the class itself (e. g. to maintain invariants), declare it `private` and append `_` to its name. Provide a `public` getter method marked `const`.

```cpp
class MyClass {
  public:
    int read_only_int() const {return read_only_int_;}
    const std::string& read_only_string() const {return read_only_string_;}

  private:
    int read_only_int_{0};
    std::string read_only_string_{"default value"};
};
```
