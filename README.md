# neml2-hit

A standalone C++17 parser for a **NEML2-flavored dialect of HIT** (Hierarchical Input Text) —
the hierarchical input format used by [MOOSE](https://mooseframework.inl.gov).  This library
provides a self-contained, opinionated implementation tailored for use in
[NEML2](https://github.com/applied-mech/neml2) and related projects.  It differs from the
upstream MOOSE HIT parser in syntax restrictions and API design choices; it is not a
general-purpose drop-in replacement.  The library depends on Flex & Bison.

The public C++ namespace is `nmhit` ("NEML2 HIT").

---

## HIT Format

HIT is a simple, human-readable format for hierarchical configuration.  A file is a flat sequence
of items — sections, key-value fields, comments, blank lines, and file includes — which together
form a tree.

---

## Syntax Reference

### Comments

A `#` character begins a comment that extends to the end of the line.  Comments are preserved in
the AST and are reproduced by `render()`.

```
# This is a comment
key = value  # inline comments are not supported; this text is part of the value
```

> **Note:** `#` is a reserved character in all value positions.  It cannot appear inside an
> unquoted string value or an array element.

### Blank lines

One or more consecutive blank lines are preserved as a single `Blank` node and are reproduced by
`render()`.

### Sections

A section groups related fields and nested sub-sections.

```
[section_name]
  key = value
  nested_key = 42
[]
```

Every section must be closed with `[]`.  There is no `[../]` or `[./name]` syntax.

**Path splitting.** A slash in the section header creates the corresponding nesting in the AST:

```
[mesh/generator]
  type = CartesianMesh
[]
```

is equivalent to:

```
[mesh]
  [generator]
    type = CartesianMesh
  []
[]
```

Sections may appear at the top level or nested inside other sections.  Fields and nested sections
can appear in any order within a section body.

### Fields

A field assigns a value to a name.

```
key = value
```

**Identifier characters.** A field name may contain letters, digits, and any of
`. / : < > + - * ! _`.  Slashes in a field name trigger path splitting (see below).

**Path splitting.** A slash in the field name creates intermediate `Section` nodes in the AST:

```
[solver]
  linear/max_iter = 100
[]
```

is equivalent to:

```
[solver]
  [linear]
    max_iter = 100
  []
[]
```

**Override assignment.** The operators `:=` and `:override=` are both accepted.  The library
implements **last-override-wins** semantics directly: the earlier occurrence of the field is
removed from the tree, leaving only the overriding value.

```
max_iter := 200
max_iter :override= 200   # identical meaning
```

### Values

Every value is one of the following kinds.

#### Integer

An optional sign followed by one or more decimal digits.

```
n = 42
n = -7
n = +0
```

#### Floating-point number

Standard decimal notation with an optional sign and optional exponent.

```
x = 3.14
x = -1.0e-3
x = .5
x = 2.
x = 1e10
```

At least one digit must appear on one side of the decimal point, or an exponent must be present.

The value is stored verbatim as a string.  At interpretation time:

- `param<double>()` parses it as a 64-bit IEEE 754 double-precision value.
- `param<float>()` parses it as `double` first, then narrows to 32-bit single precision.
  Values outside the `float` range become `±inf`; values that are representable in `double`
  but not exactly in `float` are rounded to the nearest `float`.

#### Boolean

Exactly the two lowercase literals `true` and `false`.  No other strings
(including `yes`, `no`, `on`, `off`, or any capitalised variant) are accepted.

```
flag = true
flag = false
```

#### Unquoted string

Any sequence of non-whitespace characters that does not begin a number, boolean, quoted string,
array, or brace expression, and contains none of `` [ # $ ' " \ ``.

```
type = GeneratedMesh
label = some_label
path = /usr/local/share
```

Unquoted strings are **single-line only** — they cannot contain whitespace or newlines.

#### Array (1-D)

A whitespace-delimited sequence of elements enclosed in single quotes **or double quotes** —
both delimiters are completely equivalent.  Elements may be integers, floating-point numbers, or unquoted tokens
(none of which may contain `;`, `#`, `$`, `'`, `"`, or `\`).

```
vals   = '1 2 3'
floats = '1.0 2.5 3.14'
tags   = 'alpha beta gamma'
```

The two quote styles are interchangeable:

```
vals = '1 2 3'
vals = "1 2 3"   # identical meaning
```

An empty array is written as `''` or `""`.

Array contents **may span multiple lines** — newlines inside the quotes are treated as whitespace:

```
vals = '
  1 2 3
  4 5 6
'
```

#### Array (2-D)

Rows are separated by `;`.  Each row is a whitespace-delimited sequence of elements, following the
same rules as 1-D array elements.

```
matrix = '1 2 3; 4 5 6; 7 8 9'
```

The semicolons and surrounding whitespace (including newlines) are flexible:

```
matrix = '
  1 2 3;
  4 5 6;
  7 8 9
'
```

Every row must contain at least one element.  Trailing semicolons (an empty last row) are a parse
error.

Accessing a 2-D array value as a 1-D type (e.g. `param<std::vector<int>>`) will fail because the
semicolons are stored as part of the raw value.  Accessing a 1-D array as a 2-D type returns a
single-row result.

#### Brace expressions

A `${...}` expression is expanded at value-extraction time (i.e. when `param<T>()` is called).
The raw token is stored in the AST as-is.

The following built-in commands are supported:

| Expression | Effect |
|---|---|
| `${varname}` | Look up the field at path `varname` from the document root and return its string value. |
| `${replace varname}` | Identical to `${varname}`. |
| `${env VARNAME}` | Substitute the environment variable `VARNAME`. Returns an empty string when unset. |
| `${raw a b c}` | Concatenate all arguments literally: `abc`. |

Brace expressions may be **nested**:

```
prefix = /opt
lib    = ${raw ${prefix} /lib}   # → /opt/lib
```

A brace expression may appear as the sole value of a field:

```
dim = ${mesh/dim}
```

### File inclusion

```
!include relative/or/absolute/path.i
```

The referenced file is parsed recursively and its top-level items are spliced into the AST at the
point of the `!include` directive.  Relative paths are resolved against the directory of the
including file.

---

## Complete Grammar (EBNF)

```ebnf
file        = item* ;
item        = section | field | comment | blank | include ;
section     = '[' path ']' item* '[]' ;
field       = ident ('=' | ':=' | ':override=') value ;
quote       = "'" | '"' ;
value       = integer | float | bool | unquoted_str
            | brace_expr
            | quote array_row (';' array_row)* quote
            | quote quote ;
array_row   = array_elem+ ;
array_elem  = integer | float | unquoted_elem ;
include     = '!include' path ;
comment     = '#' <to end of line> ;
blank       = <two or more consecutive newlines> ;

path        = segment ('/' segment)* ;
segment     = <one or more non-whitespace, non-bracket characters> ;
ident       = [A-Za-z0-9_./<>+\-*!:]+ ;
integer     = [+\-]? [0-9]+ ;
float       = [+\-]? ( [0-9]* '.' [0-9]+ | [0-9]+ '.' [0-9]* ) ([eE] [+\-]? [0-9]+)?
            | [+\-]? [0-9]+ [eE] [+\-]? [0-9]+ ;
            (* stored verbatim; interpreted as double-precision (64-bit IEEE 754) by default,
               narrowed to single-precision (32-bit) when read as float *)
bool        = 'true' | 'false' ;
unquoted_str= [^ \t\n\r\[#$'"\\]+ ;
unquoted_elem=[^ \t\n\r;#$'"\\]+ ;
brace_expr  = '${' <content, brace-depth-tracked> '}' ;
```

---

## C++ API

### Parsing

```cpp
#include "nmhit/nmhit.h"

// Read and parse a HIT file.  Throws nmhit::Error if the file cannot be
// opened or on syntax errors.
std::unique_ptr<nmhit::Node> root = nmhit::parse("my_file.i");
```

`parse()` accepts two optional string vectors for injecting content before and after
the file.  This is useful when an application collects HIT snippets from
command-line arguments and wants them merged with the input file:

```cpp
// Pre-strings are prepended; post-strings are appended.
// All content is parsed as a single document, so ':=' override semantics
// apply globally across pre, main, and post.
std::vector<std::string> cli_overrides = { "solver/max_iter := 200" };
auto root = nmhit::parse("input.i", /*pre=*/{}, cli_overrides);
```

### Reading values

```cpp
// Resolve a slash-separated path and return a typed value.
// Throws nmhit::Error if the path does not exist or the value cannot be converted.
int    n  = root->param<int>("mesh/dim");
double x  = root->param<double>("solver/tol");
bool   on = root->param<bool>("output/enabled");

// Return a default when the path is absent (does not throw).
int n = root->param_optional<int>("mesh/dim", 3);
```

**Built-in scalar types:** `bool`, `int`, `unsigned int`, `int64_t`, `float`, `double`,
`std::string`.

**1-D arrays:** `std::vector<T>` for any built-in or registered scalar T.

**2-D arrays:** `std::vector<std::vector<T>>` for any built-in or registered scalar T.

### Tree navigation

```cpp
// Walk direct children, optionally filtered by node type.
for (nmhit::Node * child : root->children())          { ... }
for (nmhit::Node * child : root->children(nmhit::NodeType::Field)) { ... }

// Find a node by relative path (returns nullptr when absent).
nmhit::Node * n = root->find("mesh/dim");

// Walk upward.
nmhit::Node * parent = n->parent();
nmhit::Node * docroot = n->root();

// Full slash-joined path from the root.
std::string fp = n->fullpath();   // e.g. "mesh/dim"

// Source location.
int line = n->line();
int col  = n->column();
std::string file = n->filename();
```

### Inspecting fields

```cpp
auto * f = dynamic_cast<nmhit::Field *>(root->find("mesh/dim"));
if (f) {
    std::string raw = f->raw_val();    // stored string, e.g. "3" or "'1 2 3'"
    f->set_val("4");                   // replace the stored value
}
```

### Rendering

```cpp
// Render the tree back to HIT text (preserves comments and blank lines).
std::string text = root->render();

// Custom indentation.
std::string text = root->render(0, "    ");  // 4-space indent
```

### Custom types

Register a scalar parser once before any `param<T>()` call:

```cpp
// Registration (e.g. in main() or a static initializer)
nmhit::TypeRegistry::register_parser<MyEnum>(
  [](const std::string & s) -> MyEnum {
    if (s == "linear")    return MyEnum::Linear;
    if (s == "quadratic") return MyEnum::Quadratic;
    throw std::invalid_argument("unknown MyEnum value: " + s);
  }
);

// Usage — all three arities work automatically once T is registered.
MyEnum                          e  = root->param<MyEnum>("order");
std::vector<MyEnum>             v  = root->param<std::vector<MyEnum>>("orders");
std::vector<std::vector<MyEnum>> m = root->param<std::vector<std::vector<MyEnum>>>("order_matrix");
```

The parser receives the unquoted, brace-expanded token string.  Calling `param<T>()` for an
unregistered type throws `nmhit::Error`.

> **Thread safety:** `register_parser` is not thread-safe relative to concurrent `param` calls.
> Register all custom types before spawning threads that call `param`.

### Errors

All errors throw `nmhit::Error`, which is a `std::exception` carrying a vector of
`nmhit::ErrorMessage` (filename, line, column, message).

```cpp
try {
    auto root = nmhit::parse("input.i", text);
} catch (const nmhit::Error & e) {
    for (auto & msg : e.messages)
        std::cerr << msg.str() << '\n';   // "file.i:10:5: unexpected '}'"
}
```

---

## Building

### Requirements

| Tool | Minimum version |
|------|----------------|
| CMake | 3.20 |
| C++ compiler | C++17 |
| Flex | 2.6 |
| Bison | 3.7 |

### Configure and build

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

Pass `-DNMHIT_BUILD_TESTS=OFF` to skip building the test executable.

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

### Install

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/your/prefix
cmake --build build -j$(nproc)
cmake --install build
```

This installs:

| Path | Contents |
|------|----------|
| `<prefix>/lib/libnmhit.a` | Static library |
| `<prefix>/include/nmhit/` | Public headers |
| `<prefix>/lib/cmake/nmhit/` | CMake config files |
| `<prefix>/lib/pkgconfig/nmhit.pc` | pkg-config file |

### Use from an installed location

**CMake `find_package`:**

```cmake
find_package(nmhit REQUIRED)
target_link_libraries(myapp PRIVATE nmhit::nmhit)
```

If the library was installed to a non-standard prefix, point CMake at it:

```bash
cmake -S . -B build -Dnmhit_DIR=/your/prefix/lib/cmake/nmhit
```

**pkg-config:**

```bash
pkg-config --cflags --libs nmhit
```

If the library was installed to a non-standard prefix:

```bash
PKG_CONFIG_PATH=/your/prefix/lib/pkgconfig pkg-config --cflags --libs nmhit
```

### Use as a CMake subdirectory

Add the repository as a subdirectory of your project:

```cmake
add_subdirectory(neml2-hit)
target_link_libraries(myapp PRIVATE nmhit)
```

The `nmhit` target exports `include/` as a public include directory, so
`#include "nmhit/nmhit.h"` works without any additional configuration.

---

## License

This project is a sub-component of [NEML2](https://github.com/applied-material-modeling/neml2)
and is distributed under the same license.
