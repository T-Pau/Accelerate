# Body Elements and Expressions

<!-- --8<-- [start:summary] -->
This article explains the Body and Expression class hierarchies: How to use them, and how to implement subclasses.
<!-- --8<-- [end:summary] -->

## Implementing Translation Methods

The translation methods default implementation use the `traverse()` method to recursively call the translation methods on the sub-elements and sub-expressions.  Most subclasses will override this method.

For Expressions:

```cpp
void traverse(std::function<void(Expression&)> callable) override {
    callable(sub_expression_1);
    callable(sub_expression_2);
}
```

For Body elements:

```cpp
void traverse(std::function<void(Body&)> body_callable, std::function<void(Expression&)> expression_callable) {
    body_callable(sub_element_1);
    body_callable(sub_element_2);
    expression_callable(expression_1);
    expression_callable(expression_2);
}
```

If a subclass needs special handling for any of the translation methods, it can override the relevant method directly.

The `evaluate()` method is a special case: Most subclasses need special processing after all sub-expressions have been evaluated.  The default implementation of `evaluate()` calls `traverse()` to evaluate all sub-expressions, and then calls the subclass's `evaluate_process()` method.

## Translation Methods

### `enter_names()`

This method is only available for Body elements.  It is called right after the body has been parsed and should enter all names defined in the body into the current scope. If a body element does not define any names, it doesn't need to override this method.

### `resolve()`

This method is called after all names have been entered into the current scope.  It should resolve all names used in the body or expression.  If a body element or expression does not use any names, it doesn't need to override this method.

This method also returns all referenced entities.

### `evaluate()`

All referenced entities will have been evaluated before this method is called, making sure that their evaluation will be propagated.

This method is called multiple times at different stages of the translation.  It should evaluate the body or expression as much as possible. Most subclasses will override the `evaluate_process()` method instead of this method.

### `expand_calls()`

This method should expand all macro and function calls, replacing them with the body of the macro or function, and providing their arguments.