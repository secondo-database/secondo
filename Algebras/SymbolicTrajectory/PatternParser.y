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
void deleteCurrentPatternBuffer();
bool parseSuccess = true;
void patternerror(const char* s) {
  cerr << endl << s << endl << endl;
  parseSuccess = false;
}
stj::Pattern* wholepat = 0;
Condition cond;
vector<UPat> uPats;
Assign assign;
ExprList exprList;
bool doublePars(false), firstAssign(true), assignNow(false), easyCond(true);
string regEx, expr;
set<string> curIvs, patVars, resultVars;
unsigned int pos(0);
char* errMsg;
%}

%union{
  char* text;
  class UPat* up;
  class Pattern* p;
  class ExprList* el;
}

%name-prefix="pattern"

%token ZZEND
%token<text> ZZVARIABLE ZZCONTENTS ZZEMPTYPAT ZZWILDCARD ZZDOUBLESLASH
             ZZVAR_DOT_TYPE ZZRIGHTARROW ZZCONST_OP ZZASSIGN ZZOPENREGEX
             ZZCLOSEREGEX ZZERROR
%type<text> variable patternelement conditionsequence condition expression
            resultsequence result assignment element regexseq separator
            results_assignments assignmentsequence
%type<p> patternsequence
%type<el> expressionlist expressionlistcomma expressionlistparentheses
          expressionlistbrackets expressionlistenclosed assignment_expressionlist
%%
start : patternsequence ZZDOUBLESLASH conditionsequence ZZEND {
        }
      | patternsequence ZZEND { cout << "pattern has " << wholepat->getSize() << " elements" << endl;
        }
      | patternsequence ZZRIGHTARROW results_assignments ZZEND {
          wholepat->collectAssVars();
        }
      | patternsequence ZZDOUBLESLASH conditionsequence ZZRIGHTARROW results_assignments ZZEND {
          wholepat->collectAssVars();
        }
      ;

results_assignments : resultsequence ZZDOUBLESLASH assignmentsequence
                    | resultsequence
                    ;

assignmentsequence : assignment
                   | assignmentsequence ',' assignment
                   ;

assignment : ZZVAR_DOT_TYPE ZZASSIGN assignment_expressionlist {
               string var($1);
               string arg = $3->toString();
               string type = var.substr(var.find('.') + 1);
               pair<string, int> varKey;
               var.assign(var.substr(0, var.find('.')));
               int posR = wholepat->getResultPos(var);
               if (posR == -1) {
                 errMsg = convert("variable " + var + " not found in results");
                 yyerror(errMsg);
                 free($1);
                 YYERROR;
               }
               int posP = wholepat->getPatternPos(var);
               if (posP > -1) {
                 if (wholepat->getPat(posP).getW()) {
                   errMsg = convert("assignment for sequence not allowed");
                   yyerror(errMsg);
                   free($1);
                   YYERROR;
                 }
               }
               int key = getKey(type);
               if (key < 6) {
                 bool redundant = false;
                 if (key == 1) {
                   for (int i = 1; i <= 5; i++) {
                     if (!wholepat->getAssign(posR).getText(i).empty()) {
                       redundant = true;
                     }
                   }
                 }
                 else if (key > 1) {
                   if (!wholepat->getAssign(posR).getText(1).empty()
                    || !wholepat->getAssign(posR).getText(key).empty()) {
                     redundant = true;
                   }
                 }
                 else {
                   if (!wholepat->getAssign(posR).getText(0).empty()) {
                     redundant = true;
                   }
                 }
                 if (redundant) {
                   errMsg = convert("redundant assignment(s)");
                   assign.clear();
                   exprList.exprs.clear();
                   yyerror(errMsg);
                   free($1);
                   YYERROR;
                 }
                 wholepat->setAssign(posR, posP, key, arg);
/*                  cout << "setAssign(" << posR << ", " << posP << ", " << key << ", " << arg << ")" << endl; */
                 while (assign.getRightSize(6)) { // distribute assignments
                   varKey = assign.getVarKey(6);
                   wholepat->addAssignRight(posR, key, varKey);
                   assign.removeUnordered();
                 }
               }
               else {
                 errMsg = convert("type \"" + type + "\" is invalid");
                 yyerror(errMsg);
                 free($1);
                 YYERROR;
               }
               assign.clear();
               exprList.exprs.clear();
               free($1);
             }
           ;

assignment_expressionlist : expression {
                   expr.assign($1);
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
                 }
               ;

resultsequence : result
               | resultsequence result
               | /* empty */ {}
               ;

result : ZZVARIABLE {
           assignNow = true;
           string var($1);
           if (resultVars.count(var)) {
             errMsg = convert("result variables must be unique");
             yyerror(errMsg);
             YYERROR;
           }
           else {
             assign.init(var, wholepat->getPatternPos(var));
             wholepat->addAssign(assign);
             resultVars.insert(var);
             free($1);
           }
         }
       ;

conditionsequence : condition
                  | conditionsequence ',' condition
                  ;

condition : expressionlist {
              cond.setText(exprList.toString());
              if (easyCond) {
                wholepat->addEasyCond(cond);
              }
              else {
                wholepat->addCond(cond);
              }
              easyCond = true;
              exprList.exprs.clear();
              cond.clearVectors();
            }
          ;

expression : ZZVAR_DOT_TYPE {
               if (cond.convertVarKey($1) == -1) {
                 string varDotType($1);
                 /*errMsg = convert("error: " + varDotType + " not accepted");
                 yyerror(errMsg);*/
                 YYERROR;
                 free($1);
               } else {
                 if (assignNow) {
                   if (!assign.convertVarKey($1)) {
                     string varDotType($1);
/*                     errMsg = convert("error: " + varDotType + " not accepted");
                     yyerror(errMsg);*/
                     YYERROR;
                     free($1);
                   }
                 }
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
/*                cout << "expressionlistparentheses reads " << $$ << endl; */
             }
           | expressionlistbrackets {
               string list;
               list.append(exprList.exprs.back());
               $$ = convert(list);
               exprList.exprs.erase(exprList.exprs.end());
/*                cout << "expressionlistbrackets reads " << $$ << endl; */
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
/*                            cout << "one element list = " << $1 << endl; */
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
/*                            cout << "expressionlistenclosed reads \"" << exprList.toString() << "\"" << endl; */
                         }
                       ;

expressionlistcomma : ',' expression {
                        expr.assign($2);
                        expr.insert(0, ",");
/*                         cout << "one elem list = " << expr << endl; */
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
/*                         cout << "comma list = " << exprList.toString() << endl; */
                      }
                    ;

expressionlist : expression {
                   expr.assign($1);
/*                    cout << "one element list = " << $1 << endl; */
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
/*                    cout << "condition reads \"" << exprList.toString() << "\"" << endl; */
                 }
               ;

patternsequence : patternsequence variable element {
                    $$ = wholepat;
                    string var($2);
                    if (patVars.count(var)) {
                      errMsg = convert("double variable " + var);
                      free($2);
                      yyerror(errMsg);
                      YYERROR;
                    }
                    else if (var != "empty") {
                      patVars.insert(var);
                    }
                    wholepat->addRegExSymbol(regEx.c_str());
                    for (unsigned int i = 0; i < uPats.size(); i++) {
                      wholepat->addVarPos(var, wholepat->getSize());
                      uPats[i].setVar(var);
                      wholepat->addUPat(uPats[i]);
                    }
                    uPats.clear();
                    regEx.clear();
                    free($2);
                  }
                | variable element {
                    string var($1);
                    $$ = wholepat;
                    if (patVars.count(var)) {
                      errMsg = convert("double variable " + var);
                      free($1);
                      yyerror(errMsg);
                      YYERROR;
                    }
                    else if (var != "empty") {
                      patVars.insert(var);
                    }
                    wholepat->addRegExSymbol(regEx.c_str());
                    for (unsigned int i = 0; i < uPats.size(); i++) {
                      uPats[i].setVar(var);
                      wholepat->addVarPos(var, wholepat->getSize());
                      wholepat->addUPat(uPats[i]);
                    }
                    uPats.clear();
                    regEx.clear();
                    free($1);
                  }
                ;

element : patternelement {
            regEx += int2String(wholepat->getSize() + uPats.size());
            UPat upat($1);
            uPats.push_back(upat);
            free($1);
          }
        | ZZOPENREGEX regexseq ZZCLOSEREGEX {
            string open($1), close($3);
            regEx.insert(0, open);
            regEx.append(close);
            free($1);
            free($3);
          }
        ;

regexseq : regexseq separator element
         | element
         ;

separator : '|' {
              regEx += "|";
            }
          | /* empty */ {}
          ;

variable : ZZVARIABLE
         | /* empty */ {
             $$ = convert("empty");
           }
         ;

patternelement : '(' ZZCONTENTS ')' {
                $$ = $2;
              }
            | '(' ')' {
                $$ = convert("");
              }
            | ZZWILDCARD
            | ZZEMPTYPAT
            ;

%%
/*
function ~parseString~
This function is the only one called by the algebra.

*/
Pattern* stj::parseString(const char* input, bool classify = false) {
  wholepat = new Pattern();
  patternFlushBuffer();
  pattern_scan_string(input);
  Pattern* result = 0;
  cond.clear();
  uPats.clear();
  resultVars.clear();
  patVars.clear();
  regEx.clear();
  firstAssign = true;
  assignNow = false;
  easyCond = true;
  if (patternparse() != 0) {
    cout << "Error found, parsing aborted." << endl;
    parseSuccess = false;
    delete wholepat;
    wholepat = 0;
  }
  else {
    parseSuccess = true;
    result = wholepat;
    result->setText(input);
    wholepat = 0;
  }
  if (result) {
    result->setVerified(false);
    if (!classify) { // classification => no single NFA needed
      result->initDelta();
      result->buildNFA();
    }
  }
  deleteCurrentPatternBuffer();
  return result;
}

/*
function ~getResultPos~
Searches ~v~ in the results. A returned value of -1 means that ~v~ does not
occur in the results.

*/
int Pattern::getResultPos(const string v) {
  string var(v);
  for (int i = 0; i < (int)getAssigns().size(); i++) {
    if (getAssign(i).getV() == var) {
      return i;
    }
  }
  return -1;
}

/*
function ~getUnit~
Searches varP in the pattern and verifies the correct order of variables in the
result pattern. In case of success, the unit pattern gets the suitable values.

*/
void UPat::getUnit(const char *varP, bool order) {
  if (!order || firstAssign) {
    pos = 0; // reset counter for first assignment
    firstAssign = false;
  }
  string varStr(varP);
  bool found = false;
  while ((pos < wholepat->getPats().size()) && !found) { // look for var
    if (!(wholepat->getPat(pos)).var.compare(varStr)) {
      var.assign((wholepat->getPat(pos)).var);
      ivs = (wholepat->getPat(pos)).ivs;
      lbs = (wholepat->getPat(pos)).lbs;
      wc = (wholepat->getPat(pos)).wc;
      found = true;
      curIvs = ivs;
/*       cout << "variable " << var << " found in pattern " << pos << endl; */
    }
    pos++;
  }
  if (!found) {
    var.clear();
    cout << "variable " << varStr << " not found" << endl;
  }
}

void UPat::setUnit(const char *v, const char *i, const char *l, const char *w) {
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
Constructor for class ~UPat~

*/
UPat::UPat(const char *contents) : wc(NO) {
  string input(contents);
  vector<string> parts = splitPattern(input);
  if (parts.size() == 2) {
    ivs = stringToSet(parts[0]);
    lbs = stringToSet(parts[1]);
  }
  else if (parts.size() == 1) {
    if (parts[0] == "*") {
      wc = STAR;
    }
    else if (parts[0] == "+") {
      wc = PLUS;
    }
    else if (parts[0] == "<>") {
      wc = EMPTY;
    }
  }
}

/*
function ~convertVarKey~
Checks whether the variable var occurs in the pattern and whether the key k
is valid; returns the recognized key.

*/
int Condition::convertVarKey(const char *varKey) {
  string input(varKey), var;
  int key;
  int dotpos = input.find('.');
  string varInput(input.substr(0, dotpos));
  string kInput(input.substr(dotpos + 1));
  for (unsigned int i = 0; i < wholepat->getPats().size(); i++) {
    if (!varInput.compare((wholepat->getPat(i)).getV())) {
      var.assign(varInput);
      key = ::getKey(kInput);
      if (!key && wholepat->getPat(i).getW()) {
        cout << "label condition not allowed for sequences" << endl;
        return -1;
      }
      if ((key > 5) && !wholepat->getPat(i).getW()) {
        cout << "card / labels condition not allowed for non-sequences" << endl;
        return -1;
      }
      if (easyCond) {
        if (wholepat->getPat(i).getW() || (vars.size() && (var !=vars.back()))){
          easyCond = false;
        }
      }
      vars.push_back(var);
      keys.push_back(key);
      pIds.push_back(i);
      return key;
    }
  }
  cout << "variable " << varInput << " does not exist in the pattern" << endl;
  return -1;
}

bool Assign::convertVarKey(const char *varKey) {
  string input(varKey);
  int dotpos = input.find('.');
  string varInput(input.substr(0, dotpos));
  string kInput(input.substr(dotpos + 1));
  pair<string, int> right;
  for (unsigned int i = 0; i < wholepat->getPats().size(); i++) {
    if (varInput == (wholepat->getPat(i)).getV()) {
      right.first = varInput;
      right.second = getKey(kInput);
      addRight(6, right); // assign to n \in {0, 1, ..., 5} afterwards
    }
  }
  return true;
}

void Pattern::collectAssVars() {
  for (int i = 0; i < (int)assigns.size(); i++) {
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < assigns[i].getRightSize(j); k++) {
        assignedVars.insert(assigns[i].getRightVar(j, k));
      }
    }
  }
  assign.clear();
}

int Pattern::getPatternPos(const string var) {
  for (int i = 0; i < (int)elems.size(); i++) {
    if (elems[i].getV() == var) {
      return i;
    }
  }
  return -1;
}

void Condition::clear() {
  text.clear();
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
