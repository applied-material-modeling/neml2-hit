#pragma once

#include <string>
#include <vector>

namespace nmhit
{

class Node;

// ─── BraceNode ────────────────────────────────────────────────────────────────

/// Represents one parsed ${...} expression (possibly with nested sub-expressions).
///
/// Syntax recognised:
///   ${varname}           → command="", args=["varname"]
///   ${env VARNAME}       → command="env", args=["VARNAME"]
///   ${raw a b c}         → command="raw", args=["a","b","c"]
///   ${replace varname}   → command="replace", args=["varname"]
///   ${cmd ${nested}}     → command="cmd", subnodes=[parsed nested expr]
///
/// The top-level text passed to parse_brace_expr() must include the enclosing
/// "${" and "}" delimiters.
struct BraceNode
{
  /// The command keyword ("", "env", "raw", "replace").
  std::string command;
  /// Literal string arguments.
  std::vector<std::string> args;
  /// Nested ${...} sub-expressions (appear in argument position).
  std::vector<BraceNode> subnodes;
};

// ─── Evaluator interface ──────────────────────────────────────────────────────

/// Override to customise brace expression evaluation.
struct Evaluator
{
  virtual ~Evaluator() = default;
  virtual std::string evaluate(const BraceNode & node, const Node * context) = 0;
};

// ─── free functions ───────────────────────────────────────────────────────────

/// Parse a brace expression string (must start with "${" and end with "}").
/// Returns the parsed BraceNode.
/// @throws hit::Error on malformed input.
BraceNode parse_brace_expr(const std::string & text);

/// Expand a parsed brace expression to a string.
/// When eval == nullptr the built-in evaluator is used (env, raw, replace,
/// and plain parameter lookup).
/// @param node     the parsed expression
/// @param context  node used for parameter look-ups (may be nullptr)
/// @param eval     custom evaluator, or nullptr for the built-in one
std::string
eval_brace_expr(const BraceNode & node, const Node * context, Evaluator * eval = nullptr);

/// Convenience: parse then evaluate a raw "${...}" string.
std::string
expand_brace_expr(const std::string & text, const Node * context, Evaluator * eval = nullptr);

/// Returns true if the string contains at least one "${" sequence.
bool has_brace_expr(const std::string & s);

} // namespace nmhit
