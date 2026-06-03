"""Pytest test suite for the nmhit Python bindings."""

import pytest
import nmhit


# ── parse_text / basic types ──────────────────────────────────────────────────

def test_parse_text_returns_root():
    root = nmhit.parse_text("k = 42")
    assert isinstance(root, nmhit.Root)


def test_int_field():
    root = nmhit.parse_text("k = 42")
    assert root.param_int("k") == 42


def test_negative_int():
    root = nmhit.parse_text("k = -7")
    assert root.param_int("k") == -7


def test_float_field():
    root = nmhit.parse_text("x = 3.14")
    assert abs(root.param_float("x") - 3.14) < 1e-9


def test_scientific_notation():
    root = nmhit.parse_text("v = 1.5e-3")
    assert abs(root.param_float("v") - 1.5e-3) < 1e-12


def test_bool_true():
    root = nmhit.parse_text("flag = true")
    assert root.param_bool("flag") is True


def test_bool_false():
    root = nmhit.parse_text("flag = false")
    assert root.param_bool("flag") is False


def test_bool_invalid():
    root = nmhit.parse_text("a = yes")
    with pytest.raises(nmhit.Error):
        root.param_bool("a")


def test_str_field():
    root = nmhit.parse_text("name = hello")
    assert root.param_str("name") == "hello"


# ── sections ──────────────────────────────────────────────────────────────────

def test_section_navigation():
    root = nmhit.parse_text("[mesh]\n  dim = 3\n[]")
    assert root.param_int("mesh/dim") == 3


def test_nested_sections():
    root = nmhit.parse_text("[a]\n  [b]\n    k = 42\n  []\n[]")
    assert root.param_int("a/b/k") == 42


# ── find / children ───────────────────────────────────────────────────────────

def test_find_returns_correct_type():
    root = nmhit.parse_text("k = 1")
    node = root.find("k")
    assert isinstance(node, nmhit.Field)


def test_find_returns_none_for_missing():
    root = nmhit.parse_text("k = 1")
    assert root.find("nonexistent") is None


def test_children_unfiltered():
    root = nmhit.parse_text("a = 1\nb = 2")
    kids = root.children()
    assert len(kids) == 2


def test_children_filtered_by_field():
    root = nmhit.parse_text("# comment\nk = 1")
    fields = root.children(nmhit.NodeType.Field)
    assert len(fields) == 1
    assert isinstance(fields[0], nmhit.Field)


def test_children_filtered_by_section():
    root = nmhit.parse_text("[a]\n  x = 1\n[]\n[b]\n  y = 2\n[]")
    secs = root.children(nmhit.NodeType.Section)
    assert len(secs) == 2
    assert all(isinstance(s, nmhit.Section) for s in secs)


# ── lifetime / keepalive ──────────────────────────────────────────────────────

def test_child_ref_survives_root_drop():
    # Dropping the root Python object must not invalidate child references.
    node = nmhit.parse_text("[s]\n  k = 99\n[]").find("s/k")
    import gc; gc.collect()
    assert node.param_int() == 99


def test_children_element_survives_list_drop():
    root = nmhit.parse_text("a = 1\nb = 2")
    kids = root.children()
    kid = kids[0]
    del kids
    import gc; gc.collect()
    assert kid.path() in ("a", "b")


# ── parent / root_node ────────────────────────────────────────────────────────

def test_parent_ref():
    root = nmhit.parse_text("[s]\n  k = 1\n[]")
    sec = root.find("s")
    field = sec.find("k")
    assert field.parent() is not None
    assert field.parent().path() == "s"


def test_root_node():
    root = nmhit.parse_text("[s]\n  k = 1\n[]")
    field = root.find("s/k")
    r = field.root_node()
    assert isinstance(r, nmhit.Root)


# ── param_optional ────────────────────────────────────────────────────────────

def test_param_optional_present():
    root = nmhit.parse_text("x = 5")
    assert root.param_optional_int("x", 0) == 5


def test_param_optional_absent():
    root = nmhit.parse_text("x = 5")
    assert root.param_optional_int("y", 99) == 99


def test_param_optional_float():
    root = nmhit.parse_text("x = 1.5")
    assert abs(root.param_optional_float("x", 0.0) - 1.5) < 1e-9
    assert root.param_optional_float("z", -1.0) == -1.0


def test_param_optional_str():
    root = nmhit.parse_text("name = foo")
    assert root.param_optional_str("name", "default") == "foo"
    assert root.param_optional_str("other", "default") == "default"


# ── arrays ────────────────────────────────────────────────────────────────────

def test_list_int():
    root = nmhit.parse_text("vals = '1 2 3'")
    assert root.param_list_int("vals") == [1, 2, 3]


def test_list_float():
    root = nmhit.parse_text("vals = '1.0 2.5 3.14'")
    v = root.param_list_float("vals")
    assert len(v) == 3
    assert abs(v[1] - 2.5) < 1e-9


def test_list_str():
    root = nmhit.parse_text("tags = 'alpha beta gamma'")
    assert root.param_list_str("tags") == ["alpha", "beta", "gamma"]


def test_list_bool():
    root = nmhit.parse_text("flags = 'true false true'")
    assert root.param_list_bool("flags") == [True, False, True]


def test_list_list_int():
    root = nmhit.parse_text("vals = '1 2 3; 4 5 6'")
    v = root.param_list_list_int("vals")
    assert len(v) == 2
    assert v[0] == [1, 2, 3]
    assert v[1] == [4, 5, 6]


def test_list_list_float():
    root = nmhit.parse_text("m = '1.0 2.0; 3.0 4.0'")
    v = root.param_list_list_float("m")
    assert len(v) == 2
    assert abs(v[0][0] - 1.0) < 1e-9
    assert abs(v[1][1] - 4.0) < 1e-9


def test_list_list_str():
    root = nmhit.parse_text("tags = 'a b; c d'")
    v = root.param_list_list_str("tags")
    assert v == [["a", "b"], ["c", "d"]]


# ── render / clone ────────────────────────────────────────────────────────────

def test_render_round_trip():
    original = "[mesh]\n  dim = 3\n[]\n"
    root = nmhit.parse_text(original)
    rendered = root.render()
    root2 = nmhit.parse_text(rendered)
    assert root2.param_int("mesh/dim") == 3


def test_clone():
    root = nmhit.parse_text("k = 42")
    root2 = root.clone()
    assert root2.param_int("k") == 42


def test_clone_independence():
    root = nmhit.parse_text("k = 1")
    root2 = root.clone()
    root2.find("k").set_val("99")
    assert root.param_int("k") == 1
    assert root2.param_int("k") == 99


# ── Field mutation ────────────────────────────────────────────────────────────

def test_field_raw_val():
    root = nmhit.parse_text("k = 42")
    field = root.find("k")
    assert isinstance(field, nmhit.Field)
    assert field.raw_val() == "42"


def test_field_set_val():
    root = nmhit.parse_text("k = 1")
    root.find("k").set_val("99")
    assert root.param_int("k") == 99


# ── add_child / insert_child / remove_child ───────────────────────────────────

def test_add_child():
    # add_child clones the supplied node into the tree.
    root = nmhit.parse_text("")
    root.add_child(nmhit.Field("k", "42"))
    assert root.param_int("k") == 42


def test_insert_child():
    root = nmhit.parse_text("b = 2")
    root.insert_child(0, nmhit.Field("a", "1"))
    kids = root.children(nmhit.NodeType.Field)
    assert kids[0].path() == "a"
    assert kids[1].path() == "b"


def test_remove_child():
    root = nmhit.parse_text("k = 42")
    removed = root.remove_child("k")   # pass path string
    assert isinstance(removed, nmhit.Field)
    assert root.find("k") is None


def test_remove_child_nested():
    root = nmhit.parse_text("[s]\n  k = 42\n[]")
    removed = root.remove_child("s/k")
    assert isinstance(removed, nmhit.Field)
    assert root.find("s/k") is None


def test_remove_child_missing():
    root = nmhit.parse_text("k = 42")
    with pytest.raises(nmhit.Error):
        root.remove_child("nonexistent")


def test_remove_child_empty_relpath():
    root = nmhit.parse_text("k = 42")
    with pytest.raises(nmhit.Error):
        root.remove_child("")


# ── error handling ────────────────────────────────────────────────────────────

def test_error_on_missing_section_close():
    with pytest.raises(nmhit.Error):
        nmhit.parse_text("[mesh]\n  dim = 3")


def test_error_has_messages():
    with pytest.raises(nmhit.Error) as exc_info:
        nmhit.parse_text("[mesh]\n  dim = 3")
    assert hasattr(exc_info.value, "messages")


def test_error_is_runtime_error():
    with pytest.raises(RuntimeError):
        nmhit.parse_text("[mesh]\n  dim = 3")


def test_error_on_duplicate_field():
    with pytest.raises(nmhit.Error):
        nmhit.parse_text("k = 1\nk = 2")


# ── ErrorMessage ──────────────────────────────────────────────────────────────

def test_error_message_attributes():
    with pytest.raises(nmhit.Error) as exc_info:
        nmhit.parse_text("[mesh]\n  dim = 3")
    msgs = exc_info.value.messages
    assert isinstance(msgs, list)
    if msgs:
        m = msgs[0]
        assert hasattr(m, "line")
        assert hasattr(m, "column")
        assert hasattr(m, "message")
        assert hasattr(m, "filename")


# ── source location ───────────────────────────────────────────────────────────

def test_line_numbers():
    root = nmhit.parse_text("a = hello\nb = world\nc = 3")
    assert root.find("a").line() == 1
    assert root.find("b").line() == 2
    assert root.find("c").line() == 3


def test_fullpath():
    root = nmhit.parse_text("[a]\n  [b]\n    k = 1\n  []\n[]")
    node = root.find("a/b/k")
    assert node.fullpath() == "a/b/k"


def test_path():
    root = nmhit.parse_text("[mesh]\n  dim = 3\n[]")
    sec = root.find("mesh")
    assert sec.path() == "mesh"


# ── parse_file ────────────────────────────────────────────────────────────────

def test_parse_file(tmp_path):
    f = tmp_path / "test.i"
    f.write_text("x = 7\n")
    root = nmhit.parse_file(f)
    assert root.param_int("x") == 7


def test_parse_file_str_path(tmp_path):
    f = tmp_path / "test.i"
    f.write_text("x = 7\n")
    root = nmhit.parse_file(str(f))
    assert root.param_int("x") == 7


def test_parse_file_missing():
    with pytest.raises(nmhit.Error):
        nmhit.parse_file("/nonexistent/path/file.i")


def test_parse_file_with_pre(tmp_path):
    f = tmp_path / "test.i"
    f.write_text("b = 2\n")
    root = nmhit.parse_file(f, pre=["a = 1"])
    assert root.param_int("a") == 1
    assert root.param_int("b") == 2


# ── pre / post overrides ──────────────────────────────────────────────────────

def test_pre_snippets():
    root = nmhit.parse_text("b = 2", pre=["a = 1"])
    assert root.param_int("a") == 1
    assert root.param_int("b") == 2


def test_post_override():
    root = nmhit.parse_text("k = 1", post=["k := 99"])
    assert root.param_int("k") == 99


# ── free scalar converters ────────────────────────────────────────────────────

def test_parse_bool():
    assert nmhit.parse_bool("true") is True
    assert nmhit.parse_bool("false") is False
    assert nmhit.parse_bool("'true'") is True


def test_parse_bool_invalid():
    for bad in ("yes", "no", "on", "off", "True", "1", "0"):
        with pytest.raises(nmhit.Error):
            nmhit.parse_bool(bad)


def test_parse_int():
    assert nmhit.parse_int("42") == 42
    assert nmhit.parse_int("-7") == -7
    assert nmhit.parse_int("'10'") == 10


def test_parse_int_invalid():
    with pytest.raises(nmhit.Error):
        nmhit.parse_int("3.14")


def test_parse_double():
    assert abs(nmhit.parse_double("3.14") - 3.14) < 1e-9
    assert nmhit.parse_double("1e3") == 1000.0


def test_parse_float():
    assert abs(nmhit.parse_float("1.5") - 1.5) < 1e-6


# ── nmhit.param() convenience wrapper ────────────────────────────────────────

def test_param_auto_int():
    root = nmhit.parse_text("k = 42")
    assert nmhit.param(root, "k") == 42
    assert isinstance(nmhit.param(root, "k"), int)


def test_param_auto_float():
    root = nmhit.parse_text("x = 3.14")
    val = nmhit.param(root, "x")
    assert isinstance(val, float)
    assert abs(val - 3.14) < 1e-9


def test_param_auto_bool():
    root = nmhit.parse_text("flag = true")
    assert nmhit.param(root, "flag") is True


def test_param_auto_str():
    root = nmhit.parse_text("name = hello")
    assert nmhit.param(root, "name") == "hello"


def test_param_with_type_hint_float():
    root = nmhit.parse_text("n = 5")
    assert nmhit.param(root, "n", float) == 5.0


def test_param_with_type_hint_str():
    root = nmhit.parse_text("n = 42")
    assert nmhit.param(root, "n", str) == "42"


# ── Comment / Blank node types ────────────────────────────────────────────────

def test_comment_preserved():
    root = nmhit.parse_text("# top comment\nk = 1")
    comments = root.children(nmhit.NodeType.Comment)
    assert len(comments) == 1
    assert isinstance(comments[0], nmhit.Comment)
    assert "top comment" in comments[0].text()


def test_blank_preserved():
    root = nmhit.parse_text("a = 1\n\nb = 2")
    blanks = root.children(nmhit.NodeType.Blank)
    assert len(blanks) >= 1


def test_comment_inline():
    c = nmhit.Comment("hello", is_inline=True)
    assert c.is_inline() is True
    c.set_inline(False)
    assert c.is_inline() is False


# ── node identity and type ────────────────────────────────────────────────────

def test_node_type_enum():
    root = nmhit.parse_text("[s]\n  k = 1\n[]")
    assert root.type() == nmhit.NodeType.Root
    assert root.find("s").type() == nmhit.NodeType.Section
    assert root.find("s/k").type() == nmhit.NodeType.Field


def test_node_repr():
    root = nmhit.parse_text("")
    assert "Root" in repr(root)


# ── override assignment ───────────────────────────────────────────────────────

def test_override_assign():
    root = nmhit.parse_text("k = 1\nk := 99")
    assert root.param_int("k") == 99


# ── triple-quoted strings ─────────────────────────────────────────────────────

def test_triple_single_quote_basic():
    """'''value''' is parsed verbatim and returned by param_str."""
    root = nmhit.parse_text("k = '''hello world'''")
    assert root.param_str("k") == "hello world"


def test_triple_double_quote_basic():
    """\"\"\"value\"\"\"  is parsed verbatim and returned by param_str."""
    root = nmhit.parse_text('k = """hello world"""')
    assert root.param_str("k") == "hello world"


def test_triple_single_quote_multiline():
    """Triple single-quoted strings preserve internal newlines verbatim."""
    root = nmhit.parse_text("k = '''\n  line1\n  line2\n'''")
    val = root.param_str("k")
    assert "line1" in val
    assert "line2" in val
    assert "\n" in val


def test_triple_double_quote_multiline():
    """Triple double-quoted strings preserve internal newlines verbatim."""
    root = nmhit.parse_text('k = """\n  line1\n  line2\n"""')
    val = root.param_str("k")
    assert "line1" in val
    assert "line2" in val
    assert "\n" in val


def test_triple_single_quote_contains_double_quotes():
    """Triple single-quoted strings may contain double-quote characters."""
    root = nmhit.parse_text("""k = '''say "hello"'''""")
    assert root.param_str("k") == 'say "hello"'


def test_triple_double_quote_contains_single_quotes():
    """Triple double-quoted strings may contain single-quote characters."""
    root = nmhit.parse_text("""k = \"\"\"it's fine\"\"\"""")
    assert root.param_str("k") == "it's fine"


def test_triple_single_quote_contains_single_quote():
    """A single embedded apostrophe inside triple single-quoted string is allowed."""
    root = nmhit.parse_text("k = '''it's verbatim'''")
    assert root.param_str("k") == "it's verbatim"


def test_triple_double_quote_contains_double_quote():
    """A single embedded double-quote inside triple double-quoted string is allowed."""
    root = nmhit.parse_text('k = """say "hi" """')
    assert '"hi"' in root.param_str("k")


def test_triple_single_quote_both_quote_types():
    """Triple single-quoted strings may contain both quote types."""
    root = nmhit.parse_text("""k = '''it's "great"'''""")
    val = root.param_str("k")
    assert "it's" in val
    assert '"great"' in val


def test_triple_double_quote_both_quote_types():
    """Triple double-quoted strings may contain both quote types."""
    # """...""" delimiter allows single ' freely; single " is also fine mid-string.
    root = nmhit.parse_text('k = """it\'s "great" """')
    val = root.param_str("k")
    assert "it's" in val
    assert '"great"' in val


def test_triple_single_quote_in_section():
    """Triple-quoted strings work inside sections."""
    root = nmhit.parse_text("[s]\n  code = '''\n    result = 1\n  '''\n[]")
    val = root.param_str("s/code")
    assert "result" in val


def test_triple_single_quote_multiline_indented():
    """Indented multi-line triple-quoted content is preserved verbatim (no stripping)."""
    root = nmhit.parse_text("k = '''\n      x = 1\n      y = 2\n    '''")
    val = root.param_str("k")
    assert "x = 1" in val
    assert "y = 2" in val
    # Leading whitespace of interior lines is preserved
    assert "      x" in val or "    x" in val  # some indentation present


def test_triple_quote_empty():
    """An empty triple-quoted string returns an empty string."""
    root = nmhit.parse_text("k = ''''''")
    assert root.param_str("k") == ""


def test_triple_quote_python_expression():
    """Typical use case: Python code expression stored verbatim."""
    code = "import torch\nresult = torch.tensor([1.0, 2.0, 3.0])"
    hit = f"k = '''\n{code}\n'''"
    root = nmhit.parse_text(hit)
    val = root.param_str("k")
    assert "import torch" in val
    assert "torch.tensor" in val
    assert "\n" in val


def test_triple_quote_only_readable_as_string():
    """Verbatim (triple-quoted) fields raise when read as non-string types."""
    root = nmhit.parse_text("k = '''42'''")
    # param_str must succeed
    assert root.param_str("k") == "42"
    # All non-string interpretations must raise
    with pytest.raises(nmhit.Error, match="verbatim"):
        root.param_int("k")
    with pytest.raises(nmhit.Error, match="verbatim"):
        root.param_float("k")
    with pytest.raises(nmhit.Error, match="verbatim"):
        root.param_bool("k")


def test_triple_quote_not_readable_as_list():
    """Verbatim fields also raise when read as list types."""
    root = nmhit.parse_text("k = '''1 2 3'''")
    assert root.param_str("k") == "1 2 3"
    with pytest.raises(nmhit.Error, match="verbatim"):
        root.param_list_int("k")


# ── triple-quoted round-trip rendering ────────────────────────────────────────
# Round-trip = parse_text(s).render() == s, byte-for-byte. The formatter
# (nmhit-format) walks this exact pipeline, so any divergence here is a
# direct formatter-breaking bug.


def test_triple_single_quote_roundtrip_inline():
    src = "k = '''hello world'''\n"
    assert nmhit.parse_text(src).render() == src


def test_triple_double_quote_roundtrip_inline():
    src = 'k = """hello world"""\n'
    assert nmhit.parse_text(src).render() == src


def test_triple_single_quote_roundtrip_multiline():
    src = "k = '''\n  line1\n  line2\n'''\n"
    assert nmhit.parse_text(src).render() == src


def test_triple_double_quote_roundtrip_multiline():
    src = 'k = """\n  line1\n  line2\n"""\n'
    assert nmhit.parse_text(src).render() == src


def test_triple_quote_roundtrip_python_expression():
    src = (
        "[Tensors]\n"
        "  [strain]\n"
        "    type = Python\n"
        "    expr = '''\n"
        "torch.stack([\n"
        "    torch.linspace(0, 1, 5),\n"
        "    torch.linspace(1, 2, 5),\n"
        "])\n"
        "'''\n"
        "  []\n"
        "[]\n"
    )
    assert nmhit.parse_text(src).render() == src


def test_triple_quote_roundtrip_preserves_delimiter_style():
    """The renderer must wrap the body in the same delimiter the input used."""
    src_sq = "k = '''\nbody\n'''\n"
    src_dq = 'k = """\nbody\n"""\n'
    assert nmhit.parse_text(src_sq).render() == src_sq
    assert nmhit.parse_text(src_dq).render() == src_dq
