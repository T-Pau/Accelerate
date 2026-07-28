# Expanding Macro and Function Calls

Before the expansion:

**MacroBody** owns the provided arguments, which are a vector of **Expression**. **Macro** owns the default arguments, which are **EntityConstant**.

After the expansion:

**MacroBody** is replaced with a **ScopeBody**. The provided arguments are **ArgumentConstant** in the **ScopeBody**. The default arguments are **EntityConstant** in the **Macro**.

During Expansion:

1. The **ScopeBody** is created. 

2. The provided arguments are wrapped in **ArgumentConstant** and added as constants to the **ScopeBody**.

3. The **ArgumentExpression** placeholders in the **Macro** are set to the corresponding **Constant**.

4. The body of the **Macro** is cloned and set as the body of the **ScopeBody**. Cloning an **ArgumentExpression** within will replace itself with the corresponding **ConstantExpression**.

5. The arguments are cleared in the **ArgumentExpression** placeholders in the **Macro**.

6. The **ScopeBody** is returned as the result of the expansion.

