#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "nmhit/Node.h"
#include "Parser.h" // Bison-generated: defines nmhit_detail::Parser and its token enum

// Flex reentrant scanner handle.
typedef void * yyscan_t;

namespace nmhit_detail
{

/// ParseDriver manages a single parse run.  It bridges the Flex scanner and
/// the Bison parser and builds the AST.
class ParseDriver
{
public:
  ParseDriver(const std::filesystem::path & fname, const std::string & input);
  ~ParseDriver();

  /// Run the full parse.  Returns true on success, false if errors occurred.
  bool parse();

  // ── Interface called by the Bison parser ───────────────────────────────

  /// Called via `#define yylex driver.lex` in Parser.y.
  int lex(Parser::semantic_type * yylval, Parser::location_type * yylloc);

  /// Called by the generated Parser::error() function.
  void report_error(const Parser::location_type & loc, const std::string & msg);

  // ── Interface called by YY_USER_ACTION in Lexer.l ─────────────────────

  /// Record the cursor position at the START of the current token.
  void on_token_begin();

  /// Advance the tracked cursor position past the matched text.
  void on_token_text(const char * text, int len);

  // ── Interface called from Flex action code ────────────────────────────

  /// Store a pending string value (full yytext) for the next lex() call.
  void set_pending(const char * text, int len) { _pending.assign(text, len); }

  /// Store a pending string with explicit length (used for trimmed strings).
  void set_pending_n(const char * text, int len) { _pending.assign(text, len); }

  /// Report a lexer error and mark the parse as failed.
  void lex_error(const char * text, int line);
  void lex_error(const std::string & msg, int line);

  // ── Brace-expression nesting depth ────────────────────────────────────

  void begin_brace() { ++_brace_depth; }

  /// Decrement depth.  Returns true when the OUTERMOST brace has been closed.
  bool end_brace()
  {
    if (_brace_depth > 0)
      --_brace_depth;
    return _brace_depth == 0;
  }

  // ── Tree-building helpers (called from grammar actions in Parser.y) ────

  /// Called with the top-level items after parsing completes.
  void set_root(std::vector<std::unique_ptr<nmhit::Node>> items);

  /// Build a Section node (handling 'a/b/c' path splitting).
  std::unique_ptr<nmhit::Node> build_section(const std::string & path,
                                             std::vector<std::unique_ptr<nmhit::Node>> items,
                                             const Parser::location_type & loc);

  /// Build a Field node (handling 'a/b' path splitting and override flag).
  std::unique_ptr<nmhit::Node> build_field(const std::string & name,
                                           bool is_override,
                                           const std::string & raw_value,
                                           const Parser::location_type & loc);

  /// Build a placeholder Include node; file loading happens in nmhit::parse().
  std::unique_ptr<nmhit::Node> build_include(const std::string & path,
                                             const Parser::location_type & loc);

  /// Concatenate array element raw values into a single-quoted string.
  std::string build_array_value(std::vector<std::unique_ptr<nmhit::Node>> elems);

  // ── Results ───────────────────────────────────────────────────────────

  std::unique_ptr<nmhit::Node> release_root() { return std::move(_root); }
  const std::filesystem::path & filename() const { return _fname; }
  bool failed() const { return _failed; }
  const std::vector<nmhit::ErrorMessage> & errors() const { return _errors; }

private:
  /// Validate and apply last-override-wins: removes earlier occurrences of
  /// fields that are subsequently overridden with ':=', and emits an error
  /// for any duplicate that does not use ':='.
  void apply_overrides(std::vector<std::unique_ptr<nmhit::Node>> & items);
  std::filesystem::path _fname;
  std::string _input;

  // Flex scanner state
  yyscan_t _scanner = nullptr;

  // Source location tracking
  int _line = 1;
  int _col = 1;
  int _tok_start_line = 1;
  int _tok_start_col = 1;

  // Pending string value for the current token
  std::string _pending;

  // Brace expression nesting counter
  int _brace_depth = 0;

  // Set of Field nodes constructed with the ':=' operator.
  // Populated in build_field(); consulted in apply_overrides().
  std::unordered_set<const nmhit::Field *> _override_fields;

  // Result
  std::unique_ptr<nmhit::Node> _root;
  bool _failed = false;
  std::vector<nmhit::ErrorMessage> _errors;
};

} // namespace nmhit_detail
