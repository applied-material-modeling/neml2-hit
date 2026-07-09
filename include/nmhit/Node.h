#pragma once

#include "nmhit/TypeRegistry.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

namespace nmhit
{

// ─── forward declarations ────────────────────────────────────────────────────

class Node;

// ─── error types ─────────────────────────────────────────────────────────────

struct ErrorMessage
{
  std::filesystem::path filename;
  int line = 0;
  int column = 0;
  std::string message;

  /// Returns "filename:line:col: message" (or just "message" when no location).
  std::string str() const;
};

struct Error : public std::exception
{
  explicit Error(const std::string & msg);
  Error(const std::string & msg, const Node * n);
  Error(const std::vector<ErrorMessage> & msgs);

  const char * what() const noexcept override { return _what.c_str(); }

  std::vector<ErrorMessage> messages;

private:
  std::string _what;
};

// ─── node types ──────────────────────────────────────────────────────────────

/// Every element in a parsed HIT tree.
enum class NodeType
{
  /// Matches any node type (used as a wildcard in children() / find()).
  All,
  /// Root of the document — returned by hit::parse().
  Root,
  /// A section header+body, e.g. [mesh] ... [].
  Section,
  /// A key-value pair, e.g. dim = 3.
  Field,
  /// A comment line, e.g. # text.
  Comment,
  /// A blank line (preserved for round-trip rendering).
  Blank,
};

// ─── Node base class ─────────────────────────────────────────────────────────

/// Base class for all nodes in a parsed HIT tree.
///
/// Ownership: each node owns its children (stored as unique_ptr).  A bare
/// Node* is always a non-owning observer.  Deleting any node recursively
/// deletes its subtree.
class Node
{
public:
  virtual ~Node();

  // ── identity ─────────────────────────────────────────────────────────────

  virtual NodeType type() const = 0;

  /// Local path segment contributed by this node (section name / field name).
  /// Empty for Comment, Blank, and Root.
  virtual std::string path() const { return {}; }

  /// Full slash-joined path from the root to this node.
  std::string fullpath() const;

  // ── source location ───────────────────────────────────────────────────────

  int line() const { return _line; }
  int column() const { return _col; }
  const std::filesystem::path & filename() const { return _filename; }

  /// "filename:line:col"
  std::string file_location() const;

  // ── tree navigation ───────────────────────────────────────────────────────

  Node * parent() const { return _parent; }
  Node * root();
  const Node * root() const;

  /// All direct children, optionally filtered by type.
  /// NodeType::All (default) returns every child.
  std::vector<Node *> children(NodeType filter = NodeType::All);
  std::vector<const Node *> children(NodeType filter = NodeType::All) const;

  /// Follow the given slash-separated relative path downward and return the
  /// first matching node, or nullptr if not found.
  Node * find(const std::string & relpath);
  const Node * find(const std::string & relpath) const;

  // ── typed value access ────────────────────────────────────────────────────

  /// Retrieve a typed value at the given relative path.
  /// The path must resolve to a Field node; throws hit::Error otherwise.
  template <typename T>
  T param(const std::string & relpath = "") const
  {
    const Node * n = relpath.empty() ? this : find(relpath);
    if (!n)
      throw Error("no parameter '" + relpath + "'", this);
    return _param_inner<T>(n);
  }

  /// Like param() but returns default_val when the path doesn't exist.
  template <typename T>
  T param_optional(const std::string & relpath, T default_val) const
  {
    const Node * n = find(relpath);
    if (!n)
      return default_val;
    return _param_inner<T>(n);
  }

  // ── tree mutation ─────────────────────────────────────────────────────────

  /// Transfer ownership of child to this node (appended at the end).
  void add_child(std::unique_ptr<Node> child);

  /// Insert child before the child at position idx.
  void insert_child(std::size_t idx, std::unique_ptr<Node> child);

  /// Remove and return ownership of the given child pointer.
  std::unique_ptr<Node> remove_child(Node * child);

  // ── rendering ────────────────────────────────────────────────────────────

  virtual std::string render(int indent = 0, const std::string & indent_text = "  ") const = 0;

  // ── cloning ──────────────────────────────────────────────────────────────

  virtual std::unique_ptr<Node> clone() const = 0;

  // ── internal setters (used by the parser) ─────────────────────────────────

  void _set_location(const std::filesystem::path & fname, int line, int col);

protected:
  Node() = default;

  // Node owns its children through unique_ptr and is duplicated only via the
  // virtual clone() (which rebuilds from fields, never copy-constructs). The copy
  // operations are deleted so std::is_copy_constructible / is_move_constructible
  // report false on every compiler -- GCC miscomputes them for a protected
  // `= default` copy ctor, and relying on that miscomputation forced the Python
  // bindings into an illegal std::is_move_constructible specialization that newer
  // MSVC rejects. Deleting here removes the need for any trait workaround.
  Node(const Node &) = delete;
  Node & operator=(const Node &) = delete;

private:
  /// Dispatch through TypeRegistry with automatic vector decomposition.
  /// Built-in types (bool, int, etc.) and custom types registered via
  /// TypeRegistry::register_parser<T>() all go through this single path.
  template <typename T>
  T _param_inner(const Node * n) const
  {
    if constexpr (!std::is_same_v<T, std::string>)
      _assert_not_verbatim(n);
    if constexpr (_is_vec<T>::value)
    {
      using Elem = typename T::value_type;
      if constexpr (_is_vec<Elem>::value)
      {
        // 2D: vector<vector<U>>
        using U = typename Elem::value_type;
        auto rows = _row_list(n);
        T result;
        result.reserve(rows.size());
        for (auto & row : rows)
        {
          Elem r;
          r.reserve(row.size());
          for (auto & tok : row)
            r.push_back(TypeRegistry::dispatch<U>(tok, n));
          result.push_back(std::move(r));
        }
        return result;
      }
      else
      {
        // 1D: vector<Elem>
        auto tokens = _token_list(n);
        T result;
        result.reserve(tokens.size());
        for (auto & tok : tokens)
          result.push_back(TypeRegistry::dispatch<Elem>(tok, n));
        return result;
      }
    }
    else
    {
      return TypeRegistry::dispatch<T>(_raw_string(n), n);
    }
  }

  // Helpers used by the _param_inner primary template; defined in Node.cpp.
  static std::string _raw_string(const Node * n);
  static std::vector<std::string> _token_list(const Node * n);
  static std::vector<std::vector<std::string>> _row_list(const Node * n);
  // Throws nmhit::Error if n is a verbatim (triple-quoted) Field.
  static void _assert_not_verbatim(const Node * n);

  Node * _parent = nullptr;
  std::vector<std::unique_ptr<Node>> _children;

  std::filesystem::path _filename;
  int _line = 0;
  int _col = 0;

  friend class Section; // allow build_section helpers to reparent
};

// ─── concrete node types ──────────────────────────────────────────────────────

/// Root document node.  Contains top-level sections, fields, comments, blanks.
class Root : public Node
{
public:
  Root() = default;
  NodeType type() const override { return NodeType::Root; }
  std::string render(int indent = 0, const std::string & indent_text = "  ") const override;
  std::unique_ptr<Node> clone() const override;
};

/// A section, e.g. [mesh] ... [].
class Section : public Node
{
public:
  explicit Section(const std::string & name);
  NodeType type() const override { return NodeType::Section; }
  std::string path() const override { return _name; }
  std::string render(int indent = 0, const std::string & indent_text = "  ") const override;
  std::unique_ptr<Node> clone() const override;

  /// True if this section was synthesized by the parser to wrap a path-split
  /// key (e.g. the "Models" in `Models/a/foo = 1`), rather than being written
  /// explicitly by the user as `[Models] ... []`.  Wrapper sections are merged
  /// with same-name siblings at parse time; explicit sections are not.
  bool is_path_wrapper() const { return _is_wrapper; }

  // ── internal setter (used by the parser) ──────────────────────────────────
  void _set_path_wrapper(bool v) { _is_wrapper = v; }

private:
  std::string _name;
  bool _is_wrapper = false;
};

/// A key-value field, e.g. dim = 3 or values = '1 2 3'.
class Field : public Node
{
public:
  Field(const std::string & name,
        const std::string & raw_value,
        bool verbatim = false,
        char verbatim_delim = '\'');

  NodeType type() const override { return NodeType::Field; }
  std::string path() const override { return _name; }
  std::string render(int indent = 0, const std::string & indent_text = "  ") const override;
  std::unique_ptr<Node> clone() const override;

  /// The raw stored value string (may contain quotes for string/array values).
  const std::string & raw_val() const { return _raw; }

  /// Change the stored value.
  void set_val(const std::string & value);

  /// True when this field was set via a triple-quoted verbatim string ('''...''' or """...""").
  /// Such fields can only be read via param<std::string>() / param_str().
  bool is_verbatim() const { return _verbatim; }

  /// Which delimiter the verbatim body was wrapped in originally: '\'' or '"'.
  /// Meaningful only when is_verbatim() is true.
  char verbatim_delim() const { return _verbatim_delim; }

private:
  std::string _name;
  std::string _raw;
  bool _verbatim = false;
  char _verbatim_delim = '\'';
};

/// A comment, e.g. # some text.
class Comment : public Node
{
public:
  explicit Comment(const std::string & text, bool is_inline = false);
  NodeType type() const override { return NodeType::Comment; }
  std::string render(int indent = 0, const std::string & indent_text = "  ") const override;
  std::unique_ptr<Node> clone() const override;

  const std::string & text() const { return _text; }
  bool is_inline() const { return _is_inline; }
  void set_inline(bool v) { _is_inline = v; }

private:
  std::string _text;
  // Named _is_inline, not _inline: `_inline` is a reserved keyword under MSVC
  // (a Microsoft dialect alias for `inline`), so a member of that name fails to
  // compile with cl.exe.
  bool _is_inline;
};

/// A blank line (preserved for round-trip rendering).
class Blank : public Node
{
public:
  NodeType type() const override { return NodeType::Blank; }
  std::string render(int /*indent*/ = 0, const std::string & /*indent_text*/ = "  ") const override
  {
    return "\n";
  }
  std::unique_ptr<Node> clone() const override { return std::make_unique<Blank>(); }
};

} // namespace nmhit
