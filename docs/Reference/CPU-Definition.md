# CPU Definition

A **CPU definition** defines the syntax of the assembly instructions and how they are encoded.

## Directives

### `.addressing_mode`

<pre>
<code>.addressing_mode <em>name</em> {
    <em>parameters</em>
}</code>
</pre>

#### `notation`

This parameter is optional. If it is missing, the notation is just the mnemonic of the instruction.

#### `arguments`

This parameter is optional. If it is missing, the addressing mode has no arguments.

#### `encoding`

##### `.opcode`

##### `.pc`

### `.argument_type`

#### `.enum`

<pre>
<code>.argument_type <em>name</em> map {
    <em>name</em>: <em>encoded-value</em>
    <em>...</em>
}</code>
</pre>

The argument must be one of the specified *name* and is encoded as the corresponding *encoded-value*. All names must be keywords, all values must be integers.

#### `.fits`

<pre>
  <code>.argument_type <em>name</em> fits <em>integer_encoding</em></code>
</pre>  

The argument must fit in the specified integer encoding.

For example:
```
.argument_type byte fits 1
.argument_type signed_byte fits -1
.argument_type word fits 2
```

#### `.map`

<pre>
<code>.argument_type <em>name</em> map {
    <em>source-value</em>: <em>encoded-value</em>
    <em>...</em>
}</code>
</pre>

The argument must be one of the specified *source-value* and is encoded as the corresponding *encoded-value*. All values must be integers.


#### `.range`

**Warning: This argument type is currently broken. Don't use it.**


### `.byte_order`

Specify the native byte order of the CPU.

<pre>
  <code>.byte_order <em>byte_order</em></code>
</pre>

Each digit in *byte_order* corresponds to one byte, with `1` representing the least significant and `8` the most significant byte. Bytes are encode in memory in the order they are listed. For example, little endian is `12345678`, big endian is `87654321`.


### `.instruction`

<pre>
<code>.instruction <em>mnemonic</em> {
    <em>addressing-mode</em>: <em>opcode</em>
    ...
}</code>
</pre>

### `.syntax`

<pre>
  <code>.syntax <em>[</em>keywords<em>|</em>punctuation<em>]</em> "<em>symbol</em>" ...</code>
</pre>

This specifies punctuation and keywords used the assembler syntax. Keywords that are valid identifiers (begin with a letter or underscore and contain only letters, underscore, and digits) need not be specified.
