# forgelang - Consolidated Reference

## Overview

`forgelang` is a compiled language with a C code generator backend.
The compiler itself is written in C, ported from a prior D implementation.
The pipeline is fully end-to-end and working.

---

## Compiler Pipeline

```
Source text
  → Tokenizer (lexer)
  → Recursive descent parser
  → AST
  → C code generator
  → Emitted C source
```

All three stages are implemented and functional.

---

## Build System

Custom `nob.c` build system (Tsoding-style). Compiler written in C.

| Flag      | Effect |
|-----------|--------|
| `--debug` | Debug build |
| `--cold`  | Full rebuild (bypass incremental cache) |

Incremental: only changed translation units are recompiled.

```sh
./nob                # incremental, release
./nob debug          # incremental, debug
./nob cold           # full rebuild, release
./nob cold debug     # full rebuild, debug
```

---

## Language Properties

- Compiled, not interpreted
- C as compilation target (forgelang → C → native binary via gcc/clang)
- Statically typed (implemented or in active progress)
- Originally written in D, ported to C

---

## Tokenizer

- Lexer pass over raw source text
- Produces a token stream consumed by the parser
- `strdup` used for identifier values - each token owns its string

---

## Parser

- Recursive descent
- Arena allocator for all AST nodes (4 MiB arena)
- Exits on first error (recovery deferred - *revisit later*)

---

## Code Generator

- Target: human-readable C source
- Takes AST, emits valid C
- Style: readable and structured, without modern C++ complexity
- GCC `__` internal types used for integer backing (intentional)

---

## TokenType Enum

All tokens and types are integer tags - no string comparison for identity.
Keep in mind it is probably long outdated.

```c
typedef enum {
   exit_,
   int_lit,
   string_lit,
   semi,
   colon,
   d_quote,
   s_quote,
   plus,
   minus,
   times,
   slash,
   print,
   ident,
   open_paren,
   close_paren,
   return_,
   let,
   equals,
   // types
   i8_ , char_,   // __INT8_TYPE__
   i16_,          // __INT16_TYPE__
   i32_,          // __INT32_TYPE__
   i64_,          // __INT64_TYPE__
   u8_ , uchar_,  // __UINT8_TYPE__
   u16_,          // __UINT16_TYPE__
   u32_,          // __UINT32_TYPE__
   u64_,          // __UINT64_TYPE__
   f32_,          // float
   f64_,          // double
   uptr,
   bool_,
   string,
   ustring,
   TERMINATE
} TokenType;
```

`TERMINATE` is a sentinel. Type tokens are contiguous at the end - range checks like `tok >= i8_ && tok <= ustring` are valid.

---

## Type System

### Primitive Types

| Forgelang | C backing |
|-----------|-----------|
| `i8` / `char` | `__INT8_TYPE__` |
| `i16` | `__INT16_TYPE__` |
| `i32` | `__INT32_TYPE__` |
| `i64` | `__INT64_TYPE__` |
| `u8` / `uchar` | `__UINT8_TYPE__` |
| `u16` | `__UINT16_TYPE__` |
| `u32` | `__UINT32_TYPE__` |
| `u64` | `__UINT64_TYPE__` |
| `f32` | `float` |
| `f64` | `double` |
| `bool` | - |
| `uptr` | - |

### String Types

| Type | Description |
|------|-------------|
| `string` | 8-byte fat pointer: bits 0–44 = pointer to first `char`, bits 45–47 = flags, bits 48–63 = length (max 65535) |
| `ustring` | Same layout, elements are `uchar` |

- Single 64-bit value, pointer and length bitpacked
- Opaque to the forgelang programmer
- `string str = "Hello, World!";` emits `malloc` + `strlen` in C output
- Bit 45: reserved for mut/immut flag
- Bit 46: reserved for interned flag (points to static memory, skip free)
- Bit 47: currently free

Will consider switching to a pascal string or to a simple struct based string

### Pointers

C-style syntax:
```
let x: i32 = 10;
let x_ptr: *i32 = &x;   // & = address-of, * = pointer type
let val: i32 = *x_ptr;  // * = dereference
```

### References

Distinct from raw pointers. Guaranteed non-null and non-dangling.
- Syntax: `&T` in type position, unambiguous from `&x` (address-of) in expression position
- **Strict mode**: compiler-enforced
- **Relaxed mode**: documented promise, not enforced
- No pointer arithmetic on references

### Arrays

D-style syntax:
```
i32[]       arr;   // dynamic array
i32[10]     arr;   // static array
i32[10][20] arr;   // 2D static array
i32[string] map;   // associative array / hash map - not yet decided
```

### Optional

No null. Absence expressed as `optional!T` - value of type `T` or nothing.
Unwrapped with `match`.

---

## AST Node Taxonomy - Current State

### Expression nodes
| Tag | Struct | Status |
|-----|--------|--------|
| `EXPR_INT_LIT` | `NodeExprIntLit` | Working |
| `EXPR_IDENT` | `NodeExprIdent` | Working |
| - | `BinExpr` (`EXPR_ADD`, `EXPR_MULTI`) | Defined, not wired into `parse_expr` yet (TODO: Pratt parsing) |

### Statement nodes
| Tag | Struct | Status |
|-----|--------|--------|
| `STMT_LET` | `NodeStmtLet` | Working |
| `STMT_EXIT` | `NodeStmtExit` | Working |
| `STMT_ASSIGN` | `NodeStmtAssign` | Working |

### Program node
`NodeProg` - flat darray of `NodeStmt`. Parse loop runs until `TERMINATE`.

---

## Symbol Table

### Structure
Two sets of per-type partitioned darrays:
- **Variables**: one darray per type (10–20 tables max)
- **Consts**: separate per-type darrays; entries carry an additional value field

N-table bound is a compile-time constant - name lookup across all tables is effectively O(n) over entries.

### Entry Structure
- Identifier name
- `TokenType` tag (next TODO)
- Value field for compile-time constants only

### Lookup
- Now: linear scan
- Future: hand-rolled hash map in C, or deferred to the forgelang rewrite

### Collision Policy
Duplicate identifiers are a hard error. No shadowing, no ambiguity.

---

## Syntax

### Comments
```
// line comment
/* block comment */
```

### Variable Declaration & Mutability
```
let x: i32 = 5;       // immutable by default
let mut y: i32 = 5;   // mutable
```
Future consideration: rename `let` to `const` for immutable bindings.

### Functions
```
fn add(a: i32, b: i32) -> i32 {}
fn main() {}   // entry point, void
```

### Conditionals
```
if (x > 0) return x;       // parens, single statement
if (x > 0) { return x; }   // parens, braces
if x > 0 { return x; }     // no parens - braces mandatory
```
`if` / `else if` / `else`. Parens optional; no parens requires braces.

### Loops
Single construct: `for`. No `while`, no `do while`.
```
for {}                            // infinite loop
for (init; condition; post) {}    // C-style
for (i in length(str)) {}        // index iteration
for (c in str) {}                 // element iteration
```
Parens optional on forms 2–4; no parens requires braces.
`break` and `continue` are separate keywords.

### Structs
```
struct Foo {
    public:
        x: i32,
    private:
        y: i32,

    public fn bar() {}
    private fn baz() {}
}
```
- Fields and methods inside the struct block
- Visibility: label style (`public:`) or per-item keyword
- Next token determines kind: `fn` = method, `:` = field
- Default visibility: **public**
- Constructors and destructors: supported
- Inheritance: not planned
- Interfaces vs traits: deferred - both candidates

### Enums
C-style extended with optional per-variant data (tagged unions):
```
enum Shape {
    Point,
    Circle(f32),
    Rectangle(f32, f32),
}
```

### Pattern Matching
```
match shape {
    Circle(r) => ...,
    Rectangle(w, h) => ...,
    Point => ...,
}
```

### Type Aliases
```
type Meters = i32;
```

### Compile-Time Parameters
`!` is the universal compile-time parameter syntax. Replaces C++'s `<>` which I heavily dislike.
```
optional!T
max!i32(a, b)
```

### Casting
C-style: `(i32)x`. No named casts unless a specific need arises.

### Modules / Exports
```
export ModuleName {
    MyStruct,
    some_fn,
}

import ModuleName
```
Everything not exported is file-private. Compiler resolves module location by name.

### Inline Assembly
Isolated to dedicated functions only. Intel syntax.
```
inline asm fn foo() {
    mov rax, 4
    ...
}
```
Standard functions can be overridden with `override` keyword. Scope of override (global vs local) TBD.

---

## Runtime Behavior & Language Semantics

### Immutability
`let` immutability is compiler-enforced - assigning to an immutable binding is a compile-time error.

### Memory Layout
- Structs are stack-allocated by default
- Explicit heap allocation available when needed
- No implicit heap allocation

### Safety & Undefined Behavior
- `[]` indexing has bounds checking by default
- `.at()` is raw unchecked access - explicit opt-out
- Integer overflow exits instead of wrapping/underflowing - surfaces bugs rather than silently corrupting data
- `isize` (signed size type) available alongside `uptr` for cases where negative values are needed (avoids unsigned underflow traps with `size_t`)
Constructors and destructors on structs. Destructors are **explicit** - not called automatically at end of scope.

### Defer
Executes a statement at end of enclosing scope regardless of exit path:
```
let f = open_file("foo.txt");
defer close_file(f);
```

### Move Semantics / Ownership
Rust-like with a D-inspired strictness flag:
- **Relaxed mode** (default) - ownership rules loosened for experimentation
- **Strict mode** (`--strict` or similar) - full move/borrow checking enforced

### Operator Overloading
No user-defined operator overloading. Built-in types only (e.g. `string + string`, `&`).

### Memory Management
Default allocator: arena. Otherwise manual.
Global allocator context planned:
- `Allocator.push()` - stack-based, scope-swappable
- `Allocator.set(ARENA_ALLOCATOR, SIZE)` - explicit global set

Default arena size TBD after testing (1–10 MiB range).

---

## Standard Library

### Included by Default
- `stdio` - print, file I/O, `string` type and its methods, no import needed

### `std`
- Default arena allocator
- Array/collection functions: `map`, `filter`, `reduce` (Haskell-inspired)
- `quick_sort` - only built-in sort; additional sorts are opt-in includes

### Additional Includes (opt-in)
- Other allocators
- Additional sort algorithms
- Other utilities TBD

### String Methods
`string` has methods (`str.length()`, `str.split()`, etc.). Specific method set TBD - expanding as needed rather than upfront.

### Freestanding Mode
A flag (e.g. `--freestanding`) strips all libc dependencies for bare metal/kernel/embedded targets.

### `char*`
Not encouraged. `string` is the intended type for all string handling.

---

Two options under consideration:
- **Implicit error channel** - hidden error return on every function, invisible in signature
- **Explicit multiple return values** - `fn foo() -> i32, Error`; failure visible in signature

`main` is void. C codegen will likely emit a struct for multi-return pairs.

---

## Self-Hosting

Final goal: forgelang compiles itself. Current C implementation is a bootstrap vehicle. GCC `__` type dependencies resolved at rewrite time.

---

## Open / Deferred

- Error recovery strategy - deferred until syntax stabilizes *(revisit later)*
- AST node taxonomy - expands as syntax is decided *(revisit later)*
- `std` API and scope - not yet defined
- Interfaces vs traits - both candidates, decision pending
- Generics - Jai-style (`!` syntax), simple, non-recursive; deferred
- Lambdas / function literals - pure and capturing; syntax TBD; deferred
- Comptime / compile-time execution - long-term goal
- Concurrency - threads confirmed; async/await and actors noted for future
- Memory management allocator API and default arena size - TBD
- Error handling strategy - undecided
- Hash map syntax (`i32[string]`) - not yet decided
