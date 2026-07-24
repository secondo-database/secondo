/*
----
This file is part of SECONDO.

Copyright (C) 2026,
University in Hagen,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 Language detection for typed commands

The single definition of the rules described in ~SQLLanguage.h~. The rule set is
the one the retired hybrid TTY (~SecondoPLTTYMode::isSqlCommand~) and the
JavaGUI (~CommandPanel.optimize~) independently arrived at; it now exists once.

*/

#include "SQLLanguage.h"

#include <cctype>
#include <vector>

using namespace std;

const string SQL_BLANKS = " \t\n\r\f\v\b\a";

/*
2 Tokenizing

Split ~cmd~ into at most ~maxTok~ lower-cased tokens. The delimiter set includes
the bracket and assignment characters, so that "let r5=select ..." and
"create table t(a)" tokenize the way the rules below expect.

*/
static void sqlTokens(const string& cmd, vector<string>& toks, size_t maxTok)
{
  const string delims = SQL_BLANKS + "([{=.,;";
  size_t pos = 0;
  while ( toks.size() < maxTok )
  {
    size_t s = cmd.find_first_not_of(delims, pos);
    if ( s == string::npos ) break;
    size_t e = cmd.find_first_of(delims, s);
    string t = cmd.substr(s, (e==string::npos) ? string::npos : e - s);
    for ( size_t i = 0; i < t.size(); i++ )
    {
      t[i] = tolower((unsigned char) t[i]);
    }
    toks.push_back(t);
    if ( e == string::npos ) break;
    pos = e;
  }
}

/*
3 SQL

*/
bool looksLikeSql(const string& cmd)
{
  size_t s = cmd.find_first_not_of(SQL_BLANKS);
  if ( s == string::npos ) return false;
  // nested list command or command sequence: always the kernel
  if ( cmd[s] == '(' || cmd[s] == '{' ) return false;

  vector<string> t;
  sqlTokens( cmd, t, 3 );
  if ( t.empty() ) return false;

  // unambiguous openers
  if (    t[0] == "sql"   || t[0] == "select" || t[0] == "union"
       || t[0] == "intersection" ) return true;

  // ambiguous with kernel commands: need the second token
  if ( t.size() < 2 ) return false;
  if ( t[0] == "delete" && t[1] == "from" ) return true;
  if ( t[0] == "insert" && t[1] == "into" ) return true;
  // The optimizer knows exactly two things to create and to drop. Everything
  // else the two words open is the kernel's -- notably "delete database X",
  // which is how a database is dropped, and which must reach the kernel to be
  // told that one is still open (ERR_DATABASE_OPEN).
  if ( t[0] == "create" && (t[1] == "table" || t[1] == "index") ) return true;
  if ( t[0] == "drop"   && (t[1] == "table" || t[1] == "index") ) return true;

  // ... or the third
  if ( t.size() < 3 ) return false;
  if ( t[0] == "update" && t[2] == "set" ) return true;
  // "let <ident> = select|union|intersection ...": an SQL right-hand side. The
  // server splits the prefix off and re-wraps the generated plan.
  if ( t[0] == "let" && (   t[2] == "select" || t[2] == "union"
                         || t[2] == "intersection" ) ) return true;

  return false;
}

/*
4 Optimizer control directives

The goals the optimizer offers besides SQL. Recognized when typed bare; with an
explicit optimizer prefix any non-SQL text is a directive, so this list is
only the convenience case.

*/
bool isOptimizerDirective(const string& cmd)
{
  size_t s = cmd.find_first_not_of(SQL_BLANKS);
  if ( s == string::npos ) return false;
  size_t nameEnd = cmd.find_first_of(SQL_BLANKS + "(", s);
  string name = cmd.substr(s, (nameEnd==string::npos) ? string::npos
                                                      : nameEnd - s);
  // The directive names are Prolog atoms, matched case-sensitively; the
  // argument (if any) starts at the '('.
  static const char* const directives[] = {
    "setOption", "delOption", "showOptions",
    "loadOptions", "saveOptions", "defaultOptions",
    "updateCatalog", "resetKnowledgeDB",
    "helpMe" };   // advertised by the showOptions listing itself
  for ( size_t i = 0; i < sizeof(directives)/sizeof(directives[0]); i++ )
  {
    if ( name == directives[i] ) return true;
  }
  return false;
}

/*
5 Everything else is a kernel command

~deriveKernelCommandLevel~ is inline in the header; see the note there.

*/
int resolveCommandLevel(const string& cmd, bool optimizerAddressed)
{
  if ( looksLikeSql( cmd ) ) return CMD_LEVEL_SQL;
  if ( optimizerAddressed || isOptimizerDirective( cmd ) )
  {
    return CMD_LEVEL_OPT_DIRECTIVE;
  }
  return deriveKernelCommandLevel( cmd );
}

/*
6 The ~optimizer~ prefix

*/
bool stripOptimizerPrefix(const string& cmd, string& rest)
{
  rest = cmd;
  size_t s = cmd.find_first_not_of(SQL_BLANKS);
  if ( s == string::npos ) return false;

  const string kw = "optimizer";
  if ( cmd.size() - s < kw.size() + 1 ) return false;
  for ( size_t i = 0; i < kw.size(); i++ )
  {
    if ( tolower((unsigned char) cmd[s+i]) != kw[i] ) return false;
  }
  // must be a whole word, and something has to follow it
  size_t after = s + kw.size();
  if ( !isspace((unsigned char) cmd[after]) ) return false;
  size_t r = cmd.find_first_not_of(SQL_BLANKS, after);
  if ( r == string::npos ) return false;

  rest = cmd.substr(r);
  return true;
}
