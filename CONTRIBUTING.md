# Contributing to neml2-hit

## Setting up your development environment

### 1. Clone and configure a Debug build

Flex ≥ 2.6 and Bison ≥ 3.7 are required for Debug builds; they regenerate
`generated/Lexer.cpp` and `generated/Parser.cpp` from source.

```bash
git clone <repo-url>
cd neml2-hit
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### 2. Set up the Python bindings (optional)

If you are working on `python/src/_nmhit.cpp` or `python/nmhit/__init__.py`,
install the package in editable mode after the CMake build:

```bash
pip install scikit-build-core nanobind pytest   # build deps + test runner
pip install -e . --no-build-isolation -v        # editable install
pytest python/tests/ -v                         # run Python tests
```

`--no-build-isolation` skips the PEP 517 build venv and uses the already-installed
`scikit-build-core` and `nanobind` packages directly, which avoids re-downloading them.
scikit-build-core places its own CMake build in `_skbuild/` (separate from any `build/`
you may have created for C++ development).
Re-run `pip install -e . --no-build-isolation -v` whenever you change the C++ binding source.

### 3. Install the pre-commit hook

The project uses [pre-commit](https://pre-commit.com) to run `clang-format`
automatically before every commit.  Install it once after cloning:

```bash
pip install pre-commit   # or: conda install pre-commit
pre-commit install
```

From that point on, `clang-format` runs automatically on staged `*.cpp` and
`*.h` files under `src/`, `include/`, and `tests/`.  Generated files in
`generated/` and the Flex/Bison sources (`src/Lexer.l`, `src/Parser.y`) are
intentionally excluded — no formatter exists for those file types.

To run the hook manually across the whole tree:

```bash
pre-commit run --all-files
```

---

## Development workflow

### Editing C++ source only

Make changes to `src/Node.cpp`, `src/BraceExpr.cpp`, `src/ParseDriver.h`,
the public headers under `include/`, or the Python binding source at
`python/src/_nmhit.cpp`, then rebuild:

```bash
cmake --build build -j$(nproc)
./build/tests/test_hit
```

### Editing the lexer or parser

`src/Lexer.l` (Flex) and `src/Parser.y` (Bison) require a Debug build.  After
editing either file, the build system regenerates the corresponding sources
automatically:

```bash
cmake --build build -j$(nproc)   # flex/bison run here
./build/tests/test_hit
```

Once the tests pass, copy the regenerated files back to `generated/` so that
non-Debug (Release) builds remain self-contained without requiring flex or bison:

```bash
cmake --build build --target update_generated
git add generated/
```

Commit the grammar/lexer source and the regenerated files together in the same
commit so the repository is always consistent.

---

## Running the tests

```bash
./build/tests/test_hit          # direct binary
# or via CTest:
ctest --test-dir build --output-on-failure
```

---

## Code style

`clang-format` (version 19) enforces the style defined in `.clang-format`.
The pre-commit hook applies it automatically; you can also run it by hand:

```bash
clang-format -i src/Node.cpp src/BraceExpr.cpp src/ParseDriver.h \
             include/nmhit/*.h tests/test_hit.cpp python/src/_nmhit.cpp
```
