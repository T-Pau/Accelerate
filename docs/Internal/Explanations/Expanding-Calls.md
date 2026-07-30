# Expanding Macro and Function Calls

The **Macro** or **Function** contains **EntityConstant**s pointing to **ArgumentExpression** placeholders for the argument names. The names inside the **Body** of the **Macro** or **Expression** of the **Function** refer to those.

The default arguments are wrapped in **EntityConstants**.

Before the expansion:

**MacroBody** owns the provided arguments, which are a vector of **Expression**.

After the expansion:

**MacroBody** is replaced with a **ScopeBody**. The provided arguments are **ArgumentConstant** in the **ScopeBody**. The default arguments are **EntityConstant** in the **Macro**.

During Expansion:

1. The **ScopeBody** is created. 

2. The provided arguments are wrapped in **ArgumentConstant** and added as constants to the **ScopeBody**. The **ArgumentExpression** placeholders in the **Macro** are mapped to the corresponding **ArgumentConstant** (for provided arguments) or **EntityConstant** (for default arguments) in the **CloneContext**.

3. The body of the **Macro** is cloned and set as the body of the **ScopeBody**, replacing the argument references with the corresponding **ConstantExpression** via the **CloneContext** mapping.

4. The **ScopeBody** is returned as the result of the expansion.

