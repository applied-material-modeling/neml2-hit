"""Pytest test suite for the nmhit Python bindings.

Scope is intentionally narrow: each test verifies that a Python-exposed
binding is reachable AND returns / accepts the right Python type. Parser
semantics (int / float / bool parsing, brace expansion, round-trip,
override-wins, error messages, ...) are tested in tests/test_hit.cpp;
duplicating those here just means two test suites have to be kept in
sync for the same feature.

What lives here:

* Class + free-function existence (``isinstance(root, nmhit.Root)``,
  ``nmhit.parse_text`` callable, ``nmhit.NodeType`` enum reachable).
* Type-erasing wrappers and helpers that exist *only* in Python
  (``nmhit.param`` auto-dispatch, ``nmhit-format`` CLI).
* Python-side machinery the C++ tests can't reach: ``Path``/str
  acceptance, kwarg names, ``__repr__``, ``Error`` subclassing
  ``RuntimeError``, child references surviving parent GC.

What does NOT live here: anything where the body would amount to "call
the binding, then re-test the C++ behaviour the call delegates to."
"""

import gc

import pytest

import nmhit


# ── module surface ────────────────────────────────────────────────────────────


def test_module_classes_exposed():
    """The C++ types we expose by name in __init__.pyi are real Python
    attributes (not stubs the user can't actually construct against)."""
    for name in (
        "Root",
        "Section",
        "Field",
        "Comment",
        "Blank",
        "NodeType",
        "Error",
    ):
        assert hasattr(nmhit, name), f"nmhit.{name} missing"


def test_error_subclasses_runtime_error():
    """nmhit.Error is bound as a RuntimeError subclass so generic
    ``except RuntimeError`` paths in user code catch it."""
    assert issubclass(nmhit.Error, RuntimeError)


def test_node_type_enum_round_trip():
    """NodeType is bound as a proper enum -- each value is comparable to
    itself across separate parses (rules out the stub-int-constant trap)."""
    root_a = nmhit.parse_text("[s]\n  k = 1\n[]")
    root_b = nmhit.parse_text("[t]\n[]")
    assert root_a.type() == nmhit.NodeType.Root == root_b.type()
    assert root_a.find("s").type() == nmhit.NodeType.Section
    assert root_a.find("s/k").type() == nmhit.NodeType.Field


# ── parse entry points ────────────────────────────────────────────────────────


def test_parse_text_returns_root():
    assert isinstance(nmhit.parse_text("k = 42"), nmhit.Root)


def test_parse_file_accepts_pathlike(tmp_path):
    """parse_file's path argument accepts both pathlib.Path and str."""
    f = tmp_path / "test.i"
    f.write_text("x = 7\n")
    assert isinstance(nmhit.parse_file(f), nmhit.Root)
    assert isinstance(nmhit.parse_file(str(f)), nmhit.Root)


def test_parse_text_pre_post_kwargs():
    """pre / post are keyword-argument bindings (positional ordering and
    name spelled correctly on the Python side)."""
    root = nmhit.parse_text("b = 2", pre=["a = 1"], post=["b := 99"])
    assert root.param_int("a") == 1
    assert root.param_int("b") == 99


def test_parse_file_pre_kwarg(tmp_path):
    f = tmp_path / "test.i"
    f.write_text("b = 2\n")
    root = nmhit.parse_file(f, pre=["a = 1"])
    assert root.param_int("a") == 1


# ── find / children ───────────────────────────────────────────────────────────


def test_find_missing_returns_none():
    """A missing path returns Python None (not raises, not a NULL pointer
    surfaced as a crash)."""
    root = nmhit.parse_text("k = 1")
    assert root.find("nonexistent") is None


def test_children_filter_returns_correct_subclass():
    """``children(NodeType.X)`` returns instances of the right
    concrete Python subclass (Field / Section / Comment / Blank)."""
    src = "# comment\n\na = 1\n[s]\n[]\n"
    root = nmhit.parse_text(src)
    assert all(isinstance(n, nmhit.Comment) for n in root.children(nmhit.NodeType.Comment))
    assert all(isinstance(n, nmhit.Blank) for n in root.children(nmhit.NodeType.Blank))
    assert all(isinstance(n, nmhit.Field) for n in root.children(nmhit.NodeType.Field))
    assert all(isinstance(n, nmhit.Section) for n in root.children(nmhit.NodeType.Section))


# ── keepalive / lifetime ──────────────────────────────────────────────────────


def test_child_ref_survives_root_drop():
    """A node grabbed off a Root must keep the Root alive (nanobind
    keep_alive policy). Without this, GC would free the C++ tree out
    from under the Python handle."""
    node = nmhit.parse_text("[s]\n  k = 99\n[]").find("s/k")
    gc.collect()
    assert node.param_int() == 99


def test_children_element_survives_list_drop():
    """Same keepalive contract for elements returned from children()."""
    root = nmhit.parse_text("a = 1\nb = 2")
    kids = root.children()
    kid = kids[0]
    del kids
    gc.collect()
    assert kid.path() in ("a", "b")


# ── parent / root_node ────────────────────────────────────────────────────────


def test_parent_returns_section():
    """parent() returns a Python-typed node (not the raw void* you'd get
    if the binding skipped the type-dispatch shim)."""
    root = nmhit.parse_text("[s]\n  k = 1\n[]")
    parent = root.find("s/k").parent()
    assert isinstance(parent, nmhit.Section)
    assert parent.path() == "s"


def test_root_node_returns_root():
    root = nmhit.parse_text("[s]\n  k = 1\n[]")
    r = root.find("s/k").root_node()
    assert isinstance(r, nmhit.Root)


# ── error forwarding ──────────────────────────────────────────────────────────


def test_parse_error_is_nmhit_error():
    """Parse failures raise nmhit.Error (forwarded from C++ exception)."""
    with pytest.raises(nmhit.Error):
        nmhit.parse_text("[mesh]\n  dim = 3")


def test_error_messages_attribute():
    """The Error instance carries a ``messages`` list of ErrorMessage
    objects with line / column / message / filename attributes (struct
    binding)."""
    with pytest.raises(nmhit.Error) as exc_info:
        nmhit.parse_text("[mesh]\n  dim = 3")
    msgs = exc_info.value.messages
    assert isinstance(msgs, list)
    if msgs:
        m = msgs[0]
        for attr in ("line", "column", "message", "filename"):
            assert hasattr(m, attr), f"ErrorMessage.{attr} missing"


def test_parse_file_missing_raises():
    """Filesystem-not-found is forwarded as nmhit.Error, not as a bare
    Python FileNotFoundError or a segfault."""
    with pytest.raises(nmhit.Error):
        nmhit.parse_file("/nonexistent/path/file.i")


# ── tree mutation bindings ────────────────────────────────────────────────────


def test_field_constructor_callable_from_python():
    """nmhit.Field(name, value) constructor exposed for tree building."""
    f = nmhit.Field("k", "42")
    assert isinstance(f, nmhit.Field)


def test_add_child_round_trip():
    """add_child / param_int are reachable bindings on Root."""
    root = nmhit.parse_text("")
    root.add_child(nmhit.Field("k", "42"))
    assert root.param_int("k") == 42


def test_remove_child_returns_typed_node():
    """remove_child returns the right Python-typed node (Field here)."""
    root = nmhit.parse_text("k = 42")
    removed = root.remove_child("k")
    assert isinstance(removed, nmhit.Field)


# ── Comment binding ──────────────────────────────────────────────────────────


def test_comment_inline_property():
    """Comment.is_inline / set_inline are exposed as a callable
    getter+setter pair (not a Python property — match what the C++
    side declares)."""
    c = nmhit.Comment("hello", is_inline=True)
    assert c.is_inline() is True
    c.set_inline(False)
    assert c.is_inline() is False


# ── repr ─────────────────────────────────────────────────────────────────────


def test_node_repr_contains_class_name():
    """__repr__ bindings produce something human-readable that names
    the concrete subclass."""
    assert "Root" in repr(nmhit.parse_text(""))


# ── nmhit.param: Python-only auto-dispatch helper ────────────────────────────
# Has no C++ counterpart -- the C++ API is one method per type.


def test_param_auto_int():
    root = nmhit.parse_text("k = 42")
    val = nmhit.param(root, "k")
    assert isinstance(val, int) and val == 42


def test_param_auto_float():
    root = nmhit.parse_text("x = 3.14")
    val = nmhit.param(root, "x")
    assert isinstance(val, float)


def test_param_auto_bool():
    root = nmhit.parse_text("flag = true")
    assert nmhit.param(root, "flag") is True


def test_param_auto_str():
    root = nmhit.parse_text("name = hello")
    val = nmhit.param(root, "name")
    assert isinstance(val, str) and val == "hello"


def test_param_with_explicit_type_hint():
    """Passing an explicit type forces the converter -- exercises the
    Python-side dispatch table."""
    root = nmhit.parse_text("n = 5")
    val = nmhit.param(root, "n", float)
    assert isinstance(val, float) and val == 5.0


# ── verbatim → non-string read forwarding ─────────────────────────────────────


def test_verbatim_field_raises_on_non_str_getter():
    """Verbatim (triple-quoted) fields can only be read as strings; every
    other ``param_*`` getter raises nmhit.Error. Pinned here because the
    error path crosses the bindings boundary."""
    root = nmhit.parse_text("k = '''42'''")
    assert root.param_str("k") == "42"
    for getter in ("param_int", "param_float", "param_bool", "param_list_int"):
        with pytest.raises(nmhit.Error, match="verbatim"):
            getattr(root, getter)("k")
