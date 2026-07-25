# Ownership of Data Structures

<!-- --8<-- [start:summary] -->
Accelerate has complex relationships between its data structures.  This article explains the lifetimes and ownership relationships of the main data structures.
<!-- --8<-- [end:summary] -->

## What Can Own What

This section breaks the classes down into their use cases so we can reason about their relationships to other classes, making sure there are no ownership cycles, and they are not destroyed while they are still needed. This problem is too big to hold in my head at once, so I will break it down into smaller pieces. 

### Module

**Linker**:

- Responsible for translating the program and producing the output.
- Lives forever.
- Referenced by nothing within the ownership tree.
- References **Module**, **ExternalConstant**, **InternalConstant**, **Object**.
- Root of ownership tree.
- Owns **Module**.


**Module**:

- The main program, target, or an imported library.
- Lives forever.
- Referenced by **Linker**.
- References
- Owned by **Linker**.
- Owns **VisibleScope**, **ScopeEntity**, **ExternalConstant**, **Object**, **Function**, **Macro**, **ExternalConstant**.

### Scopes

**VisibleScope**:

- A Scope of type `PUBLIC`, `PRIVATE`, or `FILE`. 
- Lives forever.
- Referenced by **Module**, **VisibleScope**, **EntityScope**.
- References **ExternalConstant**, **Object**, **Function**, **Macro**.
- Owned by **Module**, contained **VisibleScope** or **EntityScope**.
- Owns parent **Scope**.


**EntityScope**:

- A Scope of type `ENTITY`. 
- Lives forever.
- Referenced by **Object**, **Macro**.
- References **EntityConstant**.
- Owned by **Object**, **Macro**, contained **ArgumentScope**.
- Owns parent **Scope**.

**ArgumentScope**:

- A Scope of type `ARGUMENT`.
- Lives for the lifetime of its containing **ArgumentBody**.
- Referenced by **ArgumentBody**.
- References **ArgumentConstant**.
- Owned by **ArgumentBody**, contained **ArgumentScope**.
- Owns parent **Scope**.


### Entities

**Object**:

- An object.
- Lives forever.
- Referenced by **VisibleScope**, **ObjectExpression**.
- References **EntityScope**, **InternalConstant**, **Body**, **Expression**.
- Owned by **Module**.
- Owns **EntityScope**, **InternalConstant**, **Body**, **Expression**.

**Macro**:

- A macro.
- Lives forever.
- Referenced by **VisibleScope**, **MacroBody**.
- References **EntityScope**, **InternalConstant**, **Body**, **Expression**.
- Owned by **Module**.
- Owns **EntityScope**, **InternalConstant**, **Body**, **Expression**.

**Function**:

- A function.
- Lives forever.
- Referenced by **VisibleScope**, **FunctionExpression**.
- References **Expression**.
- Owned by **Module**.
- Owns **Expression**.

**ExternalConstant**:

- A constant that is defined outside an **Object** or **Macro**.
- Lives forever.
- Referenced by **VisibleScope**, **ExternalConstantExpression**.
- References **Expression**.
- Owned by **Module**.
- Owns **Expression**.

**EntityConstant**:

- A constant that is defined inside an **Object** or **Macro**.
- Lives forever.
- Referenced by **EntityScope**, **EntityConstantExpression**.
- Owned by **Object**, **Macro**.
- Owns **Expression**.

**ArgumentConstant**:

- A constant that is defined inside an **ArgumentScope**.
- Lives for the lifetime of its containing **ArgumentScope**.
- Referenced by **ArgumentScope**, **ArgumentConstantExpression**.
- Owned by **ArgumentScope**.
- Owns **Expression**.


### Bodies and Expressions



Generally, Bodies and Expressions form tree structures. A Body or Expression is owned by its containing Body or Expression, owns the contained Bodies and Expressions. They may share subtrees (after cloning). They cannot contain cycles, are referenced only by their container, reference only their contained Bodies and Expressions, and are destroyed when their container no longer needs them.

Some types of Bodies and Expressions refer to other classes. These are listed here:

**ArgumentBody**:
- A Body for a macro call.
- Lives for the lifetime of its containing **Body** or as long as any of its **ArgumentConstants** are needed.
- Referenced by its containing **Body**.
- References **ArgumentScope**, **ArgumentConstant**.
- Owned by its containing **Body**.
- Owns **ArgumentScope**, **ArgumentConstant**.

**ObjectExpression**:
- An expression that refers to an **Object**.
- Lives for the lifetime of its containing **Body** or **Expression**.
- Referenced by its containing **Body** or **Expression**.
- References an **Object**.
- Owned by its containing **Body** or **Expression**.
- Owns nothing.

**ExternalConstantExpression**:
- An expression that refers to an **ExternalConstant**.
- Lives for the lifetime of its containing **Body** or **Expression**.
- Referenced by its containing **Body** or **Expression**.
- References an **ExternalConstant**.
- Owned by its containing **Body** or **Expression**.
- Owns nothing.

**InternalConstantExpression**:
- An expression that refers to an **InternalConstant**.
- Lives for the lifetime of its containing **Body** or **Expression**.
- Referenced by its containing **Body** or **Expression**.
- References an **InternalConstant**.
- Owned by its containing **Body** or **Expression**.
- Owns nothing.

- **ArgumentConstantExpression**:
- An expression that refers to an **ArgumentConstant**.
- Lives for the lifetime of its containing **Body** or **Expression**.
- Referenced by its containing **Body** or **Expression**, which is inside the **ArgumentBody** that owns the referenced **ArgumentConstant**.
- References an **ArgumentConstant**.
- Owned by its containing **Body** or **Expression**.
- Owns **ArgumentConstant**. (This is used by the **ArgumentBody** to determine if the **ArgumentConstant** is still needed.)

Cycles:

ExternalConstant -> Expression -> ConstantExpression -> ExternalConstant

✓ Body and Expression have a tree structure. They own their children. They may share subtrees (after cloning). This cannot create circles.

✓ Scopes have a tree structure. They own their parent scopes. This cannot create circles.

Lifetime:



## Lifecycle

### Module

A module is created when the source code is parsed. It is destroyed when the program exits. 

### Scope

A scope is created when its module or entity is parsed, or when its containing BodyElement is created.

#### `PUBLIC`, `PRIVATE`, and `FILE` Scopes 

These are destroyed when the program exits, since they are owned by the module.

They can contain Constants, Objects, Functions, and Macros that can be referred to by other entities. Since these are owned by their scopes, they are destroyed when the program exits.

#### `ENTITY` Scopes

They are destroyed when their containing entity is destroyed. 

These can only contain Constants which are only visible within the entity, so they should not be needed longer than the entity itself.

!!! todo
    Investigate if an assignment with wider visibility can cause a constant to escape its `ENTITY` scope, especially when used in an assignment with wider visibility.

#### `SCOPE` Scopes

These are destroyed when their containing BodyElement is destroyed. 

These can only contain Constants which are only visible within the containing BodyElement, so they should not be needed longer than the BodyElement itself.

!!! todo
    Investigate if entities can escape their `ENTITY` or `SCOPE` scope, especially when cloned during call expansion.

### Entity

An entity is created when it is parsed, and placed in its containing scope. 

Entities in `PUBLIC`, `PRIVATE`, and `FILE` scopes are destroyed when the program exits.

Entities in `ENTITY` scope are destroyed when their containing entity is destroyed. Entities contained in these are only visible within the entity, so they should not be needed longer than the entity itself.

Entities in `SCOPE` scope are destroyed when their containing BodyElement is destroyed. Entities contained in these are only visible within the containing element, so they should not be needed longer than the containing element.

Care should be taken not to destroy BodyElements while entities within their `SCOPE` scope are still needed. If elements co-own the referenced entities, a scope can check if any of its entities are still needed.

### BodyElement and Expression

A BodyElement or Expression is created when it is parsed, and placed in its containing BodyElement or Expression.

A BodyElement or Expression is destroyed when its containing BodyElement or Expression is destroyed. Cloning may cause non-mutating elements to be shared between different parent elements, in which case they are destroyed when the last element containing them is destroyed.

An element referencing an entity needs that entity alive while it is alive. If it co-owns it, that would guarantee it. However, the entity's scope might not be (depending on the ownership model).

!!! todo
    Investigate when the entity's scope is needed after the resolve phase. During the resolve phase, no scopes are destroyed.

Try: entities are owned by containing module/object.


## Version 1: Backwards Ownership of Scopes

!!! warning
    This model has a serious problem, see Entity below.

### Module

A module represents the main program, the target, or an imported library.

It owns its public and private scopes, as well as all file scopes.

### Scope

A scope represents a place where names can be defined. Its visibility determines where the names are visible. A scope can be public, private, or file, entity, or scope.

A scope co-owns the scope that contains it. This way, the lookup chain is preserved. It has weak pointers to imported scopes. (Note: Maybe these should also be shared pointers?)

It also co-owns the entities it contains. This way, entities are kept alive as long as they can be looked up.

It has weak pointers to the scopes it contains. This allows scope name path traversal. The destructor of a scope will remove it from the containing scope's list of contained scopes; if this fails, the weak pointer acts as a fail-safe mechanism to prevent dangling pointers.

### Entity

An entity represents an constant, object, function, or macro. 

Objects, functions, and macros have their own scope of visibility `ENTITY`. There is no other data structure suited to take ownership of this scope. But if the entity owns it, that creates an ownership cycle: scope -> entity -> entity scope -> parent scope, which is the original scope.

However, this only happens for objects that are destroyed when the program exits, since only constants can be contained in `ENTITY` scopes.


## Version 2: Forward Ownership of Scopes

### Module

A module represents the main program, the target, or an imported library.

It owns its public scope, which in turn owns its private scope, which in turn owns its file scopes.

For convenience, it keeps shared pointers to all of its scopes, so that they can be accessed directly. Since the module and its public scope are deallocated at the same time, so are both copies of these shared pointers.

### Scope

A scope represents a place where names can be defined. Its visibility determines where the names are visible. A scope can be public, private, or file, entity, or scope.

It co-owns the entities it contains. This way, entities are kept alive as long as they can be looked up.

A scope owns the scopes that it contains. This is not the natural way, since contained scopes are often deallocated before the containing scope. If that is required, the contained scope must remove itself from the containing scope explicitly.

### Entities

An entity represents an constant, object, function, or macro.

Objects, functions, and macros have their own scope of visibility `ENTITY` that they own. The containing scope owns both the entity and its scope, so there is no ownership cycle. If an entity should be removed from its containing scope, it must remove its scope from the containing scope explicitly. In practice, this does not happen, since only constants are destroyed before program exit, and they do not have a scope.


## Version 3: No Ownership between Scopes

### Module

A module represents the main program, the target, or an imported library.

It owns its public and private scopes, as well as all file scopes.

### Scope

A scope represents a place where names can be defined. Its visibility determines where the names are visible. A scope can be public, private, or file, entity, or scope.

It co-owns the entities it contains. This way, entities are kept alive as long as they can be looked up.

A scope has weak pointers to the scopes it contains. This allows scope name path traversal. The destructor of a scope will remove it from the containing scope's list of contained scopes; if this fails, the weak pointer acts as a fail-safe mechanism to prevent dangling pointers.

A scope has weak pointers to the scope that contains it. If the containing scope is deallocated, the lookup chain is broken. Investigation if this is a problem is required.

### Entities

An entity represents an constant, object, function, or macro.

Objects, functions, and macros have their own scope of visibility `ENTITY` that they own. Since no one else co-owns this scope, there is no ownership cycle. When an entity is deallocated, it explicitly removes its scope from the containing scope. In practice, this does not happen, since only constants are destroyed before program exit, and they do not have a scope.


### Body Elements and Expressions

Body elements and Expressions are owned by the entity, body element, or expression that contains them. 

Elements that reference an entity have a weak pointer to that entity. If the entity is deallocated, the reference is broken. This should throw an exception. Since an entity can contain an element that references it, shared pointers cannot be used here.

