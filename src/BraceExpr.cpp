#include "nmhit/BraceExpr.h"
#include "nmhit/Node.h"

#include <cstdlib>
#include <sstream>
#include <stdexcept>

namespace nmhit
{

// ═══════════════════════════════════════════════════════════════════════════════
// Parsing
// ═══════════════════════════════════════════════════════════════════════════════

bool
has_brace_expr(const std::string & s)
{
  auto pos = s.find("${");
  return pos != std::string::npos;
}

/// Skip wnmhitespace starting at pos, return the new pos.
static std::size_t
skip_ws(const std::string & s, std::size_t pos)
{
  while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n'))
    ++pos;
  return pos;
}

/// Parse one BraceNode starting at pos (which should point at '$').
/// Returns the position AFTER the closing '}'.
static std::size_t
parse_brace_node_at(const std::string & text, std::size_t pos, BraceNode & node)
{
  if (pos + 1 >= text.size() || text[pos] != '$' || text[pos + 1] != '{')
    throw Error("expected '${' at position " + std::to_string(pos));

  pos += 2; // consume '${'

  // Collect raw content between the braces, respecting nesting.
  std::string raw;
  int depth = 1;
  while (pos < text.size() && depth > 0)
  {
    if (pos + 1 < text.size() && text[pos] == '$' && text[pos + 1] == '{')
    {
      raw += "${";
      pos += 2;
      ++depth;
    }
    else if (text[pos] == '}')
    {
      --depth;
      if (depth > 0)
        raw += '}';
      ++pos;
    }
    else
    {
      raw += text[pos++];
    }
  }

  if (depth != 0)
    throw Error("unterminated brace expression");

  // Now parse 'raw' into command + args / subnodes.
  // Tokenise: split on wnmhitespace, handle nested ${...} as sub-nodes.
  std::vector<std::string> tokens;
  std::vector<bool> is_brace; // parallel to tokens

  std::size_t rp = skip_ws(raw, 0);
  while (rp < raw.size())
  {
    rp = skip_ws(raw, rp);
    if (rp >= raw.size())
      break;

    if (rp + 1 < raw.size() && raw[rp] == '$' && raw[rp + 1] == '{')
    {
      // Nested brace expression — capture the full ${...} substring.
      std::size_t start = rp;
      int d = 0;
      do
      {
        if (rp + 1 < raw.size() && raw[rp] == '$' && raw[rp + 1] == '{')
        {
          ++d;
          rp += 2;
        }
        else if (raw[rp] == '}')
        {
          --d;
          ++rp;
        }
        else
        {
          ++rp;
        }
      } while (d > 0 && rp < raw.size());
      tokens.push_back(raw.substr(start, rp - start));
      is_brace.push_back(true);
    }
    else
    {
      // Plain token: scan to next wnmhitespace or '$'
      std::size_t start = rp;
      while (rp < raw.size() && raw[rp] != ' ' && raw[rp] != '\t' && raw[rp] != '\n' &&
             !(raw[rp] == '$' && rp + 1 < raw.size() && raw[rp + 1] == '{'))
        ++rp;
      tokens.push_back(raw.substr(start, rp - start));
      is_brace.push_back(false);
    }
  }

  // The first plain token is the command (if present).
  std::size_t first_arg = 0;
  if (!tokens.empty() && !is_brace[0])
  {
    node.command = tokens[0];
    first_arg = 1;
  }

  // Fill args and subnodes.
  for (std::size_t i = first_arg; i < tokens.size(); ++i)
  {
    if (is_brace[i])
    {
      BraceNode sub;
      parse_brace_node_at(tokens[i], 0, sub);
      node.subnodes.push_back(std::move(sub));
    }
    else
    {
      node.args.push_back(tokens[i]);
    }
  }

  return pos;
}

BraceNode
parse_brace_expr(const std::string & text)
{
  BraceNode node;
  parse_brace_node_at(text, 0, node);
  return node;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Evaluation
// ═══════════════════════════════════════════════════════════════════════════════

/// Built-in evaluator.
struct DefaultEvaluator : Evaluator
{
  std::string evaluate(const BraceNode & node, const Node * context) override
  {
    const std::string & cmd = node.command;

    if (cmd == "env")
    {
      // ${env VARNAME}
      std::string varname;
      if (!node.args.empty())
        varname = node.args[0];
      else if (!node.subnodes.empty())
        varname = evaluate(node.subnodes[0], context);
      if (varname.empty())
        throw Error("${env} requires a variable name");
      const char * val = std::getenv(varname.c_str());
      return val ? std::string(val) : std::string();
    }

    if (cmd == "raw")
    {
      // ${raw a b c} → concatenate all args
      std::ostringstream ss;
      for (auto & a : node.args)
        ss << a;
      for (auto & sub : node.subnodes)
        ss << evaluate(sub, context);
      return ss.str();
    }

    // Default (empty command or "replace"): parameter lookup.
    std::string varname;
    if (cmd == "replace")
    {
      if (!node.args.empty())
        varname = node.args[0];
      else if (!node.subnodes.empty())
        varname = evaluate(node.subnodes[0], context);
    }
    else
    {
      // cmd IS the variable name (${varname})
      varname = cmd;
      if (!node.args.empty())
        varname += " " + node.args[0]; // shouldn't normally happen
    }

    if (varname.empty())
      throw Error("brace expression: cannot determine variable name");

    if (!context)
      throw Error("brace expression: no context for parameter lookup of '" + varname + "'");

    const Node * target = context->root()->find(varname);
    if (!target)
      throw Error("brace expression: no parameter '" + varname + "' found in tree");

    return target->param<std::string>();
  }
};

std::string
eval_brace_expr(const BraceNode & node, const Node * context, Evaluator * eval)
{
  if (eval)
    return eval->evaluate(node, context);
  DefaultEvaluator def;
  return def.evaluate(node, context);
}

/// Expand all ${...} sequences inside a string.
static std::string
expand_all(const std::string & s, const Node * context, Evaluator * eval)
{
  std::string result;
  std::size_t pos = 0;
  while (pos < s.size())
  {
    auto brace = s.find("${", pos);
    if (brace == std::string::npos)
    {
      result += s.substr(pos);
      break;
    }
    result += s.substr(pos, brace - pos);

    BraceNode node;
    std::size_t after = parse_brace_node_at(s, brace, node);
    result += eval_brace_expr(node, context, eval);
    pos = after;
  }
  return result;
}

std::string
expand_brace_expr(const std::string & text, const Node * context, Evaluator * eval)
{
  return expand_all(text, context, eval);
}

} // namespace nmhit
