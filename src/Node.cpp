#include "nmhit/Node.h"
#include "nmhit/BraceExpr.h"
#include "ParseDriver.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

// ─── forward declarations for Flex-generated C++ scanner functions ────────────
// Lexer.cpp is compiled as C++ (not C), so no extern "C" wrapper.
struct yy_buffer_state;
int HITlex(void * yyscanner);
int HITlex_init_extra(nmhit_detail::ParseDriver * extra, void ** scanner);
int HITlex_destroy(void * yyscanner);
yy_buffer_state * HIT_scan_bytes(const char * bytes, int len, void * yyscanner);
void HIT_delete_buffer(yy_buffer_state * b, void * yyscanner);

namespace nmhit
{

// ═══════════════════════════════════════════════════════════════════════════════
// ErrorMessage + Error
// ═══════════════════════════════════════════════════════════════════════════════

std::string
ErrorMessage::str() const
{
  std::ostringstream ss;
  if (!filename.empty())
  {
    ss << filename.string();
    if (line > 0)
    {
      ss << ':' << line;
      if (column > 0)
        ss << ':' << column;
    }
    ss << ": ";
  }
  ss << message;
  return ss.str();
}

static std::string
join_errors(const std::vector<ErrorMessage> & msgs)
{
  std::ostringstream ss;
  for (std::size_t i = 0; i < msgs.size(); ++i)
  {
    if (i)
      ss << '\n';
    ss << msgs[i].str();
  }
  return ss.str();
}

Error::Error(const std::string & msg) : _what(msg)
{
  messages.push_back({/*filename=*/"", /*line=*/0, /*column=*/0, msg});
}

Error::Error(const std::string & msg, const Node * n) : Error(msg)
{
  if (n)
  {
    messages[0].filename = n->filename();
    messages[0].line = n->line();
    messages[0].column = n->column();
    _what = messages[0].str();
  }
}

Error::Error(const std::vector<ErrorMessage> & msgs) : messages(msgs), _what(join_errors(msgs)) {}

// ═══════════════════════════════════════════════════════════════════════════════
// Node base class
// ═══════════════════════════════════════════════════════════════════════════════

Node::~Node() = default;

void
Node::_set_location(const std::filesystem::path & fname, int line, int col)
{
  _filename = fname;
  _line = line;
  _col = col;
}

std::string
Node::fullpath() const
{
  std::string seg = path();
  if (!_parent || _parent->type() == NodeType::Root)
    return seg;

  std::string p = _parent->fullpath();
  if (p.empty())
    return seg;
  if (seg.empty())
    return p;
  return p + '/' + seg;
}

std::string
Node::file_location() const
{
  std::ostringstream ss;
  ss << _filename.string() << ':' << _line << ':' << _col;
  return ss.str();
}

Node *
Node::root()
{
  Node * n = this;
  while (n->_parent)
    n = n->_parent;
  return n;
}

const Node *
Node::root() const
{
  const Node * n = this;
  while (n->_parent)
    n = n->_parent;
  return n;
}

std::vector<Node *>
Node::children(NodeType filter)
{
  std::vector<Node *> result;
  result.reserve(_children.size());
  for (auto & c : _children)
  {
    if (filter == NodeType::All || c->type() == filter)
      result.push_back(c.get());
  }
  return result;
}

std::vector<const Node *>
Node::children(NodeType filter) const
{
  std::vector<const Node *> result;
  result.reserve(_children.size());
  for (auto & c : _children)
  {
    if (filter == NodeType::All || c->type() == filter)
      result.push_back(c.get());
  }
  return result;
}

Node *
Node::find(const std::string & relpath)
{
  if (relpath.empty())
    return this;

  auto slash = relpath.find('/');
  std::string head = (slash == std::string::npos) ? relpath : relpath.substr(0, slash);
  std::string tail = (slash == std::string::npos) ? "" : relpath.substr(slash + 1);

  for (auto & c : _children)
  {
    if (c->path() == head)
    {
      if (tail.empty())
        return c.get();
      return c->find(tail);
    }
  }
  return nullptr;
}

const Node *
Node::find(const std::string & relpath) const
{
  return const_cast<Node *>(this)->find(relpath);
}

void
Node::add_child(std::unique_ptr<Node> child)
{
  child->_parent = this;
  _children.push_back(std::move(child));
}

void
Node::insert_child(std::size_t idx, std::unique_ptr<Node> child)
{
  child->_parent = this;
  _children.insert(_children.begin() + static_cast<std::ptrdiff_t>(idx), std::move(child));
}

std::unique_ptr<Node>
Node::remove_child(Node * child)
{
  for (auto it = _children.begin(); it != _children.end(); ++it)
  {
    if (it->get() == child)
    {
      auto removed = std::move(*it);
      removed->_parent = nullptr;
      _children.erase(it);
      return removed;
    }
  }
  return nullptr;
}

// ── TypeRegistry helpers ──────────────────────────────────────────────────────

// Forward declarations needed by TypeRegistry::_store() initialiser.
static bool parse_bool(const std::string & raw, const Node * n);
static int64_t parse_int(const std::string & raw, const Node * n);
static double parse_double(const std::string & raw, const Node * n);

std::unordered_map<std::type_index, std::function<std::any(const std::string &, const Node *)>> &
TypeRegistry::_store()
{
  static auto m = []()
  {
    std::unordered_map<std::type_index, std::function<std::any(const std::string &, const Node *)>>
      map;

    map[std::type_index(typeid(bool))] =
      [](const std::string & s, const Node * n) -> std::any { return parse_bool(s, n); };

    map[std::type_index(typeid(int))] =
      [](const std::string & s, const Node * n) -> std::any
    {
      int64_t v = parse_int(s, n);
      if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max())
        throw Error("'" + s + "' overflows int", n);
      return static_cast<int>(v);
    };

    map[std::type_index(typeid(unsigned int))] =
      [](const std::string & s, const Node * n) -> std::any
    {
      int64_t v = parse_int(s, n);
      if (v < 0)
        throw Error("'" + s + "' cannot be read as unsigned int (value is negative)", n);
      if (v > static_cast<int64_t>(std::numeric_limits<unsigned int>::max()))
        throw Error("'" + s + "' overflows unsigned int", n);
      return static_cast<unsigned int>(v);
    };

    map[std::type_index(typeid(int64_t))] =
      [](const std::string & s, const Node * n) -> std::any { return parse_int(s, n); };

    map[std::type_index(typeid(double))] =
      [](const std::string & s, const Node * n) -> std::any { return parse_double(s, n); };

    map[std::type_index(typeid(float))] =
      [](const std::string & s, const Node * n) -> std::any
    { return static_cast<float>(parse_double(s, n)); };

    map[std::type_index(typeid(std::string))] =
      [](const std::string & s, const Node *) -> std::any { return s; };

    return map;
  }();
  return m;
}

void
TypeRegistry::_throw_unregistered(const char * type_name)
{
  throw Error(std::string("nmhit::TypeRegistry: no parser registered for type '") + type_name +
              "'. Call nmhit::TypeRegistry::register_parser<T>() before using param<T>().");
}

void
TypeRegistry::_throw_already_registered(const char * type_name)
{
  throw Error(std::string("nmhit::TypeRegistry: type '") + type_name +
              "' is already registered (built-in types cannot be re-registered).");
}

// ── param<T> helpers ─────────────────────────────────────────────────────────

/// Require that n is a Field; throw if not.
static const Field *
as_field(const Node * n)
{
  auto * f = dynamic_cast<const Field *>(n);
  if (!f)
    throw Error("node has no value", n);
  return f;
}

/// Strip surrounding single/double quotes from a raw field value string.
static std::string
field_unquote(const std::string & raw)
{
  if (raw.size() >= 2 &&
      ((raw.front() == '"' && raw.back() == '"') || (raw.front() == '\'' && raw.back() == '\'')))
    return raw.substr(1, raw.size() - 2);
  return raw;
}

/// Parse a boolean from a raw value string.
static bool
parse_bool(const std::string & raw, const Node * n)
{
  std::string v = field_unquote(raw);
  if (v == "true")
    return true;
  if (v == "false")
    return false;
  throw Error("'" + raw + "' is not a boolean value (expected 'true' or 'false')", n);
}

/// Parse an integer from a raw value string.
static int64_t
parse_int(const std::string & raw, const Node * n)
{
  try
  {
    std::size_t pos;
    std::string u = field_unquote(raw);
    int64_t v = std::stoll(u, &pos);
    if (pos != u.size())
      throw Error("'" + raw + "' is not a valid integer", n);
    return v;
  }
  catch (const Error &)
  {
    throw;
  }
  catch (...)
  {
    throw Error("'" + raw + "' is not a valid integer", n);
  }
}

/// Parse a double from a raw value string.
static double
parse_double(const std::string & raw, const Node * n)
{
  try
  {
    std::size_t pos;
    std::string u = field_unquote(raw);
    double v = std::stod(u, &pos);
    if (pos != u.size())
      throw Error("'" + raw + "' is not a valid number", n);
    return v;
  }
  catch (const Error &)
  {
    throw;
  }
  catch (...)
  {
    throw Error("'" + raw + "' is not a valid number", n);
  }
}

/// Return the string value of a raw field string (strips quotes, expands braces).
static std::string
parse_string(const std::string & raw, const Node * n)
{
  std::string v = field_unquote(raw);
  if (has_brace_expr(v))
    return expand_brace_expr(v, n);
  return v;
}

/// Split a (possibly quoted) raw array value into wnmhitespace-delimited tokens.
static std::vector<std::string>
field_split_tokens(const std::string & raw, const Node * n)
{
  std::string content = field_unquote(raw);
  if (has_brace_expr(content))
    content = expand_brace_expr(content, n);
  std::vector<std::string> tokens;
  std::istringstream ss(content);
  std::string tok;
  while (ss >> tok)
    tokens.push_back(tok);
  return tokens;
}

/// Split a (possibly quoted) 2D array value: ';' separates rows, wnmhitespace separates columns.
/// Empty rows (e.g. from trailing ';') are silently dropped.
static std::vector<std::vector<std::string>>
field_split_rows(const std::string & raw, const Node * n)
{
  std::string content = field_unquote(raw);
  if (has_brace_expr(content))
    content = expand_brace_expr(content, n);
  std::vector<std::vector<std::string>> rows;
  std::istringstream outer(content);
  std::string row_str;
  while (std::getline(outer, row_str, ';'))
  {
    std::vector<std::string> row;
    std::istringstream inner(row_str);
    std::string tok;
    while (inner >> tok)
      row.push_back(tok);
    if (!row.empty())
      rows.push_back(std::move(row));
  }
  return rows;
}

// ── Node private statics for _param_inner primary template ───────────────────

std::string
Node::_raw_string(const Node * n)
{
  return parse_string(as_field(n)->raw_val(), n);
}

std::vector<std::string>
Node::_token_list(const Node * n)
{
  return field_split_tokens(as_field(n)->raw_val(), n);
}

std::vector<std::vector<std::string>>
Node::_row_list(const Node * n)
{
  return field_split_rows(as_field(n)->raw_val(), n);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Root
// ═══════════════════════════════════════════════════════════════════════════════

std::string
Root::render(int indent, const std::string & indent_text) const
{
  std::string out;
  for (const auto & c : children())
    out += c->render(indent, indent_text);
  return out;
}

std::unique_ptr<Node>
Root::clone() const
{
  auto r = std::make_unique<Root>();
  for (const auto & c : children())
    r->add_child(c->clone());
  return r;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Section
// ═══════════════════════════════════════════════════════════════════════════════

Section::Section(const std::string & name) : _name(name) {}

std::string
Section::render(int indent, const std::string & indent_text) const
{
  std::string pfx;
  for (int i = 0; i < indent; ++i)
    pfx += indent_text;

  std::string out = pfx + '[' + _name + "]\n";
  for (const auto & c : children())
    out += c->render(indent + 1, indent_text);
  out += pfx + "[]\n";
  return out;
}

std::unique_ptr<Node>
Section::clone() const
{
  auto s = std::make_unique<Section>(_name);
  s->_set_location(filename(), line(), column());
  for (const auto & c : children())
    s->add_child(c->clone());
  return s;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Field
// ═══════════════════════════════════════════════════════════════════════════════

Field::Field(const std::string & name, const std::string & raw_value)
  : _name(name), _raw(raw_value)
{}

void
Field::set_val(const std::string & value)
{
  _raw = value;
}

std::string
Field::render(int indent, const std::string & indent_text) const
{
  std::string pfx;
  for (int i = 0; i < indent; ++i)
    pfx += indent_text;
  return pfx + _name + " = " + _raw + "\n";
}

std::unique_ptr<Node>
Field::clone() const
{
  auto f = std::make_unique<Field>(_name, _raw);
  f->_set_location(filename(), line(), column());
  return f;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Comment
// ═══════════════════════════════════════════════════════════════════════════════

Comment::Comment(const std::string & text, bool is_inline) : _text(text), _inline(is_inline) {}

std::string
Comment::render(int indent, const std::string & indent_text) const
{
  if (_inline)
    return " " + _text + "\n";

  std::string pfx;
  for (int i = 0; i < indent; ++i)
    pfx += indent_text;
  return pfx + _text + "\n";
}

std::unique_ptr<Node>
Comment::clone() const
{
  auto c = std::make_unique<Comment>(_text, _inline);
  c->_set_location(filename(), line(), column());
  return c;
}

} // namespace nmhit

// ═══════════════════════════════════════════════════════════════════════════════
// ParseDriver
// ═══════════════════════════════════════════════════════════════════════════════

namespace nmhit_detail
{

// ── Include node (internal placeholder) ─────────────────────────────────────

/// Temporary node that carries the include path.  Replaced by the included
/// content during the post-parse include-resolution step.
class IncludeNode : public nmhit::Node
{
public:
  explicit IncludeNode(const std::string & path) : _path(path) {}
  nmhit::NodeType type() const override { return nmhit::NodeType::Blank; /* internal */ }
  std::string render(int, const std::string &) const override { return ""; }
  std::unique_ptr<nmhit::Node> clone() const override { return std::make_unique<IncludeNode>(_path); }
  const std::string & include_path() const { return _path; }

private:
  std::string _path;
};

// ── Constructor / destructor ──────────────────────────────────────────────────

ParseDriver::ParseDriver(const std::filesystem::path & fname, const std::string & input)
  : _fname(fname), _input(input)
{}

ParseDriver::~ParseDriver()
{
  if (_scanner)
    HITlex_destroy(_scanner);
}

// ── Location tracking ─────────────────────────────────────────────────────────

void
ParseDriver::on_token_begin()
{
  _tok_start_line = _line;
  _tok_start_col = _col;
}

void
ParseDriver::on_token_text(const char * text, int len)
{
  for (int i = 0; i < len; ++i)
  {
    if (text[i] == '\n')
    {
      ++_line;
      _col = 1;
    }
    else
    {
      ++_col;
    }
  }
}

// ── lex() — called by the Bison parser ───────────────────────────────────────

int
ParseDriver::lex(Parser::semantic_type * yylval, Parser::location_type * yylloc)
{
  _pending.clear();

  int tok = HITlex(_scanner);

  yylloc->begin.line = _tok_start_line;
  yylloc->begin.column = _tok_start_col;
  yylloc->end.line = _line;
  yylloc->end.column = _col;

  if (!_pending.empty())
    yylval->emplace<std::string>(std::move(_pending));

  return tok;
}

// ── Error reporting ───────────────────────────────────────────────────────────

void
ParseDriver::report_error(const Parser::location_type & loc, const std::string & msg)
{
  nmhit::ErrorMessage em;
  em.filename = _fname;
  em.line = loc.begin.line;
  em.column = loc.begin.column;
  em.message = msg;
  _errors.push_back(em);
  _failed = true;
}

void
ParseDriver::lex_error(const char * text, int /*line*/)
{
  std::string msg = std::string("unexpected character '") + text + "'";
  nmhit::ErrorMessage em{_fname, _tok_start_line, _tok_start_col, msg};
  _errors.push_back(em);
  _failed = true;
}

void
ParseDriver::lex_error(const std::string & msg, int /*line*/)
{
  nmhit::ErrorMessage em{_fname, _tok_start_line, _tok_start_col, msg};
  _errors.push_back(em);
  _failed = true;
}

// ── parse() ──────────────────────────────────────────────────────────────────

bool
ParseDriver::parse()
{
  HITlex_init_extra(this, &_scanner);
  auto * buf = HIT_scan_bytes(_input.c_str(), static_cast<int>(_input.size()), _scanner);

  Parser parser(*this);
  int rc = parser.parse();

  HIT_delete_buffer(buf, _scanner);
  HITlex_destroy(_scanner);
  _scanner = nullptr;

  return rc == 0 && !_failed;
}

// ── Tree-building helpers ─────────────────────────────────────────────────────

/// Apply last-override-wins semantics to items, merging same-name sections.
///
/// Pass 1 — section merge: sections with the same name are collapsed into the
///   first occurrence by moving all children of the duplicate into the first.
///   This handles path-split fields that share a common ancestor section
///   (e.g. "Models/a/foo = 1" and "Models/b/bar = 2" both create a "Models"
///   wrapper; the two wrappers are merged so lookup through a single "Models"
///   finds both).
///
/// Pass 2 — field duplicate / override check (per level):
///   - If the later occurrence was built with ':=', the earlier one is removed.
///   - If the later occurrence was built with '=', an error is recorded.
///
/// Pass 3 — recurse into each remaining section to apply the same logic to
///   its children (needed so that path-split overrides like "a/k = 1" followed
///   by "a/k := 2" are resolved inside the merged "a" section).
void
ParseDriver::apply_overrides(std::vector<std::unique_ptr<nmhit::Node>> & items)
{
  // ── Pass 1: merge same-name sections ─────────────────────────────────────
  {
    std::unordered_map<std::string, std::size_t> section_first;
    std::vector<bool> merged(items.size(), false);

    for (std::size_t i = 0; i < items.size(); ++i)
    {
      auto * sec = dynamic_cast<nmhit::Section *>(items[i].get());
      if (!sec)
        continue;

      auto it = section_first.find(sec->path());
      if (it != section_first.end())
      {
        auto * first = dynamic_cast<nmhit::Section *>(items[it->second].get());
        auto raw_kids = sec->children();
        for (auto * k : raw_kids)
          first->add_child(sec->remove_child(k));
        merged[i] = true;
      }
      else
      {
        section_first[sec->path()] = i;
      }
    }

    if (std::any_of(merged.begin(), merged.end(), [](bool b) { return b; }))
    {
      std::vector<std::unique_ptr<nmhit::Node>> kept;
      kept.reserve(items.size());
      for (std::size_t i = 0; i < items.size(); ++i)
        if (!merged[i])
          kept.push_back(std::move(items[i]));
      items = std::move(kept);
    }
  }

  // ── Pass 2: field duplicate / override check at this level ───────────────
  {
    std::unordered_map<std::string, std::size_t> first_idx;
    std::vector<bool> removed(items.size(), false);

    for (std::size_t i = 0; i < items.size(); ++i)
    {
      const auto * f = dynamic_cast<const nmhit::Field *>(items[i].get());
      if (!f)
        continue;
      const std::string & name = f->path();
      auto it = first_idx.find(name);
      if (it != first_idx.end())
      {
        if (!_override_fields.count(f))
        {
          _errors.push_back({f->filename(),
                             f->line(),
                             f->column(),
                             "duplicate field '" + name + "': use ':=' to override"});
          _failed = true;
        }
        else
        {
          removed[it->second] = true;
          it->second = i;
        }
      }
      else
      {
        first_idx[name] = i;
      }
    }

    if (std::any_of(removed.begin(), removed.end(), [](bool b) { return b; }))
    {
      std::vector<std::unique_ptr<nmhit::Node>> kept;
      kept.reserve(items.size());
      for (std::size_t i = 0; i < items.size(); ++i)
        if (!removed[i])
          kept.push_back(std::move(items[i]));
      items = std::move(kept);
    }
  }

  // ── Pass 3: recurse into each section ────────────────────────────────────
  for (auto & item : items)
  {
    auto * sec = dynamic_cast<nmhit::Section *>(item.get());
    if (!sec)
      continue;

    auto raw_kids = sec->children();
    std::vector<std::unique_ptr<nmhit::Node>> children;
    children.reserve(raw_kids.size());
    for (auto * k : raw_kids)
      children.push_back(sec->remove_child(k));
    apply_overrides(children);
    for (auto & child : children)
      sec->add_child(std::move(child));
  }
}

/// Split a slash-delimited path into its segments.
static std::vector<std::string>
split_path(const std::string & path)
{
  std::vector<std::string> segs;
  std::size_t start = 0;
  while (true)
  {
    auto slash = path.find('/', start);
    std::string seg = path.substr(start, slash == std::string::npos ? slash : slash - start);
    if (!seg.empty())
      segs.push_back(seg);
    if (slash == std::string::npos)
      break;
    start = slash + 1;
  }
  return segs;
}

/// Wrap a node in a chain of Section nodes for a/b/c-style paths.
static std::unique_ptr<nmhit::Node>
wrap_in_sections(std::vector<std::string> segs,
                 std::unique_ptr<nmhit::Node> inner_node,
                 const std::string & fname,
                 int line,
                 int col)
{
  if (segs.size() <= 1)
    return inner_node;

  for (int i = static_cast<int>(segs.size()) - 2; i >= 0; --i)
  {
    auto wrapper = std::make_unique<nmhit::Section>(segs[i]);
    wrapper->_set_location(fname, line, col);
    wrapper->add_child(std::move(inner_node));
    inner_node = std::move(wrapper);
  }
  return inner_node;
}

void
ParseDriver::set_root(std::vector<std::unique_ptr<nmhit::Node>> items)
{
  apply_overrides(items);
  auto root = std::make_unique<nmhit::Root>();
  for (auto & item : items)
    root->add_child(std::move(item));
  _root = std::move(root);
}

std::unique_ptr<nmhit::Node>
ParseDriver::build_section(const std::string & path,
                           std::vector<std::unique_ptr<nmhit::Node>> items,
                           const Parser::location_type & loc)
{
  apply_overrides(items);
  auto segs = split_path(path);
  if (segs.empty())
    segs.push_back(path);

  auto section = std::make_unique<nmhit::Section>(segs.back());
  section->_set_location(_fname, loc.begin.line, loc.begin.column);
  for (auto & item : items)
    section->add_child(std::move(item));

  return wrap_in_sections(segs, std::move(section), _fname, loc.begin.line, loc.begin.column);
}

std::unique_ptr<nmhit::Node>
ParseDriver::build_field(const std::string & name,
                         bool is_override,
                         const std::string & raw_value,
                         const Parser::location_type & loc)
{
  auto segs = split_path(name);
  if (segs.empty())
    segs.push_back(name);

  auto field = std::make_unique<nmhit::Field>(segs.back(), raw_value);
  field->_set_location(_fname, loc.begin.line, loc.begin.column);
  if (is_override)
    _override_fields.insert(field.get());

  return wrap_in_sections(segs, std::move(field), _fname, loc.begin.line, loc.begin.column);
}

std::unique_ptr<nmhit::Node>
ParseDriver::build_include(const std::string & path, const Parser::location_type & loc)
{
  auto node = std::make_unique<IncludeNode>(path);
  node->_set_location(_fname, loc.begin.line, loc.begin.column);
  return node;
}

std::string
ParseDriver::build_array_value(std::vector<std::unique_ptr<nmhit::Node>> elems)
{
  std::string out = "'";
  bool first_in_row = true;
  for (auto & e : elems)
  {
    if (!e)
    {
      // nullptr sentinel: row separator
      out += "; ";
      first_in_row = true;
    }
    else
    {
      if (!first_in_row)
        out += ' ';
      first_in_row = false;
      auto * f = dynamic_cast<nmhit::Field *>(e.get());
      if (f)
        out += f->raw_val();
    }
  }
  out += '\'';
  return out;
}

} // namespace nmhit_detail

// ═══════════════════════════════════════════════════════════════════════════════
// nmhit::parse()
// ═══════════════════════════════════════════════════════════════════════════════

namespace nmhit
{

/// Run the Flex/Bison pipeline on @p input and return the root node.
/// Does not resolve !include directives.
static std::unique_ptr<Node>
parse_raw(const std::filesystem::path & fname, const std::string & input)
{
  nmhit_detail::ParseDriver driver(fname, input);
  bool ok = driver.parse();
  if (!ok)
  {
    auto & errs = driver.errors();
    if (errs.empty())
      throw Error("parse failed (unknown error)", nullptr);
    throw Error(errs);
  }
  return driver.release_root();
}

/// Resolve !include directives by recursively parsing referenced files.
static void
resolve_includes(Node * node, const std::filesystem::path & base_dir)
{
  for (auto * child : node->children())
    if (child->type() == NodeType::Section || child->type() == NodeType::Root)
      resolve_includes(child, base_dir);

  while (true)
  {
    auto kids = node->children();
    nmhit_detail::IncludeNode * inc = nullptr;
    std::size_t pos = 0;
    for (std::size_t i = 0; i < kids.size(); ++i)
    {
      inc = dynamic_cast<nmhit_detail::IncludeNode *>(kids[i]);
      if (inc)
      {
        pos = i;
        break;
      }
    }
    if (!inc)
      break;

    std::filesystem::path inc_path = inc->include_path();
    std::filesystem::path resolved = inc_path.is_absolute() ? inc_path : base_dir / inc_path;

    std::ifstream ifs(resolved);
    if (!ifs.is_open())
    {
      ErrorMessage em{inc->filename(),
                      inc->line(),
                      inc->column(),
                      "cannot open include file '" + inc->include_path() + "'"};
      throw Error({em});
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto included = parse_raw(resolved, content);
    resolve_includes(included.get(), resolved.parent_path());

    auto inc_kids = included->children();
    std::vector<std::unique_ptr<Node>> to_insert;
    to_insert.reserve(inc_kids.size());
    for (auto * k : inc_kids)
      to_insert.push_back(included->remove_child(k));

    node->remove_child(inc);

    for (std::size_t j = 0; j < to_insert.size(); ++j)
      node->insert_child(pos + j, std::move(to_insert[j]));
  }
}

std::unique_ptr<Node>
parse_file(const std::filesystem::path & fname,
           const std::vector<std::string> & pre,
           const std::vector<std::string> & post)
{
  std::ifstream ifs(fname);
  if (!ifs.is_open())
    throw Error("cannot open file '" + fname.string() + "'");

  std::string input((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

  std::string full;
  for (const auto & s : pre)
    full += s + '\n';
  full += input;
  for (const auto & s : post)
    full += '\n' + s;

  auto root = parse_raw(fname, full);
  resolve_includes(root.get(), fname.parent_path());
  return root;
}

std::unique_ptr<Node>
parse_text(const std::string & input,
           const std::vector<std::string> & pre,
           const std::vector<std::string> & post)
{
  std::string full;
  for (const auto & s : pre)
    full += s + '\n';
  full += input;
  for (const auto & s : post)
    full += '\n' + s;

  auto root = parse_raw({}, full);
  resolve_includes(root.get(), std::filesystem::current_path());
  return root;
}

} // namespace nmhit
