# Assembler Source File

A source file contains definitions that will be translated to a **library** or a **binary program**.

It consists of a list of elements and directives, separated by newlines. Instructions, labels, and assignments within an **object** or **macro** are also separated by newlines.

## Directives

### `.cpu`

<pre>
  <code>.cpu <em>name</em></code>
</pre>

This overrides the CPU specified in the target for this file.

### `.pin`

<pre>
  <code>.pin <em>name</em> <em>address</em></code>
</pre>

Sets the address of object *name* to *address* and marks it as used.

### `.section`

<pre>
  <code>.section <em>section-name</em></code>
</pre>

The `.section` directive specifies which section the following objects will be placed in. This determines where in memory they will be placed, and whether they will be included in the **binary program** (**data objects**) or not (**reservation objects**).


### `.target`

<pre>
  <code>.target "<em>target-name</em>"</code>
</pre>

The `.target` directive specifies which [**target definition**](Target-Definition.md) file will be used in translating the file.


### `.use`

<pre>
  <code>.use <em>name</em></code>
</pre>

Marks the object *name* as used.

### `.visibility`

<pre>
<code>.visibility <em>{</em>public<em>|</em>private<em>}</em></code>
</pre>

The `.visibility` directive specifies the visibility of the following elements.

`private` elements are only visible within the same module (library, main program, or target).

`public` elements are also visible to modules using a library. Elements from the program used by the target must also be `public`.


## Elements

### Element Modifiers

Elements can be preceded by one or more modifiers:

#### `.default`

This definition is only used if no regular definition with the same name is found. It can be used by a target to define a default implementation.

#### `.private`

Makes the element private.

#### `.public`

Makes the element public.


### Objects

#### Object Modifiers

##### `.align`

<pre>
<code>.align <em>alignment</em></code>
</pre>

The object's address will be a multiple of *alignment*.


##### `.address`

<pre>
  <code>.address <em>address</em></code>
</pre>

Specifies the address to place the object at.

##### `.used`

Marks the object as used.

##### `.uses`

<pre>
  <code>.uses <em>name</em></code>
</pre>

If the object is used, object *name* is also used.

#### Data Objects

These contain code or other data and are saved in the resulting program binary:

<pre>
  <code><em>[<a href="#element-modifiers">element modifiers</a>]</em> <em>name</em> <em>[<a href="#object-modifiers">object modifiers</a>]</em> {
    <em>body</em>
}</code>
</pre>

#### Reservation Object

These only reserve a certain amount of memory, which will not be initialized and is usually not saved in the resulting program binary:

<pre>
  <code><em>[<a href="#element-modifiers">element modifiers</a>]</em> <em>name</em> <em>[<a href="#object-modifiers">object modifiers</a>]</em> .reserve <em>length</em></code>
</pre>


### Constants

Constants define values that can be used in other parts of the program. They can refer to other constants or objects (which results in their address).

<pre>
  <code><em>[<a href="#element-modifiers">element modifiers</a>]</em> <em>name</em> = <em>value</em></code>
</pre>

### Macros

<pre>
  <code><em>[<a href="#element-modifiers">element modifiers</a>]</em> .macro <em>name</em> <em>[argument[</em>=<em>default-value]</em>, ...<em>]</em> {
    <em>body</em>
}</code>
</pre>

### Functions

<pre>
  <code><em>[<a href="#element-modifiers">element modifiers</a>]</em> <em>name</em> (<em>[argument[</em>=<em>default-value]</em>, ...<em>]</em>) = <em>value</em></code>
</pre>
