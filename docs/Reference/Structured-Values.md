# Structured Values

Structured values are used to represent complex data structures in a hierarchical manner. They are Accelerate's counterpart to JSON or YAML, enhanced with the ability to include expressions and bodies of code.

They are used in user written configuration files like definitions for CPUs and targets, as well as internal files like libraries.

Each type of structured value begins with a specific character, allowing for easy identification of the type of value being represented. 


## Scalar

Scalar values are single tokens or a list of tokens. They begin with `:`.

```
: token1 token2 token3 ...
```


## Array

Arrays begin with `[`, followed by a list of elements, each on its own line, and end with `]`. The elements begin with their own structured value type character, allowing for nested structures.

Elements can be any structured value.

```
[
    element1
    element2
    ...
]
```


## Dictionary

Dictionaries begin with `{`, followed by a list of key-value pairs, each on its own line, and end with `}`. Keys and values are separated by whitespace. Keys are tokens, and values can be any structured value.

```
{
    key1 value1
    key2 value2
    ...
}
```


## Body

Bodies begin with `<`, followed by the body content, and end with `>`. The body content uses the same syntax as a body in an object. Depending on the context, the availability of directives may vary.

```
<
    body content
    ...
>
```


## Example

This provides a hypothetical example of a structured value representing an object.

```
{
    name: foo
    used_entities [
        : bar
        : baz
    ]
    address: $2000
    body <
        .data bar:2, baz:2
    >
}
```
