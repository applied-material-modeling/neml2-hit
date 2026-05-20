"""Command-line entry points for nmhit.

Currently provides a single command, ``nmhit-format``, which reparses each
input file with nmhit and writes the canonical render back in place. The
intended use is as a pre-commit hook on NEML2-flavored HIT input files.

Dialect opt-in
--------------

nmhit can parse plain MOOSE-HIT as well as NEML2-flavored HIT. To distinguish
the two without ambiguity, NEML2 input files declare their dialect by writing
the line ``# neml2`` as the first non-blank line of the file. The neml2-langserv
VS Code extension uses the same marker to decide whether to attach. The
formatter follows suit: files whose first non-blank line is not ``# neml2``
are silently skipped, even if they match the hook's ``files:`` pattern.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from . import parse_file

DIALECT_MARKER = "# neml2"


def _is_neml2(path: Path) -> bool:
    """Return True iff the first non-blank line of *path* is exactly the
    NEML2 dialect marker.
    """
    try:
        with path.open() as f:
            for line in f:
                stripped = line.strip()
                if not stripped:
                    continue
                return stripped == DIALECT_MARKER
    except OSError:
        return False
    return False


def format_files(argv: list[str] | None = None) -> int:
    """Entry point for ``nmhit-format``.

    Reparses each NEML2-marked input file and rewrites it in place if the
    canonical render differs from the original. Returns 1 when any file was
    modified (so the script exits non-zero under pre-commit, signalling that
    a reformat happened and the commit should be retried), 0 otherwise.
    """
    parser = argparse.ArgumentParser(
        prog="nmhit-format",
        description=(
            "Reformat NEML2-dialect HIT input files in place. Files without "
            "the `# neml2` dialect marker on their first non-blank line are "
            "skipped silently."
        ),
    )
    parser.add_argument("files", nargs="+", help="HIT input files to format")
    args = parser.parse_args(argv)

    changed = 0
    for arg in args.files:
        path = Path(arg)
        if not _is_neml2(path):
            continue
        src = path.read_text()
        new = parse_file(path).render()
        if new != src:
            path.write_text(new)
            print(f"reformatted {arg}", file=sys.stderr)
            changed += 1
    return 1 if changed else 0


if __name__ == "__main__":
    raise SystemExit(format_files())
