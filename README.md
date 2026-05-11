# logic_opt

A digital logic expression minimizer written in C++17. Parses Boolean expressions, generates truth tables, and minimizes them using the Quine-McCluskey algorithm with Petrick's method for exact minimum cover.

## Features

- **Lexer & Parser** — Recursive descent parser for Boolean expressions, supporting:
  - AND (`&`), OR (`|`), NOT (`~`), XOR (`^`)
  - Parenthesized sub-expressions for grouping
  - Multi-character variable names (e.g., `A`, `ABC`, `X1`)
  - Constants `0` and `1`
  - Proper operator precedence: NOT > AND > XOR > OR
- **Truth Table Generation** — Evaluates all 2^n input combinations against the parsed AST, with short-circuit evaluation for AND/OR
- **Quine-McCluskey Minimization** — Finds all prime implicants, identifies essential prime implicants, and selects a minimum cover
- **Petrick's Method** — Exact minimum cover selection via Boolean algebra on the prime implicant chart
- **Output Formats** — Both Sum-of-Products (SOP) and Product-of-Sums (POS) forms

## Build

```bash
git clone https://github.com/ligc131415/logic_opt.git
cd logic_opt
mkdir build && cd build
cmake ..
make
```

Dependencies: C++17 compiler, CMake 3.16+, Google Test (for tests only).

## Usage

```bash
./logic_opt "expression"

# Examples
./logic_opt "(A&B)|(A&~B)"
./logic_opt "(A|B)&(~A|C)"
./logic_opt "A ^ B ^ C"
./logic_opt "~A&B | A&C | B&C"
```

### Output

For each expression, the tool prints:
- The input expression
- Extracted variable names
- Full truth table
- Minterm (Σm) and maxterm (ΠM) indices
- Prime implicants and essential prime implicants
- Minimum cover in SOP form
- Simplified POS form

### Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `&`      | Logical AND | `A & B` |
| `\|`      | Logical OR  | `A \| B` |
| `~`      | Logical NOT | `~A`    |
| `^`      | Logical XOR | `A ^ B` |
| `()`     | Grouping    | `(A & B) \| C` |

## Architecture

```
Input String
    ↓
Lexer ───→ Token stream
    ↓
Parser ───→ AST (Abstract Syntax Tree)
    ↓
TruthTable ───→ Truth table, minterms/maxterms
    ↓
Quine-McCluskey ───→ Prime implicants
    ↓
Petrick's Method  ───→ Minimum cover
    ↓
OutputFormatter ───→ SOP / POS expression
```

### Module Layout

| Module | Directory | Responsibility |
|--------|-----------|----------------|
| Parser | `src/parser/` | Lexer (`lexer.h`), AST nodes (`expression.h`), Recursive descent parser (`parser.h`) |
| Evaluator | `src/evaluator/` | Truth table generation from AST (`truth_table.cpp`) |
| Minimizer | `src/minimizer/` | QMC algorithm (`qmc.cpp`), Petrick's method (`petrick.cpp`), Output formatting (`output.cpp`) |
| Tests | `tests/` | Google Test unit tests (128 tests across 9 suites) |

## Tests

```bash
cd build
cmake ..
make
./logic_opt_tests
```

128 unit tests covering:
- Lexer: tokenization of all operators, variables, whitespace, invalid characters
- Parser: AST construction, operator precedence, parenthesization, error cases
- TruthTable: AND/OR/NOT/XOR evaluation, minterm/maxterm extraction
- QMC: implicant merging, coverage, essential prime implicants, minimum cover
- Petrick: term operations, expression simplification, cost calculation
- Output: SOP/POS formatting, edge cases (empty/all/single-variable)

## License

MIT
