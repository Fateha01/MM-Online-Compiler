/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT_LIT = 258,
     FLOAT_LIT = 259,
     IDENT = 260,
     STRING_LIT = 261,
     KW_INT = 262,
     KW_FLOAT = 263,
     KW_BOOL = 264,
     KW_STRING = 265,
     KW_IF = 266,
     KW_ELSE = 267,
     KW_WHILE = 268,
     KW_FOR = 269,
     KW_TRUE = 270,
     KW_FALSE = 271,
     KW_CLASS = 272,
     KW_PUBLIC = 273,
     KW_STATIC = 274,
     KW_VOID = 275,
     KW_MAIN = 276,
     KW_STRING_ARR = 277,
     SYS_OUT_PRINTLN = 278,
     ASSIGN = 279,
     SEMI = 280,
     LBRACE = 281,
     RBRACE = 282,
     LPAREN = 283,
     RPAREN = 284,
     COMMA = 285,
     DOT = 286,
     PLUS = 287,
     MINUS = 288,
     STAR = 289,
     SLASH = 290,
     PERCENT = 291,
     LT = 292,
     GT = 293,
     LE = 294,
     GE = 295,
     EQ = 296,
     NE = 297,
     AND = 298,
     OR = 299,
     NOT = 300,
     UMINUS = 301
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 13 "src/parser/parser.y"

    int ival;
    double fval;
    char *sval;
    struct ASTNode *node;



/* Line 1685 of yacc.c  */
#line 106 "build/parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


