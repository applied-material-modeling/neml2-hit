"""
nmhit — Python bindings for the NEML2-flavored HIT parser.

Typical usage::

    import nmhit

    root = nmhit.parse_file("model.i")
    dim = root.param_int("mesh/dim")

    for child in root.children(nmhit.NodeType.Field):
        print(child.path(), child.raw_val())
"""

from __future__ import annotations

from pathlib import Path
from typing import Any, Optional, Sequence, Union

from ._nmhit import (
    # Exception types
    Error,
    ErrorMessage,
    # Enum
    NodeType,
    # Node classes
    Node,
    Root,
    Section,
    Field,
    Comment,
    Blank,
    # Free functions (C++ names preserved, parse_file wrapped below)
    parse_text,
    parse_bool,
    parse_int,
    parse_double,
    parse_float,
)
from ._nmhit import parse_file as _parse_file_cpp


def parse_file(
    path: Union[str, Path],
    pre: Sequence[str] = (),
    post: Sequence[str] = (),
) -> Root:
    """Parse a HIT input file and return the document root.

    Accepts both :class:`str` and :class:`pathlib.Path`.  The optional *pre*
    and *post* lists are HIT snippets prepended / appended before parsing so
    that command-line override semantics (``:=``) apply globally.
    """
    return _parse_file_cpp(Path(path), list(pre), list(post))


def param(node: Node, relpath: str = "", type_hint: Optional[type] = None) -> Any:
    """Retrieve a typed parameter from *node* at *relpath*.

    If *type_hint* is given (one of :class:`bool`, :class:`int`, :class:`float`,
    :class:`str`) the corresponding typed C++ accessor is called directly.
    Without a hint, the value is auto-detected by attempting conversions in the
    order bool → int → float → str.
    """
    if type_hint is bool:
        return node.param_bool(relpath)
    if type_hint is int:
        return node.param_int(relpath)
    if type_hint is float:
        return node.param_float(relpath)
    if type_hint is str:
        return node.param_str(relpath)

    raw = node.param_str(relpath)
    try:
        return parse_bool(raw)
    except Error:
        pass
    try:
        v = parse_int(raw)
        # Only return int if the string really is an integer (no decimal point).
        if "." not in raw and "e" not in raw.lower():
            return v
    except Error:
        pass
    try:
        return parse_double(raw)
    except Error:
        pass
    return raw


__all__ = [
    "Error",
    "ErrorMessage",
    "NodeType",
    "Node",
    "Root",
    "Section",
    "Field",
    "Comment",
    "Blank",
    "parse_file",
    "parse_text",
    "parse_bool",
    "parse_int",
    "parse_double",
    "parse_float",
    "param",
]
