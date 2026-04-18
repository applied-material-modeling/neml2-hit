/*
 * Bison grammar for the HIT (Hierarchical Input Text) format.
 *
 * Context-free grammar:
 *   file        → item*
 *   item        → section | field | comment | blank | include
 *   section     → '[' path ']' item* '[]'
 *   field       → ident ('=' | ':=') value
 *   value       → scalar | array
 *   scalar      → integer | float | unquoted | quoted | brace_expr
 *   array       → "'" array_elem* "'"
 *   include     → '!include' path
 */

%require "3.7"
%skeleton "lalr1.cc"

%define api.namespace    {nmhit_detail}
%define api.parser.class {Parser}
%define api.value.type   variant
%define parse.error      verbose
%locations

%code requires {
#include <memory>
#include <string>
#include <vector>
#include "nmhit/Node.h"

namespace nmhit_detail { class ParseDriver; }
}

%parse-param { nmhit_detail::ParseDriver & driver }

%code {
#include "ParseDriver.h"
#undef  yylex
#define yylex driver.lex
}

/* ── token declarations ──────────────────────────────────────────────────── */

/* Tokens carrying a string semantic value */
%token <std::string> TOK_SECTION_PATH  "section path"
%token <std::string> TOK_IDENT         "identifier"
%token <std::string> TOK_INTEGER       "integer"
%token <std::string> TOK_FLOAT          "floating-point number"
%token <std::string> TOK_UNQUOTED_STR  "unquoted string"
%token <std::string> TOK_ARRAY_ELEM    "array element"
%token <std::string> TOK_BRACE_EXPR    "brace expression"
%token <std::string> TOK_COMMENT       "comment"
%token <std::string> TOK_INCLUDE_PATH  "include path"

/* Punctuation tokens (no semantic value) */
%token TOK_LBRACKET    "["
%token TOK_SECTION_END "[]"
%token TOK_ASSIGN      "="
%token TOK_OVERRIDE    ":="
%token TOK_SINGLE_QUOTE "'"
%token TOK_SEMICOLON   ";"
%token TOK_INCLUDE_KW  "!include"
%token TOK_BLANK       "blank line"
%token TOK_EOF      0  "end of file"

/* ── non-terminal types ──────────────────────────────────────────────────── */

%type <std::unique_ptr<nmhit::Node>>              item section field comment blank include
%type <std::vector<std::unique_ptr<nmhit::Node>>> items array_row array_rows
%type <std::string>                             value scalar array_elem assign_op

%%

/* ── top level ───────────────────────────────────────────────────────────── */

start
  : items  { driver.set_root(std::move($1)); }
  ;

items
  : %empty  {}
  | items item
    {
      if ($2)
        $1.push_back(std::move($2));
      $$ = std::move($1);
    }
  ;

item
  : section  { $$ = std::move($1); }
  | field    { $$ = std::move($1); }
  | comment  { $$ = std::move($1); }
  | blank    { $$ = std::move($1); }
  | include  { $$ = std::move($1); }
  ;

/* ── section ─────────────────────────────────────────────────────────────── */

section
  : TOK_LBRACKET TOK_SECTION_PATH ']' items TOK_SECTION_END
    { $$ = driver.build_section($2, std::move($4), @$); }
  ;

/* ── field ───────────────────────────────────────────────────────────────── */

field
  : TOK_IDENT assign_op value
    { $$ = driver.build_field($1, $2 == ":=", $3, @$); }
  ;

assign_op
  : TOK_ASSIGN    { $$ = "=";  }
  | TOK_OVERRIDE  { $$ = ":="; }
  ;

/* value is either a scalar or a single-quoted array (1D or 2D) */
value
  : scalar                                            { $$ = std::move($1); }
  | TOK_SINGLE_QUOTE array_rows TOK_SINGLE_QUOTE      { $$ = driver.build_array_value(std::move($2)); }
  | TOK_SINGLE_QUOTE TOK_SINGLE_QUOTE                 { $$ = "''"; }
  ;

scalar
  : TOK_INTEGER      { $$ = std::move($1); }
  | TOK_FLOAT         { $$ = std::move($1); }
  | TOK_UNQUOTED_STR { $$ = std::move($1); }
  | TOK_BRACE_EXPR   { $$ = std::move($1); }
  ;

/* array_rows: one or more semicolon-separated rows; nullptr sentinel = row boundary */
array_rows
  : array_row  { $$ = std::move($1); }
  | array_rows TOK_SEMICOLON array_row
    {
      $1.push_back(nullptr);               // nullptr = row separator sentinel
      for (auto & e : $3)
        $1.push_back(std::move(e));
      $$ = std::move($1);
    }
  ;

array_row
  : array_elem
    {
      $$.push_back(std::make_unique<nmhit::Field>("", $1));
      driver._ws_pending = false;
    }
  | array_row array_elem
    {
      std::string sp = driver._ws_pending ? " " : "";
      driver._ws_pending = false;
      $1.push_back(std::make_unique<nmhit::Field>(std::move(sp), $2));
      $$ = std::move($1);
    }
  ;

array_elem
  : TOK_ARRAY_ELEM { $$ = std::move($1); }
  | TOK_INTEGER    { $$ = std::move($1); }
  | TOK_FLOAT       { $$ = std::move($1); }
  | TOK_BRACE_EXPR { $$ = std::move($1); }
  ;

/* ── file include ─────────────────────────────────────────────────────────── */

include
  : TOK_INCLUDE_KW TOK_INCLUDE_PATH
    { $$ = driver.build_include($2, @$); }
  ;

/* ── comment ─────────────────────────────────────────────────────────────── */

comment
  : TOK_COMMENT
    {
      auto n = std::make_unique<nmhit::Comment>($1);
      n->_set_location(driver.filename(), @1.begin.line, @1.begin.column);
      $$ = std::move(n);
    }
  ;

/* ── blank line ──────────────────────────────────────────────────────────── */

blank
  : TOK_BLANK  { $$ = std::make_unique<nmhit::Blank>(); }
  ;

%%

namespace nmhit_detail
{

void
Parser::error(const location_type & loc, const std::string & msg)
{
  driver.report_error(loc, msg);
}

} // namespace nmhit_detail
