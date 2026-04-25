/**
 * Lightweight self-contained test runner for the HIT parser.
 *
 * Each test is a lambda that is expected NOT to throw.  Any uncaught
 * exception or explicit FAIL() call counts as a failure.
 */

#include "nmhit/nmhit.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ─── mini test framework ──────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;

#define EXPECT(cond)                                                                               \
  do                                                                                               \
  {                                                                                                \
    if (!(cond))                                                                                   \
    {                                                                                              \
      std::cerr << "  FAIL at " << __FILE__ << ':' << __LINE__ << ": " << #cond << '\n';           \
      throw std::runtime_error("expectation failed");                                              \
    }                                                                                              \
  } while (0)

#define EXPECT_THROW(expr, ExcType)                                                                \
  do                                                                                               \
  {                                                                                                \
    bool caught_ = false;                                                                          \
    try                                                                                            \
    {                                                                                              \
      expr;                                                                                        \
    }                                                                                              \
    catch (const ExcType &)                                                                        \
    {                                                                                              \
      caught_ = true;                                                                              \
    }                                                                                              \
    if (!caught_)                                                                                  \
    {                                                                                              \
      std::cerr << "  FAIL at " << __FILE__ << ':' << __LINE__                                     \
                << ": expected " #ExcType " not thrown\n";                                         \
      throw std::runtime_error("expected exception not thrown");                                   \
    }                                                                                              \
  } while (0)

static void
run(const std::string & name, std::function<void()> fn)
{
  std::cerr << "[ RUN ] " << name << '\n';
  try
  {
    fn();
    std::cerr << "[ OK  ] " << name << '\n';
    ++g_passed;
  }
  catch (const std::exception & e)
  {
    std::cerr << "[ FAIL] " << name << " — " << e.what() << '\n';
    ++g_failed;
  }
}

// ─── helpers ─────────────────────────────────────────────────────────────────

// Shared temp file used by tests that exercise parse_file().
static const std::filesystem::path g_test_file =
  std::filesystem::temp_directory_path() / "nmhit_test.i";

static std::unique_ptr<nmhit::Node>
p(const std::string & input)
{
  return nmhit::parse_text(input);
}

// ─── custom type used in section 14 tests ────────────────────────────────────

enum class Color
{
  Red,
  Green,
  Blue
};

// ─── tests ───────────────────────────────────────────────────────────────────

int
main()
{
  // Register custom type parser once, before all tests.
  nmhit::TypeRegistry::register_parser<Color>([](const std::string & s) -> Color {
    if (s == "red")
      return Color::Red;
    if (s == "green")
      return Color::Green;
    if (s == "blue")
      return Color::Blue;
    throw std::invalid_argument("unknown color: " + s);
  });
  // ── 1. Basic key-value pairs ──────────────────────────────────────────────

  run("int_value", []() {
    auto root = p("k = 42");
    EXPECT(root->param<int>("k") == 42);
  });

  run("negative_int", []() {
    auto root = p("k = -7");
    EXPECT(root->param<int64_t>("k") == -7);
  });

  run("float_value", []() {
    auto root = p("x = 3.14");
    EXPECT(std::abs(root->param<double>("x") - 3.14) < 1e-9);
  });

  run("scientific_notation", []() {
    auto root = p("v = 1.5e-3");
    EXPECT(std::abs(root->param<double>("v") - 1.5e-3) < 1e-12);
  });

  run("unquoted_string", []() {
    auto root = p("name = hello");
    EXPECT(root->param<std::string>("name") == "hello");
  });

  run("double_quoted_string", []() {
    auto root = p("msg = \"hello world\"");
    // Double quotes are now equivalent to single quotes — tokenised as an array.
    EXPECT(root->param<std::string>("msg") == "hello world");
    auto v = root->param<std::vector<std::string>>("msg");
    EXPECT(v.size() == 2 && v[0] == "hello" && v[1] == "world");
  });

  run("quote_styles_equivalent", []() {
    auto r1 = p("v = '1 2 3'");
    auto r2 = p("v = \"1 2 3\"");
    EXPECT(r1->param<std::string>("v") == r2->param<std::string>("v"));
    auto a1 = r1->param<std::vector<int>>("v");
    auto a2 = r2->param<std::vector<int>>("v");
    EXPECT(a1 == a2);
  });

  // ── 2. Bool values ────────────────────────────────────────────────────────

  run("bool_true", []() {
    auto root = p("a = true");
    EXPECT(root->param<bool>("a") == true);
  });

  run("bool_false", []() {
    auto root = p("a = false");
    EXPECT(root->param<bool>("a") == false);
  });

  run("bool_invalid", []() {
    // Only lowercase "true"/"false" are valid; everything else must throw.
    for (auto s : {"yes", "no", "on", "off", "True", "False", "TRUE", "FALSE", "1", "0"})
      EXPECT_THROW(p(std::string("a = ") + s)->param<bool>("a"), nmhit::Error);
  });

  // ── 3. Arrays ────────────────────────────────────────────────────────────

  run("int_array", []() {
    auto root = p("vals = '1 2 3'");
    auto v = root->param<std::vector<int64_t>>("vals");
    EXPECT(v.size() == 3);
    EXPECT(v[0] == 1 && v[1] == 2 && v[2] == 3);
  });

  run("float_array", []() {
    auto root = p("vals = '1.0 2.5 3.14'");
    auto v = root->param<std::vector<double>>("vals");
    EXPECT(v.size() == 3);
    EXPECT(std::abs(v[1] - 2.5) < 1e-9);
  });

  run("string_array", []() {
    auto root = p("tags = 'alpha beta gamma'");
    auto v = root->param<std::vector<std::string>>("tags");
    EXPECT(v.size() == 3);
    EXPECT(v[0] == "alpha");
    EXPECT(v[2] == "gamma");
  });

  run("empty_array", []() {
    auto root = p("v = ''");
    // empty array → empty string value
    EXPECT(root->find("v") != nullptr);
  });

  // ── 4. Sections ───────────────────────────────────────────────────────────

  run("simple_section", []() {
    auto root = p("[mesh]\n  dim = 3\n[]");
    EXPECT(root->param<int>("mesh/dim") == 3);
  });

  run("nested_sections", []() {
    auto root = p("[a]\n  [b]\n    k = 42\n  []\n[]");
    EXPECT(root->param<int>("a/b/k") == 42);
  });

  run("path_split_section", []() {
    // [a/b] should create nested Section("a") > Section("b")
    auto root = p("[a/b]\n  k = 1\n[]");
    EXPECT(root->param<int>("a/b/k") == 1);
  });

  run("path_split_field", []() {
    // x/y = 1 inside a section should create nested structure
    auto root = p("[s]\n  a/b = 99\n[]");
    EXPECT(root->param<int>("s/a/b") == 99);
  });

  run("path_split_shared_parent", []() {
    // Two path-split fields sharing a common ancestor must merge into one section.
    auto root = p("Models/a/foo = 1\nModels/b/bar = 2");
    EXPECT(root->param<int>("Models/a/foo") == 1);
    EXPECT(root->param<int>("Models/b/bar") == 2);
    // Only one top-level "Models" section should exist.
    EXPECT(root->children(nmhit::NodeType::Section).size() == 1);
  });

  run("path_split_shared_parent_in_section", []() {
    // Same merging applies inside an explicit section.
    auto root = p("[outer]\n  a/x = 10\n  a/y = 20\n[]");
    EXPECT(root->param<int>("outer/a/x") == 10);
    EXPECT(root->param<int>("outer/a/y") == 20);
  });

  run("path_split_override_shared_parent", []() {
    // ':=' override through a shared path-split parent.
    auto root = p("a/k = 1\na/k := 99");
    EXPECT(root->param<int>("a/k") == 99);
    // Only one Field "k" should survive.
    auto * sec = root->find("a");
    EXPECT(sec != nullptr);
    EXPECT(sec->children(nmhit::NodeType::Field).size() == 1);
  });

  run("path_split_duplicate_error_shared_parent", []() {
    // Duplicate (non-override) field through a shared path-split parent is an error.
    EXPECT_THROW(p("a/k = 1\na/k = 2"), nmhit::Error);
  });

  run("multiple_sections", []() {
    auto root = p("[a]\n  x = 1\n[]\n[b]\n  y = 2\n[]");
    EXPECT(root->param<int>("a/x") == 1);
    EXPECT(root->param<int>("b/y") == 2);
  });

  // ── 5. Comments and blank lines ───────────────────────────────────────────

  run("comment_preserved", []() {
    auto root = p("# top comment\nk = 1");
    auto kids = root->children();
    // Should have a comment and a field
    bool has_comment = false;
    for (auto * c : kids)
      if (c->type() == nmhit::NodeType::Comment)
        has_comment = true;
    EXPECT(has_comment);
    EXPECT(root->param<int>("k") == 1);
  });

  run("blank_line_preserved", []() {
    auto root = p("a = 1\n\nb = 2");
    bool has_blank = false;
    for (auto * c : root->children())
      if (c->type() == nmhit::NodeType::Blank)
        has_blank = true;
    EXPECT(has_blank);
  });

  // ── 6. Round-trip rendering ───────────────────────────────────────────────

  run("round_trip_simple", []() {
    std::string input = "[mesh]\n  dim = 3\n[]\n";
    auto root = p(input);
    auto rendered = root->render();
    auto root2 = p(rendered);
    EXPECT(root2->param<int>("mesh/dim") == 3);
  });

  run("round_trip_with_comment", []() {
    std::string input = "# My comment\nk = 42\n";
    auto root = p(input);
    auto rendered = root->render();
    EXPECT(rendered.find("# My comment") != std::string::npos);
  });

  // ── 7. Override assignment ────────────────────────────────────────────────

  run("override_assign", []() {
    auto root = p("k = 1\nk := 99");
    // The override replaces the original; only k = 99 remains.
    EXPECT(root->param<int>("k") == 99);
  });

  // ── 8. Brace expressions ──────────────────────────────────────────────────

  run("brace_env", []() {
    // Set a known env var
    ::setenv("HIT_TEST_VAR", "hello_from_env", 1);
    auto root = p("v = ${env HIT_TEST_VAR}");
    EXPECT(root->param<std::string>("v") == "hello_from_env");
  });

  run("brace_raw", []() {
    auto root = p("v = ${raw foo bar}");
    EXPECT(root->param<std::string>("v") == "foobar");
  });

  run("brace_param_lookup", []() {
    auto root = p("base = hello\nderived = ${base}");
    EXPECT(root->param<std::string>("derived") == "hello");
  });

  run("brace_as_array_element", []() {
    // A brace expression appearing as one element among others in a quoted array.
    auto root = p("base = 42\nvals = '${base} 2 3'");
    auto v = root->param<std::vector<std::string>>("vals");
    EXPECT(v.size() == 3);
    EXPECT(v[0] == "42");
    EXPECT(v[1] == "2");
    EXPECT(v[2] == "3");
  });

  run("brace_as_sole_array_element", []() {
    // A brace expression that is the only element in a quoted array.
    auto root = p("n = 7\nvals = '${n}'");
    auto v = root->param<std::vector<std::string>>("vals");
    EXPECT(v.size() == 1);
    EXPECT(v[0] == "7");
  });

  run("brace_array_double_quoted", []() {
    // Double-quoted arrays also support brace expressions.
    auto root = p("tag = foo\nvals = \"${tag} bar\"");
    auto v = root->param<std::vector<std::string>>("vals");
    EXPECT(v.size() == 2);
    EXPECT(v[0] == "foo");
    EXPECT(v[1] == "bar");
  });

  run("brace_as_int_array_element", []() {
    // A brace expression that resolves to a number in an integer array.
    auto root = p("n = 10\nvals = '1 ${n} 3'");
    auto v = root->param<std::vector<int>>("vals");
    EXPECT(v.size() == 3);
    EXPECT(v[0] == 1 && v[1] == 10 && v[2] == 3);
  });

  run("brace_no_extra_whitespace", []() {
    // Substitution must not pad the expanded value with surrounding spaces.
    auto root = p("a = 7\nb = '(${a})'");
    EXPECT(root->param<std::string>("b") == "(7)");
  });

  run("brace_in_unquoted_string", []() {
    // Brace expression embedded in an unquoted value (no surrounding quotes).
    auto root = p("a = 7\nb = (${a})");
    EXPECT(root->param<std::string>("b") == "(7)");
  });

  run("brace_in_unquoted_leading", []() {
    // Brace expression at the start of an unquoted value, followed by literal.
    auto root = p("x = hello\ny = ${x}_world");
    EXPECT(root->param<std::string>("y") == "hello_world");
  });

  // ── 9. fullpath / find ────────────────────────────────────────────────────

  run("fullpath", []() {
    auto root = p("[a]\n  [b]\n    k = 1\n  []\n[]");
    auto * n = root->find("a/b/k");
    EXPECT(n != nullptr);
    EXPECT(n->fullpath() == "a/b/k");
  });

  run("param_optional_present", []() {
    auto root = p("x = 5");
    EXPECT(root->param_optional<int>("x", 0) == 5);
  });

  run("param_optional_absent", []() {
    auto root = p("x = 5");
    EXPECT(root->param_optional<int>("y", 99) == 99);
  });

  // ── 10. Error cases ───────────────────────────────────────────────────────

  run("missing_section_close", []() { EXPECT_THROW(p("[mesh]\n  dim = 3"), nmhit::Error); });

  run("unterminated_string", []() { EXPECT_THROW(p("k = \"unterminated"), nmhit::Error); });

  // ── 11. Clone ─────────────────────────────────────────────────────────────

  run("clone_field", []() {
    auto root = p("k = 42");
    auto root2 = root->clone();
    EXPECT(root2->param<int>("k") == 42);
  });

  run("clone_section", []() {
    auto root = p("[mesh]\n  dim = 3\n[]");
    auto root2 = root->clone();
    EXPECT(root2->param<int>("mesh/dim") == 3);
  });

  // ── 12. Scalar converter public API ──────────────────────────────────────

  run("parse_bool_true", []() { EXPECT(nmhit::parse_bool("true") == true); });
  run("parse_bool_false", []() { EXPECT(nmhit::parse_bool("false") == false); });
  run("parse_bool_quoted", []() { EXPECT(nmhit::parse_bool("'true'") == true); });
  run("parse_bool_invalid_throws", []() { EXPECT_THROW(nmhit::parse_bool("yes"), nmhit::Error); });

  run("parse_int_positive", []() { EXPECT(nmhit::parse_int("42") == int64_t{42}); });
  run("parse_int_negative", []() { EXPECT(nmhit::parse_int("-7") == int64_t{-7}); });
  run("parse_int_quoted", []() { EXPECT(nmhit::parse_int("'10'") == int64_t{10}); });
  run("parse_int_invalid_throws", []() { EXPECT_THROW(nmhit::parse_int("3.14"), nmhit::Error); });

  run("parse_double_value", []() { EXPECT(nmhit::parse_double("3.14") == 3.14); });
  run("parse_double_scientific", []() { EXPECT(nmhit::parse_double("1e3") == 1000.0); });
  run("parse_double_invalid_throws",
      []() { EXPECT_THROW(nmhit::parse_double("abc"), nmhit::Error); });

  run("parse_float_value", []() { EXPECT(nmhit::parse_float("1.5") == 1.5f); });

  // ── 12. Misc value types ──────────────────────────────────────────────────

  run("size_t_value", []() {
    auto root = p("n = 42");
    EXPECT(root->param<std::size_t>("n") == std::size_t{42});
  });

  run("size_t_zero", []() {
    auto root = p("n = 0");
    EXPECT(root->param<std::size_t>("n") == std::size_t{0});
  });

  run("size_t_negative_throws", []() {
    auto root = p("n = -1");
    EXPECT_THROW(root->param<std::size_t>("n"), nmhit::Error);
  });

  run("int_as_double", []() {
    auto root = p("k = 10");
    EXPECT(root->param<double>("k") == 10.0);
  });

  run("multiple_fields", []() {
    auto root = p("a = 1\nb = 2\nc = 3");
    EXPECT(root->param<int>("a") == 1);
    EXPECT(root->param<int>("b") == 2);
    EXPECT(root->param<int>("c") == 3);
  });

  // ── 13. 2-D arrays ────────────────────────────────────────────────────────

  run("int_2d_array", []() {
    auto root = p("vals = '1 2 3; 4 5 6'");
    auto v = root->param<std::vector<std::vector<int>>>("vals");
    EXPECT(v.size() == 2);
    EXPECT(v[0].size() == 3 && v[1].size() == 3);
    EXPECT(v[0][0] == 1 && v[0][1] == 2 && v[0][2] == 3);
    EXPECT(v[1][0] == 4 && v[1][1] == 5 && v[1][2] == 6);
  });

  run("float_2d_array", []() {
    auto root = p("m = '1.0 2.0; 3.0 4.0'");
    auto v = root->param<std::vector<std::vector<double>>>("m");
    EXPECT(v.size() == 2);
    EXPECT(std::abs(v[0][0] - 1.0) < 1e-9);
    EXPECT(std::abs(v[1][1] - 4.0) < 1e-9);
  });

  run("string_2d_array", []() {
    auto root = p("tags = 'a b; c d'");
    auto v = root->param<std::vector<std::vector<std::string>>>("tags");
    EXPECT(v.size() == 2);
    EXPECT(v[0][0] == "a" && v[0][1] == "b");
    EXPECT(v[1][0] == "c" && v[1][1] == "d");
  });

  run("single_row_as_2d", []() {
    // A 1D array read as 2D returns a single row.
    auto root = p("vals = '1 2 3'");
    auto v = root->param<std::vector<std::vector<int>>>("vals");
    EXPECT(v.size() == 1);
    EXPECT(v[0].size() == 3);
  });

  run("2d_round_trip", []() {
    auto root = p("m = '1 2; 3 4'");
    auto rendered = root->render();
    auto root2 = p(rendered);
    auto v = root2->param<std::vector<std::vector<int>>>("m");
    EXPECT(v.size() == 2 && v[0][0] == 1 && v[1][1] == 4);
  });

  // ── 14. Custom type registration ──────────────────────────────────────────

  run("custom_scalar", []() {
    auto root = p("color = red");
    EXPECT(root->param<Color>("color") == Color::Red);
  });

  run("custom_1d_array", []() {
    auto root = p("colors = 'red green blue'");
    auto v = root->param<std::vector<Color>>("colors");
    EXPECT(v.size() == 3);
    EXPECT(v[0] == Color::Red && v[1] == Color::Green && v[2] == Color::Blue);
  });

  run("custom_2d_array", []() {
    auto root = p("m = 'red green; blue red'");
    auto v = root->param<std::vector<std::vector<Color>>>("m");
    EXPECT(v.size() == 2);
    EXPECT(v[0][0] == Color::Red && v[0][1] == Color::Green);
    EXPECT(v[1][0] == Color::Blue && v[1][1] == Color::Red);
  });

  run("unregistered_type_throws", []() {
    struct Unregistered
    {};
    auto root = p("k = foo");
    EXPECT_THROW(root->param<Unregistered>("k"), nmhit::Error);
  });

  // ── 15. Duplicate field detection ─────────────────────────────────────────

  run("duplicate_field_error", []() { EXPECT_THROW(p("k = 1\nk = 2"), nmhit::Error); });

  run("duplicate_field_in_section",
      []() { EXPECT_THROW(p("[s]\n  k = 1\n  k = 2\n[]"), nmhit::Error); });

  run("override_allows_duplicate", []() {
    // ':=' replaces the original value; only the override remains.
    auto root = p("k = 1\nk := 99");
    EXPECT(root->param<int>("k") == 99);
  });

  run("duplicate_override_chain", []() {
    // Multiple successive overrides: last one wins.
    auto root = p("k := 1\nk := 2\nk := 3");
    EXPECT(root->param<int>("k") == 3);
  });

  run("builtin_reregistration_throws", []() {
    EXPECT_THROW(nmhit::TypeRegistry::register_parser<int>([](const std::string &) { return 42; }),
                 nmhit::Error);
  });

  // ── 16. pre/post string arguments ─────────────────────────────────────────

  run("pre_strings_parsed_before_main", []() {
    auto root = nmhit::parse_text("b = 2", {"a = 1"});
    EXPECT(root->param<int>("a") == 1);
    EXPECT(root->param<int>("b") == 2);
  });

  run("post_strings_parsed_after_main", []() {
    auto root = nmhit::parse_text("a = 1", {}, {"b = 2"});
    EXPECT(root->param<int>("a") == 1);
    EXPECT(root->param<int>("b") == 2);
  });

  run("post_override_wins_over_main", []() {
    auto root = nmhit::parse_text("k = 1", {}, {"k := 99"});
    EXPECT(root->param<int>("k") == 99);
    EXPECT(root->children(nmhit::NodeType::Field).size() == 1);
  });

  run("pre_and_post_empty_vectors_are_noop", []() {
    auto root1 = nmhit::parse_text("k = 42");
    auto root2 = nmhit::parse_text("k = 42", {}, {});
    EXPECT(root1->param<int>("k") == root2->param<int>("k"));
    EXPECT(root1->render() == root2->render());
  });

  run("parse_file_reads_from_disk", []() {
    {
      std::ofstream f(g_test_file);
      f << "x = 7\n";
    }
    auto root = nmhit::parse_file(g_test_file);
    EXPECT(root->param<int>("x") == 7);
  });

  run("parse_file_missing_throws",
      []() { EXPECT_THROW(nmhit::parse_file("/nonexistent/path/file.i"), nmhit::Error); });

  // ── 17. Line number tracking ──────────────────────────────────────────────

  run("line_numbers_unquoted_scalar", []() {
    // yyless(0) in <IN_UNQUOTED> used to double-count the terminating character,
    // shifting all subsequent line numbers up by one per unquoted-string field.
    auto root = p("a = hello\nb = world\nc = 3");
    EXPECT(root->find("a")->line() == 1);
    EXPECT(root->find("b")->line() == 2);
    EXPECT(root->find("c")->line() == 3);
  });

  run("line_numbers_sections_and_fields", []() {
    const std::string input = "[Foo]\n"        // 1
                              "  type = Bar\n" // 2
                              "  x = 1\n"      // 3
                              "[]\n"           // 4
                              "[Baz]\n"        // 5
                              "  type = Qux\n" // 6
                              "[]";            // 7
    auto root = p(input);
    EXPECT(root->find("Foo")->line() == 1);
    EXPECT(root->find("Foo/type")->line() == 2);
    EXPECT(root->find("Foo/x")->line() == 3);
    EXPECT(root->find("Baz")->line() == 5);
    EXPECT(root->find("Baz/type")->line() == 6);
  });

  run("line_numbers_with_brace_expr", []() {
    // Unquoted brace expressions also go through IN_UNQUOTED and must not
    // corrupt subsequent line numbers.
    auto root = p("a = ${x}\nb = hello\nc = 3");
    EXPECT(root->find("a")->line() == 1);
    EXPECT(root->find("b")->line() == 2);
    EXPECT(root->find("c")->line() == 3);
  });

  run("line_numbers_multiline_array", []() {
    // Multiline double-quoted array: field location must point to the opening line.
    const std::string input = "models = \"foo\n" // 1
                              "         bar\"\n" // 2
                              "x = 1";           // 3
    auto root = p(input);
    EXPECT(root->find("models")->line() == 1);
    EXPECT(root->find("x")->line() == 3);
  });

  run("line_numbers_brace_expr_multiline_in_array", []() {
    // A ${...} spanning multiple lines inside a quoted array: the token start
    // line must remain at the opening '$', not drift to a later continuation.
    const std::string input = "a = '${foo\n" // 1  — brace expr starts here
                              "bar}'\n"      // 2  — brace expr ends here
                              "b = 1";       // 3
    auto root = p(input);
    EXPECT(root->find("a") != nullptr);
    EXPECT(root->find("b") != nullptr);
    // Field 'a' is on line 1, 'b' on line 3
    EXPECT(root->find("a")->line() == 1);
    EXPECT(root->find("b")->line() == 3);
  });

  // ── 18. Inline comments next to section headers ───────────────────────────

  run("section_trailing_comment", []() {
    auto root = p("[foo] # comment\n[]");
    EXPECT(root->find("foo") != nullptr);
  });

  run("section_trailing_comment_with_fields", []() {
    auto root = p("[foo] # a comment\n  k = 1\n[]");
    EXPECT(root->find("foo/k") != nullptr);
    EXPECT(root->param<int>("foo/k") == 1);
  });

  run("nested_section_trailing_comment", []() {
    auto root = p("[outer]\n  [inner] # nested comment\n  []\n[]");
    EXPECT(root->find("outer/inner") != nullptr);
  });

  run("nested_section_trailing_comment_with_fields", []() {
    auto root = p("[outer] # outer comment\n  [inner] # inner comment\n    v = 7\n  []\n[]");
    EXPECT(root->find("outer/inner/v") != nullptr);
    EXPECT(root->param<int>("outer/inner/v") == 7);
  });

  // ── Summary ───────────────────────────────────────────────────────────────

  std::cerr << "\n=== Results: " << g_passed << " passed, " << g_failed << " failed ===\n";
  return g_failed ? 1 : 0;
}
