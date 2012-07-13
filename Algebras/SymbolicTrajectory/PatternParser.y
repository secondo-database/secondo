/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

Started March 2012, Fabio Vald\'{e}s

[TOC]

\section{Overview}
This is the bison file that parses the user input for the Symbolic Trajectory
Algebra.

\section{Defines and Includes}

*/
%{
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <set>
#include <map>
#include "Pattern.h"

using namespace std;
using namespace stj;

extern int patternlex();
extern FILE* patternin;
extern void pattern_scan_string(const char* yystr);
extern void deleteCurrentBuffer();
bool parseSuccess = true;
void patternerror(const char* s) {
  cerr << endl << s << endl << endl;
  parseSuccess = false;
}
stj::Pattern* wholepat = 0;
Condition cond;
UnitPattern uPat;
ExprList exprList;
bool doublePars(false), firstAssign(true);
string expr;
set<string> curIvs;
unsigned int pos = 0;
%}

%union{
  char* text;
  class UnitPattern* up;
  class Pattern* p;
  class ExprList* el;
}

%name-prefix="pattern"

%token ZZEND
%token<text> ZZVARIABLE ZZCONTENTS ZZWILDCARD ZZDOUBLESLASH ZZVAR_DOT_TYPE
             ZZRIGHTARROW ZZCONST_OP ZZCONTENTS_RESULT ZZVAR_DOT_LABEL ZZASSIGN
             ZZLABELSET ZZERROR
%type<text> variable unitpattern conditionsequence condition expression
            resultsequence result assignment unitpattern_result
            results_assignments assignmentsequence
%type<p> patternsequence
%type<el> expressionlist expressionlistcomma expressionlistparentheses
          expressionlistbrackets expressionlistenclosed
%%
start : patternsequence ZZDOUBLESLASH conditionsequence ZZEND {
          cout << wholepat->toString();
        }
      | patternsequence ZZEND {
          cout << wholepat->toString();
        }
      | patternsequence ZZRIGHTARROW results_assignments ZZEND {
          cout << wholepat->toString();
        }
      | patternsequence ZZDOUBLESLASH conditionsequence ZZRIGHTARROW results_assignments ZZEND {
          cout << wholepat->toString();
        }
      ;

results_assignments : resultsequence ZZDOUBLESLASH assignmentsequence
                    | resultsequence
                    ;

assignmentsequence : assignment
                   | assignmentsequence ',' assignment
                   ;

assignment : ZZVAR_DOT_LABEL ZZASSIGN ZZLABELSET {
               string var($1);
               string labels($3);
               var.assign(var.substr(0, var.find('.')));
               uPat.getUnit(convert(var), true);
               uPat.lbs = stringToSet(labels);
               uPat.wc = NO;
               wholepat->assigns.push_back(uPat);
               cout << "unit added to assignments" << endl;
             }
           ;

resultsequence : result
               | resultsequence result
               ;

result : ZZVARIABLE unitpattern_result {
           uPat.getUnit($1, false);
           if (!uPat.var.empty()) {
             uPat.createUnit($1, $2);
             uPat.ivs = curIvs;
             wholepat->results.push_back(uPat);
             cout << "unit " << $2 << " added to results" << endl;}
           else {
             cout << $1 << " was not found in the pattern" << endl;
           }
         }
       | ZZVARIABLE {
           uPat.getUnit($1, false);
           if (!uPat.var.empty()) {
             wholepat->results.push_back(uPat);
             cout << "unit added to results" << endl;}
           else {
             cout << $1 << " was not found in the pattern" << endl;
           }
         }
       ;

unitpattern_result : '(' ZZCONTENTS_RESULT ')' {
                       $$ = $2;
                     }
                   ;

conditionsequence : condition
                  | conditionsequence ',' condition
                  ;

condition : expressionlist {
              cond.text.assign(exprList.toString());
              cout << "store condition " << cond.text << endl;
              cond.substitute();
              wholepat->conds.push_back(cond);
              exprList.exprs.clear();
              cond.vars.clear();
              cond.keys.clear();
              cond.pIds.clear();
            }
          ;

expression : ZZVAR_DOT_TYPE {
               if (cond.convertVarKey($1) == ERROR) {
                 $$ = convert("");
                 free($1);
               } else {
                 $$ = $1;
               }
             }
           | ZZCONST_OP {
               $$ = $1;
             }
           | expressionlistparentheses {
               string list;
               list.append(exprList.exprs.back());
               $$ = convert(list);
               exprList.exprs.erase(exprList.exprs.end());
               cout << "expressionlistparentheses reads " << $$ << endl;
             }
           | expressionlistbrackets {
               string list;
               list.append(exprList.exprs.back());
               $$ = convert(list);
               exprList.exprs.erase(exprList.exprs.end());
               cout << "expressionlistbrackets reads " << $$ << endl;
             }
           ;

expressionlistparentheses : '(' enclosedlist ')' {
                              int exprSize = exprList.exprs.size();
                              if (exprSize > 0) {
                                exprList.exprs[exprSize - 1].insert(0, "(");
                                exprList.exprs[exprSize - 1].append(")");
                              }
                              $$ = &exprList;
                            }
                          ;

expressionlistbrackets : '[' enclosedlist ']' {
                           int exprSize = exprList.exprs.size();
                           if (exprSize > 0) {
                             exprList.exprs[exprSize - 1].insert(0, "[");
                             exprList.exprs[exprSize - 1].append("]");
                           }
                           $$ = &exprList;
                         }
                       ;

enclosedlist : expression expressionlistenclosed {
                 expr.assign($1);
                 int exprSize = exprList.exprs.size();
                 if (exprSize > 0) {
                   exprList.exprs[exprSize - 1].insert(0, " ");
                   exprList.exprs[exprSize - 1].insert(0, expr);
                 }
                 free($1);
               }
             | expression expressionlistcomma {
                 expr.assign($1);
                 int exprSize = exprList.exprs.size();
                 if (exprSize > 0) {
                   exprList.exprs[exprSize - 1].insert(0, " ");
                   exprList.exprs[exprSize - 1].insert(0, expr);
                 }
                 free($1);
              }
             | expression {
                 expr.assign($1);
                 exprList.exprs.push_back(expr);
                free($1);
               }
             | /* empty */ {
                 expr.clear();
               }
             ;

expressionlistenclosed : expression {
                           expr.assign($1);
                           cout << "one element list = " << $1 << endl;
                           free($1);
                           exprList.exprs.push_back(expr);
                           $$ = &exprList;
                         }
                       | expressionlistenclosed expression {
                           expr.assign($2);
                           int exprSize = exprList.exprs.size();
                           if (exprSize > 0) {
                             exprList.exprs[exprSize - 1].append(" ");
                             exprList.exprs[exprSize - 1].append(expr);
                           }
                           free($2);
                           $$ = &exprList;
                           cout << "expressionlistenclosed reads \"" << exprList.toString() << "\"" << endl;
                         }
                       ;

expressionlistcomma : ',' expression {
                        expr.assign($2);
                        expr.insert(0, ",");
                        cout << "one elem list = " << expr << endl;
                        exprList.exprs.push_back(expr);
                        free($2);
                        $$ = &exprList;
                      }
                    | expressionlistcomma ',' expression {
                        expr.assign($3);
                        int exprSize = exprList.exprs.size();
                        if (exprSize > 0) {
                          exprList.exprs[exprSize - 1].append(",");
                          exprList.exprs[exprSize - 1].append(expr);
                        }
                        free($3);
                        $$ = &exprList;
                        cout << "comma list = " << exprList.toString() << endl;
                      }
                    ;

expressionlist : expression {
                   expr.assign($1);
                   cout << "one element list = " << $1 << endl;
                   exprList.exprs.push_back(expr);
                   free($1);
                   $$ = &exprList;
                 }
               | expressionlist expression {
                   expr.assign($2);
                   int exprSize = exprList.exprs.size();
                   if (exprSize > 0) {
                     exprList.exprs[exprSize - 1].append(" ");
                     exprList.exprs[exprSize - 1].append(expr);
                   }
                   free($2);
                   $$ = &exprList;
                   cout << "condition reads \"" << exprList.toString() << "\"" << endl;
                 }
               ;

patternsequence : variable unitpattern {
                    $$ = wholepat;
                    uPat.createUnit($1, $2);
                    wholepat->patterns.push_back(uPat);
                    free($1);
                    free($2);
                    cout << "pattern #" << wholepat->patterns.size() << endl;
                  }
                | patternsequence variable unitpattern {
                    $$ = wholepat;
                    uPat.createUnit($2, $3);
                    wholepat->patterns.push_back(uPat);
                    free($2);
                    free($3);
                    cout << "pattern #" << wholepat->patterns.size() << endl;
                  }
                ;

variable : ZZVARIABLE
         | /* empty */ {
             $$ = convert("empty");
           }
         ;

unitpattern : '(' ZZCONTENTS ')' {
                $$ = $2;
              }
            | '(' '(' ZZCONTENTS ')' ')' {
                $$ = $3;
                doublePars = true;
              }
            | '(' ')' {
                $$ = convert("");
              }
            | '(' '(' ')' ')' {
                $$ = convert("");
                doublePars = true;
              }
            | ZZWILDCARD
            ;

%%
/*
function ~parseString~
This function is the only one called by the algebra.

*/
Pattern* stj::parseString(const char* input) {
  wholepat = new Pattern();
  pattern_scan_string(input);
  Pattern* result = 0;
  if (patternparse() != 0) {
    cout << "Error found, parsing aborted." << endl;
    parseSuccess = false;
    delete wholepat;
    wholepat = 0;
  }
  else {
    parseSuccess = true;
    result = wholepat;
    result->text.assign(input);
    wholepat = 0;
  }
  uPat.lbs.clear();
  return result;
}

/*
function ~getUnit~
Searches var in the pattern and verifies the correct order of variables in the
result pattern. In case of success, the unit pattern gets the suitable values.

*/
void UnitPattern::getUnit(const char *varP, bool assign) {
  if (assign && firstAssign) {
    pos = 0; // reset counter for first assignment
    firstAssign = false;
  }
  string varStr(varP);
  bool found = false;
  while ((pos < wholepat->patterns.size()) && !found) { // look for the variable
    if (!(wholepat->patterns)[pos].var.compare(varStr)) {
      var.assign((wholepat->patterns)[pos].var);
      ivs = (wholepat->patterns)[pos].ivs;
      lbs = (wholepat->patterns)[pos].lbs;
      wc = (wholepat->patterns)[pos].wc;
      found = true;
      curIvs = ivs;
      cout << "variable " << var << " found in pattern " << pos << endl;
    }
    pos++;
  }
  if (!found) {
    var.clear();
    cout << "variable " << varStr << " not found" << endl;
  }
}

void UnitPattern::setUnit(const char *v, const char *i,
                          const char *l, const char *w) {
  string lstr(l), istr(i);
  var.assign(v);
  ivs = stringToSet(istr);
  lbs = stringToSet(lstr);
  string wcstr(w);
  if (!wcstr.compare("*")) {
    wc = STAR;
  }
  else if (!wcstr.compare("+")) {
    wc = PLUS;
  }
  else {
    wc = NO;
  }
}

/*
function ~createUnit~

Modifies the unit pattern according to the parameters.
*/
void UnitPattern::createUnit(const char *varP, const char *pat) {
  string patstr(pat), varstr(varP), token;
  int pos = 0;
  while ((pos = varstr.find(' ', pos)) != string::npos) {
    varstr.erase(pos, 1);
  }
  if (varstr.compare("") && varstr.compare("*") && varstr.compare("+")
     && (varstr.at(0) > 64) && (varstr.at(0) < 91)) {
    var.assign(varstr);
  }
  else {
    var.clear();
  }
  vector<string> pattern = splitPattern(patstr);
  if (doublePars) {
    wc = PLUS;
  }
  if (pattern.size() == 2) {
    if (!pattern[0].compare("_")) { // (_ a)
      ivs.clear();
    }
    else {
      ivs = stringToSet(pattern[0]);
    }
    if (!pattern[1].compare("_")) {
      lbs.clear();
    }
    else {
      lbs = stringToSet(pattern[1]);
    }
    if (!doublePars) {
      wc = NO;
    }
  }
  else if ((pattern.size() == 1) && (!pattern[0].compare("*")
                                  || !pattern[0].compare("+"))) {
    wc = (pattern[0].compare("*") ? PLUS : STAR);
    ivs.clear();
    lbs.clear();
  }
  else if ((pattern.size() == 1) && pattern[0].empty() && !doublePars) {
    wc = NO;
  }
  else if ((pattern.size() == 0) || ((pattern.size() == 1)
       && (!pattern[0].compare("*") || !pattern[0].compare("+")))) {
    ivs.clear();
    lbs.clear();
  }
  if ((pattern.size() == 0) && !doublePars) {
    wc = NO;
  }
  doublePars = false;
}

/*
function ~substitute~
variable.type is substituted by a value of the same type in order to be able
to test-execute the condition as a query.

*/
void Condition::substitute() {
  string varKey;
  unsigned int i = 0;
  textSubst.assign(text);
  while (i < keys.size()) {
    varKey.assign(vars[i]);
    varKey.append(types[keys[i]]);
    int pos = textSubst.find(varKey);
    textSubst.replace(pos, varKey.size(), subst[keys[i]]);
    i++;
  }
}

/*
function ~convertVarKey~
Checks whether the variable var occurs in the pattern and whether the key k
is valid; returns the recognized key.

*/
Key Condition::convertVarKey(const char *varKey) {
  string input(varKey), var;
  Key key;
  int dotpos = input.find('.');
  string varInput(input.substr(0, dotpos));
  string kInput(input.substr(dotpos + 1));
  for (unsigned int i = 0; i < wholepat->patterns.size(); i++) {
    if (!varInput.compare(((wholepat->patterns)[i]).var)) {
      var.assign(varInput);
      if (!kInput.compare("label"))
        key = LABEL;
      else if (!kInput.compare("time"))
        key = TIME;
      else if (!kInput.compare("start"))
        key = START;
      else if (!kInput.compare("end"))
        key = END;
      else if (!kInput.compare("card"))
        key = CARD;
      else
        key = ERROR;
      cond.vars.push_back(var);
      cond.keys.push_back(key);
      cond.pIds.push_back(i);
      cout << varInput << " | pat #" << i << " | cond #" << wholepat->conds.size() << endl;
      return key;
    }
  }
  cout << "variable " << var << " does not exist in the pattern" << endl;
  return ERROR;
}

void Condition::clear() {
  text.clear();
  subst.clear();
  keys.clear();
  vars.clear();
  pIds.clear();
}

string ExprList::toString() {
  string result(exprs[0]);
  for (unsigned int i = 1; i < exprs.size(); i++) {
    result.append(" ");
    result.append(exprs[i]);
  }
  return result;
}
