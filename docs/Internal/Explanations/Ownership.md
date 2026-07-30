# Ownership of Data Structures

<!-- --8<-- [start:summary] -->
Accelerate has complex relationships between its data structures.  This article explains the lifetimes and ownership relationships of the main data structures.
<!-- --8<-- [end:summary] -->

This section breaks the classes down into their use cases so we can reason about their relationships to other classes, making sure there are no ownership cycles, and they are not destroyed while they are still needed. This problem is too big to hold in my head at once, so I will break it down into smaller pieces. 

*Owns* relationships are implemented using `std::shared_ptr`. It is fine for multiple objects to own the same object, as long as that does not create a cycle.

*Referenced by* relationships are implemented using `std::weak_ptr` or a raw pointer. Referenced objects must not be destroyed before the object that references them.

*Owned by* and **Referenced by* are their inverse relationships. If A owns B, then B is owned by A. If A references B, then B is referenced by A.


### Module

**Linker**:

- Responsible for translating the program and producing the output.
- Lives forever.
- Referenced by nothing.
- References nothing.
- Root of ownership tree.
- Owns **Module**.


**Module**:

- The main program, target, or an imported library.
- Lives forever.
- Referenced by nothing.
- References nothing.
- Owned by **Linker**.
- Owns **VisibleScope**, **ExternalConstant**, **Object**, **Function**, **Macro**.


### Scopes

Scopes own their parent scope. This is unusual for a tree structure (which usually owns its children), but since the scope trees are destroyed from the leaves up, this reflects their life cycle better. They do not own the entities they contain, but they do reference them. The entities are owned by the module, entity, or body that contains them.

**VisibleScope**:

- A Scope of type `PUBLIC`, `PRIVATE`, or `FILE`. 
- Lives forever.
- Referenced by parent **VisibleScope**.
- References **ExternalConstant**, **Object**, **Function**, **Macro**, child **VisibleScope**.
- Owned by **Module**, child **VisibleScope**, child **EntityScope**.
- Owns parent **VisibleScope**.


**EntityScope**:

- A Scope of type `ENTITY`. 
- Lives forever.
- Referenced by nothing.
- References **EntityConstant**.
- Owned by **Object**, **Macro**, **Function**, child **ArgumentScope**.
- Owns parent **VisibleScope**.

**ArgumentScope**:

- A Scope of type `ARGUMENT`.
- Lives for the lifetime of its containing **ArgumentBody**.
- Referenced by nothing.
- References **ArgumentConstant**.
- Owned by **ArgumentBody**, child **ArgumentScope**.
- Owns parent **ArgumentScope**, parent **EntityScope**.


### Entities

**Object**:

- An object.
- Lives forever.
- Referenced by **VisibleScope**, **ObjectExpression**.
- References nothing.
- Owned by **Module**.
- Owns **EntityScope**, **EntityConstant**, **Body**, **Expression**.

**Macro**:

- A macro.
- Lives forever.
- Referenced by **VisibleScope**, **MacroBody**.
- References nothing.
- Owned by **Module**.
- Owns **EntityScope**, **EntityConstant**, **Body**, **Expression**.

**Function**:

- A function.
- Lives forever.
- Referenced by **VisibleScope**, **FunctionExpression**.
- References nothing.
- Owned by **Module**.
- Owns **EntityScope**, **Expression**.

**ExternalConstant**:

- A constant that is defined outside an **Object** or **Macro**, or inside an **Object** but exported via explicit visibility.
- Lives forever.
- Referenced by **VisibleScope**, **ExternalConstantExpression**.
- References nothing.
- Owned by **Module**.
- Owns **Expression**.

**EntityConstant**:

- A constant that is defined inside an **Object** or **Macro**.
- Lives forever.
- Referenced by **EntityScope**, **EntityConstantExpression**.
- References nothing.
- Owned by **Object**, **Macro**.
- Owns **Expression**.

**ArgumentConstant**:

- A constant that is defined inside an **ArgumentBody**.
- Lives for the lifetime of its containing **ArgumentBody** or as long as it is needed.
- Referenced by **ArgumentScope**.
- References nothing.
- Owned by **ArgumentBody**, **FunctionExpression**, **ArgumentConstantExpression**.
- Owns **Expression**.


### Bodies and Expressions

Generally, Bodies and Expressions form tree structures. A Body or Expression is owned by its parent Body or Expression, owns the child Bodies and Expressions. They may share subtrees (after cloning). They cannot contain cycles, are not referenced by any other Body or Expression, and are destroyed when their parent no longer needs them. Bodies can contain Bodies and Expressions, and Expressions can contain Expressions.

When Bodies or Expressions can be simplified during evaluation, they will be replaced by their simplified form, and the original Body or Expression subtree will be destroyed. This is done by the Body or Expression itself, which will replace itself in its parent with the simplified form.

Some types of Bodies and Expressions refer to or own other classes. These are listed here:

**ArgumentBody**:
- A Body for a macro call.
- Lives for the lifetime of its parent **Body** or as long as any of its **ArgumentConstants** are needed.
- Referenced by nothing.
- References nothing.
- Owned by its parent **Body**, **MacroBody**.
- Owns **ArgumentScope**, **ArgumentConstant**.

**LabelOffsetBody**:
- A Body representing the position of a label.
- Lives for the lifetime of its parent **Body** or as long it is needed by a **LabelOffsetExpression**.
- Referenced by nothing.
- References nothing.
- Owned by its parent **Body**, **LabelOffsetExpression**.
- Owns nothing.

**LabelOffsetExpression**:
- An Expression representing the offset of a label. They are used to create a constant that represents the address of the label. This can escape the containing entity via an **ExternalConstant**.
- Lives for the lifetime of its containing **Body** or **Expression** or until it is no longer needed.
- Referenced by nothing.
- References nothing.
- Owned by its parent **Expression**.
- Owns **LabelOffsetBody** (this is used to determine if the **LabelOffsetBody** is still needed).

**ObjectExpression**:
- An expression that refers to an **Object**.
- Lives for the lifetime of its parent **Body** or **Expression** or until it is no longer needed.
- Referenced by nothing.
- References an **Object**.
- Owned by its parent **Body** or **Expression**.
- Owns nothing.

**ExternalConstantExpression**:
- An expression that refers to an **ExternalConstant**.
- Lives for the lifetime of its parent **Body** or **Expression** or until it is no longer needed.
- Referenced by nothing.
- References an **ExternalConstant**.
- Owned by its parent **Body** or **Expression**.
- Owns nothing.

**EntityConstantExpression**:
- An expression that refers to an **EntityConstant**.
- Lives for the lifetime of its parent **Body** or **Expression** or until it is no longer needed.
- Referenced by nothing.
- References an **EntityConstant**.
- Owned by its parent **Body** or **Expression**.
- Owns nothing.

**ArgumentConstantExpression**:

- An expression that refers to an **ArgumentConstant**.
- Lives for the lifetime of its parent **Body** or **Expression** or until it is no longer needed.
- Referenced by nothing.
- References nothing.
- Owned by its parent **Body** or **Expression** (which is inside the **ArgumentBody** that owns the **ArgumentConstant** it refers to).
- Owns **ArgumentConstant** (this is used by the **ArgumentBody** to determine if the **ArgumentConstant** is still needed).

**FunctionExpression**:

- An expression that represents a call to a **Function**.
- Lives for the lifetime of its parent **Body** or **Expression** or until it is no longer needed.
- Referenced by nothing.
- References **Function**.
- Owned by its parent **Body** or **Expression**.
- Owns **ArgumentConstant**.

**MacroBody**:

- A Body for a macro invocation.
- Lives for the lifetime of its parent **Body** or until it is no longer needed.
- Referenced by nothing.
- References **Macro**.
- Owned by its parent **Body**.
- Owns **ArgumentBody** (for the arguments of the macro invocation).