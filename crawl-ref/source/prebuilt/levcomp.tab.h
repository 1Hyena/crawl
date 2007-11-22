/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DEFAULT_DEPTH = 258,
     SHUFFLE = 259,
     SUBST = 260,
     TAGS = 261,
     KFEAT = 262,
     KITEM = 263,
     KMONS = 264,
     KMASK = 265,
     NAME = 266,
     DEPTH = 267,
     ORIENT = 268,
     PLACE = 269,
     CHANCE = 270,
     MONS = 271,
     ITEM = 272,
     MARKER = 273,
     COLOUR = 274,
     PRELUDE = 275,
     MAIN = 276,
     VALIDATE = 277,
     VETO = 278,
     NSUBST = 279,
     WELCOME = 280,
     LFLAGS = 281,
     BFLAGS = 282,
     FLOORCOL = 283,
     ROCKCOL = 284,
     COMMA = 285,
     INTEGER = 286,
     CHARACTER = 287,
     STRING = 288,
     MAP_LINE = 289,
     MONSTER_NAME = 290,
     ITEM_INFO = 291,
     LUA_LINE = 292
   };
#endif
#define DEFAULT_DEPTH 258
#define SHUFFLE 259
#define SUBST 260
#define TAGS 261
#define KFEAT 262
#define KITEM 263
#define KMONS 264
#define KMASK 265
#define NAME 266
#define DEPTH 267
#define ORIENT 268
#define PLACE 269
#define CHANCE 270
#define MONS 271
#define ITEM 272
#define MARKER 273
#define COLOUR 274
#define PRELUDE 275
#define MAIN 276
#define VALIDATE 277
#define VETO 278
#define NSUBST 279
#define WELCOME 280
#define LFLAGS 281
#define BFLAGS 282
#define FLOORCOL 283
#define ROCKCOL 284
#define COMMA 285
#define INTEGER 286
#define CHARACTER 287
#define STRING 288
#define MAP_LINE 289
#define MONSTER_NAME 290
#define ITEM_INFO 291
#define LUA_LINE 292




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 46 "levcomp.ypp"
typedef union YYSTYPE {
    int i;
    const char *text;
    raw_range range;
} YYSTYPE;
/* Line 1274 of yacc.c.  */
#line 119 "levcomp.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



