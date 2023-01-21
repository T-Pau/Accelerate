# Unary Operators

All unary operators have the same precedence, since only one operator can precede any one operand directly.

### Unary Plus (`+`)

Counterpart to unary `-`, does nothing.

### Unary Minus (`-`)

Changes sign of operand.

### Bitwise Not (`~`)

Gets the bitwise not of the operand. The operand must be integer. The result has the same size as the operand.

### Low Byte (`<`)

Extracts the low byte of the operand.

### High Byte (`>`)

Extracts the second-lowest byte of the operand (which is the high byte of a 16-bit value).

### Bank Byte (`^`)

Extracts the third-lowest byte (which is the high byte of a 24-bit value).


# Binary Operators

All binary operators are left associative. The following sections are listed from lowest precedence to highest:

## Size

### Size (`:`)

Sets the size of the left operand in bytes to the value of the right.


## Additive

### Add (`+`)

Adds the operands.

### Subtract (`-`)

Subtracts the right operand from the left.

### Bitwise Or (`|`)

Gets the bitwise or of the operands. Both operands must be integers.


## Multiplicative

### Multiply (`*`)

Multiplies the operands.

### Divide (`/`)

Divides the left operand by the right. If both operands are integers, the result is an integer.

### Modulo ()

Gets the remainder of dividing the left operand by the right. Both operands must be integers.

### Bitwise And (`&`)

Gets the bitwise and of the operands. Both operands must be integers.

### Bitwise Exclusive Or (`^`)

Gets the bitwise exclusive or of the operands. Both operands must be integers. The result has the size of the bigger operand.

### Shift Left (`<<`)

Shifts the left operand to the left by the value of the right operand number of bits. Both operands must be integers.

### Shift Right (`>>`)

Shifts the left operand to the right by the value of the right operand number of bits. Both operands must be integers.
