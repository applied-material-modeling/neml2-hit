%top{
#define YY_NO_UNISTD_H
}

%{
#include <string>
#include "ParseDriver.h"

typedef nmhit_detail::ParseDriver HIT_Driver;
#define DRIVER  ((HIT_Driver *)yyextra)

#define YY_USER_ACTION  DRIVER->on_token_begin(); DRIVER->on_token_text(yytext, yyleng);

#define TOK(t)   return nmhit_detail::Parser::token::t
#define TOK_S(t) do { DRIVER->set_pending(yytext, yyleng); \
                      return nmhit_detail::Parser::token::t; } while(0)
%}

%option noyywrap nounput noinput
%option reentrant never-interactive
%option prefix="HIT"
%option extra-type="HIT_Driver *"

%s SECTION_BODY IN_PARAM
%x IN_PATH AFTER_EQ IN_ARRAY IN_DARRAY IN_BRACE IN_INCLUDE

SIGN       [+\-]
DIGIT      [0-9]
EXP        [eE][+\-]?[0-9]+
FLOAT      ({SIGN}?([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)([eE][+\-]?[0-9]+)?|{SIGN}?[0-9]+[eE][+\-]?[0-9]+)
INTEGER    {SIGN}?[0-9]+
IDENT      [A-Za-z0-9_.\/:<>+\-*!]+
COMMENT    #[^\n]*

%%

<INITIAL,SECTION_BODY>[ \t\r]+             { /* skip */ }
<INITIAL,SECTION_BODY>\n([ \t]*\n)+        { TOK(TOK_BLANK); }
<INITIAL,SECTION_BODY>\n                   { /* skip */ }
<INITIAL,SECTION_BODY>{COMMENT}            { TOK_S(TOK_COMMENT); }
<INITIAL,SECTION_BODY>"!include"           { BEGIN(IN_INCLUDE); TOK(TOK_INCLUDE_KW); }
<INITIAL,SECTION_BODY>"[]"                 { BEGIN(INITIAL); TOK(TOK_SECTION_END); }
<INITIAL,SECTION_BODY>"["                  { BEGIN(IN_PATH); TOK(TOK_LBRACKET); }
<INITIAL,SECTION_BODY>{IDENT}              { BEGIN(IN_PARAM); TOK_S(TOK_IDENT); }
<INITIAL,SECTION_BODY>.                    { DRIVER->lex_error(yytext, yylineno); }

<IN_PATH>[ \t]+                            { /* skip */ }
<IN_PATH>"]"                               { BEGIN(SECTION_BODY); return ']'; }
<IN_PATH>[^ \t\n\r\[\]]+                   { TOK_S(TOK_SECTION_PATH); }
<IN_PATH>\n                                { DRIVER->lex_error("newline in section header", yylineno); BEGIN(SECTION_BODY); }
<IN_PATH>.                                 { DRIVER->lex_error(yytext, yylineno); }

<IN_PARAM>[ \t\r]+                         { /* skip */ }
<IN_PARAM>":override="                     { BEGIN(AFTER_EQ); TOK(TOK_OVERRIDE); }
<IN_PARAM>":="                             { BEGIN(AFTER_EQ); TOK(TOK_OVERRIDE); }
<IN_PARAM>"="                              { BEGIN(AFTER_EQ); TOK(TOK_ASSIGN); }
<IN_PARAM>\n                               { BEGIN(SECTION_BODY); }
<IN_PARAM>.                                { DRIVER->lex_error(yytext, yylineno); BEGIN(SECTION_BODY); }

<AFTER_EQ>[ \t\r]+                         { /* skip */ }
<AFTER_EQ>\n                               { BEGIN(SECTION_BODY); }
<AFTER_EQ>{FLOAT}                           { BEGIN(SECTION_BODY); TOK_S(TOK_FLOAT); }
<AFTER_EQ>{INTEGER}                        { BEGIN(SECTION_BODY); TOK_S(TOK_INTEGER); }
<AFTER_EQ>"\""                             { BEGIN(IN_DARRAY); TOK(TOK_SINGLE_QUOTE); }
<AFTER_EQ>"'"                              { BEGIN(IN_ARRAY); TOK(TOK_SINGLE_QUOTE); }
<AFTER_EQ>"${"                             { DRIVER->begin_brace(); yymore(); BEGIN(IN_BRACE); }
<AFTER_EQ>[^ \t\n\r\[#$'"\\]+             { BEGIN(SECTION_BODY); TOK_S(TOK_UNQUOTED_STR); }
<AFTER_EQ>.                                { DRIVER->lex_error(yytext, yylineno); BEGIN(SECTION_BODY); }

<IN_BRACE>"${"                             { DRIVER->begin_brace(); yymore(); }
<IN_BRACE>"}"                              {
    if (DRIVER->end_brace()) {
        BEGIN(SECTION_BODY);
        TOK_S(TOK_BRACE_EXPR);
    } else {
        yymore();
    }
}
<IN_BRACE>[^$}]+                           { yymore(); }
<IN_BRACE>"$"[^{]                          { yymore(); }

<IN_ARRAY>[ \t\n\r]+                       { /* skip */ }
<IN_ARRAY>"'"                              { BEGIN(SECTION_BODY); TOK(TOK_SINGLE_QUOTE); }
<IN_ARRAY>";"                              { TOK(TOK_SEMICOLON); }
<IN_ARRAY>{FLOAT}                           { TOK_S(TOK_FLOAT); }
<IN_ARRAY>{INTEGER}                        { TOK_S(TOK_INTEGER); }
<IN_ARRAY>[^ \t\n\r;#$'"\\]+              { TOK_S(TOK_ARRAY_ELEM); }
<IN_ARRAY>.                                { DRIVER->lex_error(yytext, yylineno); }

<IN_DARRAY>[ \t\n\r]+                      { /* skip */ }
<IN_DARRAY>"\""                            { BEGIN(SECTION_BODY); TOK(TOK_SINGLE_QUOTE); }
<IN_DARRAY>";"                             { TOK(TOK_SEMICOLON); }
<IN_DARRAY>{FLOAT}                          { TOK_S(TOK_FLOAT); }
<IN_DARRAY>{INTEGER}                       { TOK_S(TOK_INTEGER); }
<IN_DARRAY>[^ \t\n\r;#$'"\\]+             { TOK_S(TOK_ARRAY_ELEM); }
<IN_DARRAY>.                               { DRIVER->lex_error(yytext, yylineno); }

<IN_INCLUDE>[ \t]+                         { /* skip */ }
<IN_INCLUDE>[^ \t\n\r#][^\n\r#]*          {
    int len = yyleng;
    while (len > 0 && (yytext[len-1] == ' ' || yytext[len-1] == '\t'))
        --len;
    DRIVER->set_pending_n(yytext, len);
    BEGIN(SECTION_BODY);
    return nmhit_detail::Parser::token::TOK_INCLUDE_PATH;
}
<IN_INCLUDE>\n                             { BEGIN(SECTION_BODY); }
<IN_INCLUDE>.                              { DRIVER->lex_error(yytext, yylineno); BEGIN(SECTION_BODY); }

<*><<EOF>>                                 { TOK(TOK_EOF); }

%%
