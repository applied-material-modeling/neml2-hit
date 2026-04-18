// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "Parser.h"


// Unqualified %code blocks.
#line 35 "/home/gary/projects/neml2-hit/src/Parser.y"

#include "ParseDriver.h"
#undef  yylex
#define yylex driver.lex

#line 52 "/home/gary/projects/neml2-hit/build/Parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 18 "/home/gary/projects/neml2-hit/src/Parser.y"
namespace nmhit_detail {
#line 145 "/home/gary/projects/neml2-hit/build/Parser.cpp"

  /// Build a parser object.
  Parser::Parser (nmhit_detail::ParseDriver & driver_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg)
  {}

  Parser::~Parser ()
  {}

  Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/

  // basic_symbol.
  template <typename Base>
  Parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value ()
    , location (that.location)
  {
    switch (this->kind ())
    {
      case symbol_kind::S_TOK_SECTION_PATH: // "section path"
      case symbol_kind::S_TOK_IDENT: // "identifier"
      case symbol_kind::S_TOK_INTEGER: // "integer"
      case symbol_kind::S_TOK_FLOAT: // "floating-point number"
      case symbol_kind::S_TOK_UNQUOTED_STR: // "unquoted string"
      case symbol_kind::S_TOK_ARRAY_ELEM: // "array element"
      case symbol_kind::S_TOK_BRACE_EXPR: // "brace expression"
      case symbol_kind::S_TOK_COMMENT: // "comment"
      case symbol_kind::S_TOK_INCLUDE_PATH: // "include path"
      case symbol_kind::S_assign_op: // assign_op
      case symbol_kind::S_value: // value
      case symbol_kind::S_scalar: // scalar
      case symbol_kind::S_array_elem: // array_elem
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_item: // item
      case symbol_kind::S_section: // section
      case symbol_kind::S_field: // field
      case symbol_kind::S_include: // include
      case symbol_kind::S_comment: // comment
      case symbol_kind::S_blank: // blank
        value.copy< std::unique_ptr<nmhit::Node> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_items: // items
      case symbol_kind::S_array_rows: // array_rows
      case symbol_kind::S_array_row: // array_row
        value.copy< std::vector<std::unique_ptr<nmhit::Node>> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }




  template <typename Base>
  Parser::symbol_kind_type
  Parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  Parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  Parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case symbol_kind::S_TOK_SECTION_PATH: // "section path"
      case symbol_kind::S_TOK_IDENT: // "identifier"
      case symbol_kind::S_TOK_INTEGER: // "integer"
      case symbol_kind::S_TOK_FLOAT: // "floating-point number"
      case symbol_kind::S_TOK_UNQUOTED_STR: // "unquoted string"
      case symbol_kind::S_TOK_ARRAY_ELEM: // "array element"
      case symbol_kind::S_TOK_BRACE_EXPR: // "brace expression"
      case symbol_kind::S_TOK_COMMENT: // "comment"
      case symbol_kind::S_TOK_INCLUDE_PATH: // "include path"
      case symbol_kind::S_assign_op: // assign_op
      case symbol_kind::S_value: // value
      case symbol_kind::S_scalar: // scalar
      case symbol_kind::S_array_elem: // array_elem
        value.move< std::string > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_item: // item
      case symbol_kind::S_section: // section
      case symbol_kind::S_field: // field
      case symbol_kind::S_include: // include
      case symbol_kind::S_comment: // comment
      case symbol_kind::S_blank: // blank
        value.move< std::unique_ptr<nmhit::Node> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_items: // items
      case symbol_kind::S_array_rows: // array_rows
      case symbol_kind::S_array_row: // array_row
        value.move< std::vector<std::unique_ptr<nmhit::Node>> > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
  }

  // by_kind.
  Parser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  Parser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  Parser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  Parser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  Parser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  Parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  Parser::symbol_kind_type
  Parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  Parser::symbol_kind_type
  Parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }



  // by_state.
  Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Parser::symbol_kind_type
  Parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_TOK_SECTION_PATH: // "section path"
      case symbol_kind::S_TOK_IDENT: // "identifier"
      case symbol_kind::S_TOK_INTEGER: // "integer"
      case symbol_kind::S_TOK_FLOAT: // "floating-point number"
      case symbol_kind::S_TOK_UNQUOTED_STR: // "unquoted string"
      case symbol_kind::S_TOK_ARRAY_ELEM: // "array element"
      case symbol_kind::S_TOK_BRACE_EXPR: // "brace expression"
      case symbol_kind::S_TOK_COMMENT: // "comment"
      case symbol_kind::S_TOK_INCLUDE_PATH: // "include path"
      case symbol_kind::S_assign_op: // assign_op
      case symbol_kind::S_value: // value
      case symbol_kind::S_scalar: // scalar
      case symbol_kind::S_array_elem: // array_elem
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_item: // item
      case symbol_kind::S_section: // section
      case symbol_kind::S_field: // field
      case symbol_kind::S_include: // include
      case symbol_kind::S_comment: // comment
      case symbol_kind::S_blank: // blank
        value.YY_MOVE_OR_COPY< std::unique_ptr<nmhit::Node> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_items: // items
      case symbol_kind::S_array_rows: // array_rows
      case symbol_kind::S_array_row: // array_row
        value.YY_MOVE_OR_COPY< std::vector<std::unique_ptr<nmhit::Node>> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_TOK_SECTION_PATH: // "section path"
      case symbol_kind::S_TOK_IDENT: // "identifier"
      case symbol_kind::S_TOK_INTEGER: // "integer"
      case symbol_kind::S_TOK_FLOAT: // "floating-point number"
      case symbol_kind::S_TOK_UNQUOTED_STR: // "unquoted string"
      case symbol_kind::S_TOK_ARRAY_ELEM: // "array element"
      case symbol_kind::S_TOK_BRACE_EXPR: // "brace expression"
      case symbol_kind::S_TOK_COMMENT: // "comment"
      case symbol_kind::S_TOK_INCLUDE_PATH: // "include path"
      case symbol_kind::S_assign_op: // assign_op
      case symbol_kind::S_value: // value
      case symbol_kind::S_scalar: // scalar
      case symbol_kind::S_array_elem: // array_elem
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_item: // item
      case symbol_kind::S_section: // section
      case symbol_kind::S_field: // field
      case symbol_kind::S_include: // include
      case symbol_kind::S_comment: // comment
      case symbol_kind::S_blank: // blank
        value.move< std::unique_ptr<nmhit::Node> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_items: // items
      case symbol_kind::S_array_rows: // array_rows
      case symbol_kind::S_array_row: // array_row
        value.move< std::vector<std::unique_ptr<nmhit::Node>> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_TOK_SECTION_PATH: // "section path"
      case symbol_kind::S_TOK_IDENT: // "identifier"
      case symbol_kind::S_TOK_INTEGER: // "integer"
      case symbol_kind::S_TOK_FLOAT: // "floating-point number"
      case symbol_kind::S_TOK_UNQUOTED_STR: // "unquoted string"
      case symbol_kind::S_TOK_ARRAY_ELEM: // "array element"
      case symbol_kind::S_TOK_BRACE_EXPR: // "brace expression"
      case symbol_kind::S_TOK_COMMENT: // "comment"
      case symbol_kind::S_TOK_INCLUDE_PATH: // "include path"
      case symbol_kind::S_assign_op: // assign_op
      case symbol_kind::S_value: // value
      case symbol_kind::S_scalar: // scalar
      case symbol_kind::S_array_elem: // array_elem
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_item: // item
      case symbol_kind::S_section: // section
      case symbol_kind::S_field: // field
      case symbol_kind::S_include: // include
      case symbol_kind::S_comment: // comment
      case symbol_kind::S_blank: // blank
        value.copy< std::unique_ptr<nmhit::Node> > (that.value);
        break;

      case symbol_kind::S_items: // items
      case symbol_kind::S_array_rows: // array_rows
      case symbol_kind::S_array_row: // array_row
        value.copy< std::vector<std::unique_ptr<nmhit::Node>> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_TOK_SECTION_PATH: // "section path"
      case symbol_kind::S_TOK_IDENT: // "identifier"
      case symbol_kind::S_TOK_INTEGER: // "integer"
      case symbol_kind::S_TOK_FLOAT: // "floating-point number"
      case symbol_kind::S_TOK_UNQUOTED_STR: // "unquoted string"
      case symbol_kind::S_TOK_ARRAY_ELEM: // "array element"
      case symbol_kind::S_TOK_BRACE_EXPR: // "brace expression"
      case symbol_kind::S_TOK_COMMENT: // "comment"
      case symbol_kind::S_TOK_INCLUDE_PATH: // "include path"
      case symbol_kind::S_assign_op: // assign_op
      case symbol_kind::S_value: // value
      case symbol_kind::S_scalar: // scalar
      case symbol_kind::S_array_elem: // array_elem
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_item: // item
      case symbol_kind::S_section: // section
      case symbol_kind::S_field: // field
      case symbol_kind::S_include: // include
      case symbol_kind::S_comment: // comment
      case symbol_kind::S_blank: // blank
        value.move< std::unique_ptr<nmhit::Node> > (that.value);
        break;

      case symbol_kind::S_items: // items
      case symbol_kind::S_array_rows: // array_rows
      case symbol_kind::S_array_row: // array_row
        value.move< std::vector<std::unique_ptr<nmhit::Node>> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  Parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  Parser::state_type
  Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  Parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::operator() ()
  {
    return parse ();
  }

  int
  Parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_TOK_SECTION_PATH: // "section path"
      case symbol_kind::S_TOK_IDENT: // "identifier"
      case symbol_kind::S_TOK_INTEGER: // "integer"
      case symbol_kind::S_TOK_FLOAT: // "floating-point number"
      case symbol_kind::S_TOK_UNQUOTED_STR: // "unquoted string"
      case symbol_kind::S_TOK_ARRAY_ELEM: // "array element"
      case symbol_kind::S_TOK_BRACE_EXPR: // "brace expression"
      case symbol_kind::S_TOK_COMMENT: // "comment"
      case symbol_kind::S_TOK_INCLUDE_PATH: // "include path"
      case symbol_kind::S_assign_op: // assign_op
      case symbol_kind::S_value: // value
      case symbol_kind::S_scalar: // scalar
      case symbol_kind::S_array_elem: // array_elem
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_item: // item
      case symbol_kind::S_section: // section
      case symbol_kind::S_field: // field
      case symbol_kind::S_include: // include
      case symbol_kind::S_comment: // comment
      case symbol_kind::S_blank: // blank
        yylhs.value.emplace< std::unique_ptr<nmhit::Node> > ();
        break;

      case symbol_kind::S_items: // items
      case symbol_kind::S_array_rows: // array_rows
      case symbol_kind::S_array_row: // array_row
        yylhs.value.emplace< std::vector<std::unique_ptr<nmhit::Node>> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // start: items
#line 76 "/home/gary/projects/neml2-hit/src/Parser.y"
           { driver.set_root(std::move(yystack_[0].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ())); }
#line 851 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 3: // items: %empty
#line 80 "/home/gary/projects/neml2-hit/src/Parser.y"
            {}
#line 857 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 4: // items: items item
#line 82 "/home/gary/projects/neml2-hit/src/Parser.y"
    {
      if (yystack_[0].value.as < std::unique_ptr<nmhit::Node> > ())
        yystack_[1].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<nmhit::Node> > ()));
      yylhs.value.as < std::vector<std::unique_ptr<nmhit::Node>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ());
    }
#line 867 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 5: // item: section
#line 90 "/home/gary/projects/neml2-hit/src/Parser.y"
             { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = std::move(yystack_[0].value.as < std::unique_ptr<nmhit::Node> > ()); }
#line 873 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 6: // item: field
#line 91 "/home/gary/projects/neml2-hit/src/Parser.y"
             { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = std::move(yystack_[0].value.as < std::unique_ptr<nmhit::Node> > ()); }
#line 879 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 7: // item: comment
#line 92 "/home/gary/projects/neml2-hit/src/Parser.y"
             { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = std::move(yystack_[0].value.as < std::unique_ptr<nmhit::Node> > ()); }
#line 885 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 8: // item: blank
#line 93 "/home/gary/projects/neml2-hit/src/Parser.y"
             { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = std::move(yystack_[0].value.as < std::unique_ptr<nmhit::Node> > ()); }
#line 891 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 9: // item: include
#line 94 "/home/gary/projects/neml2-hit/src/Parser.y"
             { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = std::move(yystack_[0].value.as < std::unique_ptr<nmhit::Node> > ()); }
#line 897 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 10: // section: "[" "section path" ']' items "[]"
#line 101 "/home/gary/projects/neml2-hit/src/Parser.y"
    { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = driver.build_section(yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ()), yylhs.location); }
#line 903 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 11: // field: "identifier" assign_op value
#line 108 "/home/gary/projects/neml2-hit/src/Parser.y"
    { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = driver.build_field(yystack_[2].value.as < std::string > (), yystack_[1].value.as < std::string > () == ":=", yystack_[0].value.as < std::string > (), yylhs.location); }
#line 909 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 12: // assign_op: "="
#line 112 "/home/gary/projects/neml2-hit/src/Parser.y"
                  { yylhs.value.as < std::string > () = "=";  }
#line 915 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 13: // assign_op: ":="
#line 113 "/home/gary/projects/neml2-hit/src/Parser.y"
                  { yylhs.value.as < std::string > () = ":="; }
#line 921 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 14: // value: scalar
#line 118 "/home/gary/projects/neml2-hit/src/Parser.y"
                                                      { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 927 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 15: // value: "'" array_rows "'"
#line 119 "/home/gary/projects/neml2-hit/src/Parser.y"
                                                      { yylhs.value.as < std::string > () = driver.build_array_value(std::move(yystack_[1].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ())); }
#line 933 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 16: // value: "'" "'"
#line 120 "/home/gary/projects/neml2-hit/src/Parser.y"
                                                      { yylhs.value.as < std::string > () = "''"; }
#line 939 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 17: // scalar: "integer"
#line 124 "/home/gary/projects/neml2-hit/src/Parser.y"
                     { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 945 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 18: // scalar: "floating-point number"
#line 125 "/home/gary/projects/neml2-hit/src/Parser.y"
                      { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 951 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 19: // scalar: "unquoted string"
#line 126 "/home/gary/projects/neml2-hit/src/Parser.y"
                     { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 957 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 20: // scalar: "brace expression"
#line 127 "/home/gary/projects/neml2-hit/src/Parser.y"
                     { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 963 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 21: // array_rows: array_row
#line 132 "/home/gary/projects/neml2-hit/src/Parser.y"
               { yylhs.value.as < std::vector<std::unique_ptr<nmhit::Node>> > () = std::move(yystack_[0].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ()); }
#line 969 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 22: // array_rows: array_rows ";" array_row
#line 134 "/home/gary/projects/neml2-hit/src/Parser.y"
    {
      yystack_[2].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ().push_back(nullptr);               // nullptr = row separator sentinel
      for (auto & e : yystack_[0].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ())
        yystack_[2].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ().push_back(std::move(e));
      yylhs.value.as < std::vector<std::unique_ptr<nmhit::Node>> > () = std::move(yystack_[2].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ());
    }
#line 980 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 23: // array_row: array_elem
#line 144 "/home/gary/projects/neml2-hit/src/Parser.y"
    {
      yylhs.value.as < std::vector<std::unique_ptr<nmhit::Node>> > ().push_back(std::make_unique<nmhit::Field>("", yystack_[0].value.as < std::string > ()));
      driver._ws_pending = false;
    }
#line 989 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 24: // array_row: array_row array_elem
#line 149 "/home/gary/projects/neml2-hit/src/Parser.y"
    {
      std::string sp = driver._ws_pending ? " " : "";
      driver._ws_pending = false;
      yystack_[1].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ().push_back(std::make_unique<nmhit::Field>(std::move(sp), yystack_[0].value.as < std::string > ()));
      yylhs.value.as < std::vector<std::unique_ptr<nmhit::Node>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<nmhit::Node>> > ());
    }
#line 1000 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 25: // array_elem: "array element"
#line 158 "/home/gary/projects/neml2-hit/src/Parser.y"
                   { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 1006 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 26: // array_elem: "integer"
#line 159 "/home/gary/projects/neml2-hit/src/Parser.y"
                   { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 1012 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 27: // array_elem: "floating-point number"
#line 160 "/home/gary/projects/neml2-hit/src/Parser.y"
                    { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 1018 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 28: // array_elem: "brace expression"
#line 161 "/home/gary/projects/neml2-hit/src/Parser.y"
                   { yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); }
#line 1024 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 29: // include: "!include" "include path"
#line 168 "/home/gary/projects/neml2-hit/src/Parser.y"
    { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = driver.build_include(yystack_[0].value.as < std::string > (), yylhs.location); }
#line 1030 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 30: // comment: "comment"
#line 175 "/home/gary/projects/neml2-hit/src/Parser.y"
    {
      auto n = std::make_unique<nmhit::Comment>(yystack_[0].value.as < std::string > ());
      n->_set_location(driver.filename(), yystack_[0].location.begin.line, yystack_[0].location.begin.column);
      yylhs.value.as < std::unique_ptr<nmhit::Node> > () = std::move(n);
    }
#line 1040 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;

  case 31: // blank: "blank line"
#line 185 "/home/gary/projects/neml2-hit/src/Parser.y"
               { yylhs.value.as < std::unique_ptr<nmhit::Node> > () = std::make_unique<nmhit::Blank>(); }
#line 1046 "/home/gary/projects/neml2-hit/build/Parser.cpp"
    break;


#line 1050 "/home/gary/projects/neml2-hit/build/Parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  Parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // Parser::context.
  Parser::context::context (const Parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  Parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
  Parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char Parser::yypact_ninf_ = -26;

  const signed char Parser::yytable_ninf_ = -1;

  const signed char
  Parser::yypact_[] =
  {
     -26,     3,     0,   -26,   -13,   -26,     2,     6,   -26,   -26,
     -26,   -26,   -26,   -26,   -26,   -26,   -26,    15,    -9,   -26,
     -26,   -26,   -26,   -26,    20,   -26,   -26,   -26,   -26,   -26,
     -26,   -26,   -26,    16,    29,   -26,    -4,   -26,    29,   -26,
     -26,    29
  };

  const signed char
  Parser::yydefact_[] =
  {
       3,     0,     2,     1,     0,    30,     0,     0,    31,     4,
       5,     6,     9,     7,     8,    12,    13,     0,     0,    29,
      17,    18,    19,    20,     0,    11,    14,     3,    26,    27,
      25,    28,    16,     0,    21,    23,     0,    15,     0,    24,
      10,    22
  };

  const signed char
  Parser::yypgoto_[] =
  {
     -26,   -26,   -20,   -26,   -26,   -26,   -26,   -26,   -26,   -26,
     -25,   -18,   -26,   -26,   -26
  };

  const signed char
  Parser::yydefgoto_[] =
  {
       0,     1,     2,     9,    10,    11,    17,    25,    26,    33,
      34,    35,    12,    13,    14
  };

  const signed char
  Parser::yytable_[] =
  {
       4,    15,    16,     3,     4,    18,     5,    36,     6,    40,
       5,    27,     6,    41,     7,     8,    39,    19,     7,     8,
      20,    21,    22,    39,    23,    28,    29,     0,    30,    31,
       0,    24,    37,    38,    28,    29,    32,    30,    31
  };

  const signed char
  Parser::yycheck_[] =
  {
       4,    14,    15,     0,     4,     3,    10,    27,    12,    13,
      10,    20,    12,    38,    18,    19,    34,    11,    18,    19,
       5,     6,     7,    41,     9,     5,     6,    -1,     8,     9,
      -1,    16,    16,    17,     5,     6,    16,     8,     9
  };

  const signed char
  Parser::yystos_[] =
  {
       0,    22,    23,     0,     4,    10,    12,    18,    19,    24,
      25,    26,    33,    34,    35,    14,    15,    27,     3,    11,
       5,     6,     7,     9,    16,    28,    29,    20,     5,     6,
       8,     9,    16,    30,    31,    32,    23,    16,    17,    32,
      13,    31
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    21,    22,    23,    23,    24,    24,    24,    24,    24,
      25,    26,    27,    27,    28,    28,    28,    29,    29,    29,
      29,    30,    30,    31,    31,    32,    32,    32,    32,    33,
      34,    35
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     1,     1,     1,     1,     1,
       5,     3,     1,     1,     1,     3,     2,     1,     1,     1,
       1,     1,     3,     1,     2,     1,     1,     1,     1,     2,
       1,     1
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const Parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "\"section path\"",
  "\"identifier\"", "\"integer\"", "\"floating-point number\"",
  "\"unquoted string\"", "\"array element\"", "\"brace expression\"",
  "\"comment\"", "\"include path\"", "\"[\"", "\"[]\"", "\"=\"", "\":=\"",
  "\"'\"", "\";\"", "\"!include\"", "\"blank line\"", "']'", "$accept",
  "start", "items", "item", "section", "field", "assign_op", "value",
  "scalar", "array_rows", "array_row", "array_elem", "include", "comment",
  "blank", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const unsigned char
  Parser::yyrline_[] =
  {
       0,    76,    76,    80,    81,    90,    91,    92,    93,    94,
     100,   107,   112,   113,   118,   119,   120,   124,   125,   126,
     127,   132,   133,   143,   148,   158,   159,   160,   161,   167,
     174,   185
  };

  void
  Parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  Parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  Parser::symbol_kind_type
  Parser::yytranslate_ (int t) YY_NOEXCEPT
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    20,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19
    };
    // Last valid token kind.
    const int code_max = 274;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

#line 18 "/home/gary/projects/neml2-hit/src/Parser.y"
} // nmhit_detail
#line 1593 "/home/gary/projects/neml2-hit/build/Parser.cpp"

#line 188 "/home/gary/projects/neml2-hit/src/Parser.y"


namespace nmhit_detail
{

void
Parser::error(const location_type & loc, const std::string & msg)
{
  driver.report_error(loc, msg);
}

} // namespace nmhit_detail
