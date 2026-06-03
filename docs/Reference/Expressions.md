# Expressions

Expressions are used in various places in a program.

## Types

### Void

This type represents the absence of a value and is written as `.none`. It can be used in macros as default value for optional arguments. 

### Boolean

A boolean is either `.true` or `.false`. Numbers and strings can be implicitly converted to boolean, with `0` and the empty string being `.false` and all others being `.true`.

### Signed Integers

A signed integer with a maximum precision of 64 bits, giving a value range of -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807.

### Unsigned Integer

An unsigned integer with a maximum precision of 64 bits, giving a value range of 0 to 18,446,744,073,709,551,615.

For hexadecimal numbers, prefix them with `$`, for binary numbers with `%`.

### Floating Point Number

A floating point number expressed as a **double**.

Scientific notation (including an exponent) is not supported yet.

### String

Strings are enclosed in `"` and support Unicode characters.

### Characters

Characters are not a separate type, but a different way to specify an unsigned integer. They are enclosed in `'` and can optionally be followed by `:` and the name of a string encoding. They are translated to an unsigned number according to the string encoding. 

## Operators

### Unary Operators

All unary operators are followed by their operand and have higher precedence than binary operators.

#### `!` (Logical Not)

Converts its operand to boolean and negates it. The result is always boolean.

#### `+` (Unary Plus)

The operand must be a number, the result is the operand unchanged.

#### `-` (Unary Minus)

The operand must be a number, the result is the negative operand. 

#### `^` (Bank Byte)

The operand must be an unsigned integer, the result is the third lowest byte of the operand.

#### `<` (Low Byte)

The operand must be an unsigned integer, the result is the lowest byte of the operand.

#### `>` (Hight Byte)

The operand must be an unsigned integer, the result is the second lowest byte of the operand (high byte of a 16-bit value).

#### `~` (Bitwise Not)

The operand must be an unsigned integer, the result is the operand with each bit inverted.

### Binary Operators

All binary operators are left associative. The following list shows their relative precedence from high to low; operands on one line have the same precedence:

- `<<` (Shift Left), `>>` (Shift Right)
- `*` (Multiply), `/` (Divide), `&` (Bitwise And)
- `+` (Add), `-` (Subtract), `|` (Bitwise Or), `^` (Bitwise Exclusive Or)
- `==` (Equal), `!=` (Not Equal), `<` (Less), `<=` (Less Than), `>` (Greater), `>=` (Greater Than)
- `&&` (Logical And)
- `||` (Logical Or)

#### `<<` (Shift Left)

Both operands must be integers. The left operand is shifted to the left by the number of bits of the value of the right operand. The result has the same type as the left operand.

#### `>>` (Shift Right)

Both operands must be integers. The left operand is shifted to the left by the number of bits of the value of the right operand. The result has the same type as the left operand.

#### `*` (Multiply)

Both operands must be numbers. The result is the product of the two operands. If both operands are integers, the result is an integer, otherwise a float point number.

#### `/` (Divide)

Both operands must be numbers. If both operands are integers, the result is the quotient of the two operands, otherwise a floating point division is performed.

#### `&` (Bitwise And)

Both operands must be unsigned integers. The result is the bitwise and of the two operands.

#### `+` (Add)

Both operands must be numbers. The result is the sum of the two operands. If both operands are integers, the result is an integer, otherwise a float point number.

#### `-` (Subtract)

Both operands must be numbers. The result is the difference of the two operands. If both operands are integers, the result is an integer, otherwise a float point number.

#### `|` (Bitwise Or)

Both operands must be unsigned integers. The result is the bitwise or of the two operands.

#### `^` (Bitwise Exclusive Or)

Both operands must be unsigned integers. The result is the bitwise exclusive or of the two operands.

#### `==` (Equal)

Both operands must be either booleans, numbers, or strings. The result is `true` if both operands are the same, `false` otherwise.

#### `!=` (Not Equal)

Both operands must be either booleans, numbers, or strings. The result is `false` if both operands are the same, `true` otherwise.

#### `<` (Less)

Both operands must be either numbers or strings. The result is `false` if the left operand is less than the right, `true` otherwise.

#### `<=` (Less Equal)

Both operands must be either numbers or strings. The result is `false` if the left operand is grelessater than or equal to the right, `true` otherwise.

#### `>` (Greater)

Both operands must be either numbers or strings. The result is `false` if the left operand is greater than the right, `true` otherwise.

#### `>=` (Greater Equal)

Both operands must be either numbers or strings. The result is `false` if the left operand is greater than or equal to the right, `true` otherwise.

#### `&&` (Logical And)

Both operands are converted to boolean. The result is `true` if both operands are true, `false` otherwise.

#### `||` (Logical Or)

Both operands are converted to boolean. The result is `true` if at least one operand is true, `false` otherwise.
