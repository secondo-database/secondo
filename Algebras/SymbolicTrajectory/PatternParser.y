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
#include "Algorithms.h"

using namespace std;
using namespace stj;

extern int patternlex();
extern FILE* patternin;
extern void pattern_scan_string(const char* yystr);
extern void deleteCurrentBuffer();
void deleteCurrentPatternBuffer();
int getOpenPos(vector<int> &posBeforeOpen, string &regEx) {
  if (posBeforeOpen.empty()) {
    cout << "vector is empty" << endl;
    return -1;
  }
  int patElem = posBeforeOpen.back();
  posBeforeOpen.pop_back();
  return regEx.find_first_of(int2Str(patElem));
}
bool parseSuccess = true;
void patternerror(const char* s) {
  cerr << endl << s << endl << endl;
  parseSuccess = false;
}
stj::Pattern* wholepat = 0;
Condition cond;
vector<PatElem> pElems;
Assign assign;
set<string> unitVars;
ExprList exprList;
bool doublePars(false), firstAssign(true), assignNow(false), easyCond(true);
string regEx, expr;
set<string> curIvs, patVars, resultVars, condVars;
unsigned int pos(0), regExLength(1), numOfElems(0);
vector<int> posBeforeOpen;
char* errMsg;
%}

%union{
  char* text;
  class PatElem* up;
  class Pattern* p;
  class ExprList* el;
}

%name-prefix="pattern"

%token ZZEND
%token<text> ZZVARIABLE ZZCONTENTS ZZWILDCARD ZZDOUBLESLASH ZZVAR_DOT_TYPE 
             ZZRIGHTARROW ZZCONST_OP ZZASSIGN ZZOPENREGEX ZZCLOSEREGEX ZZERROR
%type<text> variable patternelement conditionsequence condition expression
            resultsequence result assignment element separator regexseq
            results_assignments assignmentsequence
%type<p> patternsequence
%type<el> expressionlist expressionlistcomma expressionlistparentheses
          expressionlistbrackets expressionlistenclosed assignment_expressionlist
%%
start : patternsequence ZZDOUBLESLASH conditionsequence ZZEND {}
      | patternsequence ZZEND {}
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
                 errMsg = Tools::convert("variable " + var + " not found in results");
                 yyerror(errMsg);
                 free($1);
                 YYERROR;
               }
               int posP = wholepat->getPatternPos(var);
               if (posP > -1) {
                 if (!unitVars.count(var)) {
                   errMsg = Tools::convert("assignment for sequence not allowed");
                   assign.clear();
                   exprList.exprs.clear();
                   yyerror(errMsg);
                   free($1);
                   YYERROR;
                 }
               }
               int key = Tools::getKey(type);
               bool redundant = false;
               if (key == 2) { // time
                 for (int i = 2; i <= 6; i++) {
                   if (!wholepat->getAssign(posR).getText(i).empty()) {
                     redundant = true;
                   }
                 }
               }
               else if ((key > 2) && (key < 7)) { // start, end, lc, rc
                 if (!wholepat->getAssign(posR).getText(2).empty()
                 || !wholepat->getAssign(posR).getText(key).empty()) {
                   redundant = true;
                 }
               }
               else {
                 if (!wholepat->getAssign(posR).getText(0).empty() ||
                     !wholepat->getAssign(posR).getText(1).empty() ||
                     !wholepat->getAssign(posR).getText(8).empty() ||
                     !wholepat->getAssign(posR).getText(9).empty()) {
                   redundant = true;
                 }
               }
               if (redundant) {
                 errMsg = Tools::convert("redundant assignment(s)");
                 yyerror(errMsg);
                 free($1);
                 YYERROR;
               }
               wholepat->setAssign(posR, posP, key, arg);
             /*                  cout << "setAssign(" << posR << ", " << posP << ", " << key << ", " << arg << ")" << endl; */
               while (assign.getRightSize(9)) { // distribute assignments
                 varKey = assign.getVarKey(9);
                 wholepat->addAssignRight(posR, key, varKey);
                 assign.removeUnordered();
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
             errMsg = Tools::convert("result variables must be unique");
             yyerror(errMsg);
             YYERROR;
           }
           else {
             assign.init(var, wholepat->getPatternPos(var));
             wholepat->addAssign(assign);
             resultVars.insert(var);
             wholepat->addRelevantVar(var);
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
                wholepat->addEasyCond(wholepat->getVarPos(cond.getVar(0)).first, cond);
              }
              else {
                wholepat->addCond(cond);
              }
              easyCond = true;
              exprList.exprs.clear();
              cond.clearVectors();
              condVars.clear();
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
/*                      string varDotType($1); */
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
               $$ = Tools::convert(list);
               exprList.exprs.erase(exprList.exprs.end());
/*                cout << "expressionlistparentheses reads " << $$ << endl; */
             }
           | expressionlistbrackets {
               string list;
               list.append(exprList.exprs.back());
               $$ = Tools::convert(list);
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
                      errMsg = Tools::convert("double variable " + var);
                      free($2);
                      yyerror(errMsg);
                      YYERROR;
                    }
                    else if (var != "") {
                      patVars.insert(var);
                      if ((pElems.size() == 1) && (pElems[0].getW() == NO) &&
                          (regEx[0] != '(')) {
                        unitVars.insert(var);
                      }
                    }
                    wholepat->addRegExSymbol(regEx.c_str());
                    for (unsigned int i = 0; i < pElems.size(); i++) {
                      wholepat->addVarPos(var, wholepat->getSize());
                      pElems[i].setVar(var);
                      wholepat->addPatElem(pElems[i]);
                    }
                    numOfElems++;
                    pElems.clear();
                    regEx.clear();
                    free($2);
                  }
                | variable element {
                    string var($1);
                    $$ = wholepat;
                    if (patVars.count(var)) {
                      errMsg = Tools::convert("double variable " + var);
                      free($1);
                      yyerror(errMsg);
                      YYERROR;
                    }
                    else if (var != "") {
                      patVars.insert(var);
                      if ((pElems.size() == 1) && (pElems[0].getW() == NO) &&
                          (regEx[0] != '(')) {
                        unitVars.insert(var);
                      }
                    }
                    wholepat->addRegExSymbol(regEx.c_str());
                    for (unsigned int i = 0; i < pElems.size(); i++) {
                      pElems[i].setVar(var);
                      wholepat->addVarPos(var, wholepat->getSize());
                      wholepat->addPatElem(pElems[i]);
                    }
                    numOfElems++;
                    pElems.clear();
                    regEx.clear();
                    free($1);
                  }
                ;

element : patternelement {
            PatElem pElem($1);
            if (!pElem.isOk()) {
              errMsg = Tools::convert("invalid time information in pattern element");
              free($1);
              yyerror(errMsg);
              YYERROR;
            }
            string wildcard = (pElem.getW() == STAR ? "*" : (pElem.getW() == PLUS ? "+" : ""));
            regEx += Tools::int2String(wholepat->getSize() + pElems.size()) + wildcard + " ";
            wholepat->addAtomicPos();
            pElems.push_back(pElem);
            free($1);
          }
        | ZZOPENREGEX regexseq ZZCLOSEREGEX {
            string close($3);
            regEx.insert(getOpenPos(posBeforeOpen, regEx), "(");
            regEx.append(")");
            if (close.length() > 1) {
              regEx.append(close.substr(1));
            }
            free($1);
            free($3);
          }
        ;

regexseq : regexseq separator element
         | element {posBeforeOpen.push_back(wholepat->getSize() + pElems.size() - 1);}
         ;

separator : '|' {
              regEx += "|";
            }
          | /* empty */ {
            }
          ;

variable : ZZVARIABLE
         | /* empty */ {
             $$ = Tools::convert("");
           }
         ;

patternelement : '(' ZZCONTENTS ')'{
                   $$ = $2;
                 }
               | '(' ')' {
                   $$ = Tools::convert("");
                 }
               | ZZWILDCARD {}
               ;

%%
/*
function ~parseString~
This function is the only one called by the algebra.

*/
Pattern* stj::parseString(const char* input, bool classify = false) {
  wholepat = new Pattern(0);
  patternFlushBuffer();
  pattern_scan_string(input);
  Pattern* result = 0;
  posBeforeOpen.clear();
  cond.clear();
  pElems.clear();
  resultVars.clear();
  patVars.clear();
  condVars.clear();
  regEx.clear();
  unitVars.clear();
  assign.clear();
  exprList.exprs.clear();
  firstAssign = true;
  assignNow = false;
  easyCond = true;
  numOfElems = 0;
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
    if (!classify) { // classification => no single NFA needed
      result->parseNFA();
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
function ~stringToSet~

Converts a string containing time, label, or place information for internal
storage.

*/
void PatElem::stringToSet(const string& input, bool isTime) {
  string element, contents(input);
  pair<string, unsigned int> place;
  bool isPlace = false;
  int limitpos;
  contents.erase(0, input.find_first_not_of(" "));
  if (contents == "_") {
    return;
  }
  if (contents.substr(0, 8) == "disjoint") {
    setRel = DISJOINT;
    contents = contents.substr(8);
  }
  if (contents.substr(0, 8) == "superset") {
    setRel = SUPERSET;
    contents = contents.substr(8);
  }
  if (contents.substr(0, 5) == "equal") {
    setRel = EQUAL;
    contents = contents.substr(5);
  }
  if (contents.substr(0, 9) == "intersect") {
    setRel = INTERSECT;
    contents = contents.substr(9);
  }
  if (contents[0] == '{') {
    contents = contents.substr(1, contents.length() - 2);
  }
  while (!contents.empty()) {
    contents.erase(0, contents.find_first_not_of(", "));
    if ((contents[0] == '\"') || (contents[0] == '\'')) { //label/time inside qm
      limitpos = contents.find('\"', 1);
      element = contents.substr(1, limitpos - 1);
      contents.erase(0, limitpos + 1);
      isPlace = false;
    }
    else if (contents[0] == '(') { // place
      limitpos = contents.find(')');
      element = contents.substr(1, limitpos - 1);
      contents.erase(0, limitpos + 1);
      element.erase(0, element.find_first_not_of(" "));
      place.first = element.substr(1, element.find_first_of("\"\'", 1) - 1);
      element.erase(0, element.find_first_of("\"\'", 1) + 1);
      element.erase(0, element.find_first_not_of(" "));
      place.second = Tools::str2Int(element);
      pls.insert(place);
      isPlace = true;
    }
    else { // label/time without qm
      limitpos = contents.find_first_of(",");
      element = contents.substr(0, limitpos);
      contents.erase(0, limitpos);
      isPlace = false;
    }
    if (isTime) {
      ivs.insert(element);
    }
    else {
      if (!isPlace) {
        lbs.insert(element);
      }
      if (!lbs.empty() && !pls.empty()) {
        ok = false;
      }
    }
  }
}

/*
\subsubsection{Function ~hasRealInterval~}

*/
bool PatElem::hasRealInterval() const {
  for (set<string>::iterator is = ivs.begin(); is != ivs.end(); is++) {
    if (Tools::isInterval(*is)) {
      return true;
    }
  }
  return false;
}

/*
Constructor for class ~PatElem~

*/
PatElem::PatElem(const char *contents) : var(""), ivs(), lbs(), pls(), wc(NO), 
                                         setRel(STANDARD), ok(true) {
  string input(contents);
  vector<string> parts;
  Tools::splitPattern(input, parts);
  if (parts.size() == 2) {
    stringToSet(parts[0], true);
    stringToSet(parts[1], false);
    SecInterval iv;
    for (set<string>::iterator it = ivs.begin(); it != ivs.end(); it++) {
      if ((*it)[0] >= 65 && (*it)[0] <= 122 &&
          !Tools::checkSemanticDate(*it, iv, false)) {
        ok = false;
      }
    }
  }
  else if (parts.size() == 1) {
    if (parts[0] == "*") {
      wc = STAR;
    }
    else if (parts[0] == "+") {
      wc = PLUS;
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
  PatElem elem;
  for (unsigned int i = 0; i < wholepat->getElems().size(); i++) {
    wholepat->getElem(i, elem);
    elem.getV(var);
    if (varInput == var) {
      key = Tools::getKey(kInput);
      if ((key < 2) && ((elem.getW() != NO)
       || (wholepat->getVarPos(var).first < wholepat->getVarPos(var).second))) {
        cout << "label/place condition not allowed for sequences" << endl;
        return -1;
      }
      if ((key == 7) && ((elem.getW() == NO)
                    && (wholepat->getVarPos(var).first == wholepat->getVarPos(var).second))) {
        cout << "card condition not allowed for non-sequences" << endl;
        return -1;
      }
      condVars.insert(var);
      wholepat->addRelevantVar(var);
      if (easyCond) {
        if ((elem.getW() != NO) || (condVars.size() > 1) || !unitVars.count(var)) {
          easyCond = false;
        }
      }
      varKeys.push_back(make_pair(var, key));
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
  PatElem elem;
  for (int i = 0; i < wholepat->getSize(); i++) {
    wholepat->getElem(i, elem);
    elem.getV(right.first);
    if (varInput == right.first) {
      right.second = Tools::getKey(kInput);
      if (((right.second == 0) || (right.second == 1)) && (elem.getW() != NO)) {
        cout << "invalid assignment type " << kInput << " for a sequence" << endl;
        return false;
      }
      addRight(9, right); // assign to n \in {0, 1, ..., 8} afterwards
      wholepat->addRelevantVar(varInput);
    }
  }
  return true;
}

void Pattern::addVarPos(string var, int pos) {
  if (var.length() > 0) {
    if (varPos.count(var)) {
      varPos[var].second = pos;
    }
    else {
      varPos[var] = make_pair(pos, pos);
    }
  }
  elemToVar[numOfElems] = var;
}

void Pattern::addAtomicPos() {
  int atomNo = atomicToElem.size();
  atomicToElem[atomNo] = numOfElems;
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
  string v;
  for (int i = 0; i < (int)elems.size(); i++) {
    elems[i].getV(v);
    if (v == var) {
      return i;
    }
  }
  return -1;
}

void Condition::clear() {
  text.clear();
  varKeys.clear();
}

string ExprList::toString() {
  string result(exprs[0]);
  for (unsigned int i = 1; i < exprs.size(); i++) {
    result.append(" ");
    result.append(exprs[i]);
  }
  return result;
}
