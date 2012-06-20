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
int findPos(const char *text, const char letter);
bool parseSuccess = true;
void patternerror( const char* s ) {
  cerr << endl << s << endl << endl;
  parseSuccess = false;
}
stj::Pattern* wholepat = 0;
Condition cond;
UnitPattern unitpat;
ExpressionList exprList;
bool doubleParentheses = false;
string expr;
set<string> currentIntervals;
unsigned int currentPos = 0;
bool firstAssignment = true;
%}

%union{
  char* text;
  class UnitPattern* up;
  class Pattern* p;
  class ExpressionList* el;
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
          expressionlistbrackets  expressionlistenclosed
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
               string var, labels;
               labels.assign($3);
               var.assign($1);
               var.assign(var.substr(0, var.find('.')));
               unitpat.getUnit(convert(var), true);
               unitpat.labelset = stringToSet(labels);
               unitpat.wildcard.clear();
               wholepat->assignments.push_back(unitpat);
               cout << "unit added to assignments" << endl;
             }
           ;

resultsequence : result
               | resultsequence result
               ;

result : ZZVARIABLE unitpattern_result {
           unitpat.getUnit($1, false);
           if (!unitpat.variable.empty()) {
             unitpat.createUnit($1, $2);
             unitpat.intervalset = currentIntervals;
             wholepat->results.push_back(unitpat);
             cout << "unit " << $2 << " added to results" << endl;}
           else {
             cout << $1 << " was not found in the pattern" << endl;
           }
         }
       | ZZVARIABLE {
           unitpat.getUnit($1, false);
           if (!unitpat.variable.empty()) {
             wholepat->results.push_back(unitpat);
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
              cond.condition.assign(exprList.toString());
              cout << "store condition " << cond.condition << endl;
              cond.substitute();
              wholepat->conditions.push_back(cond);
              exprList.expressions.clear();
              cond.variables.clear();
              cond.keys.clear();
              cond.patternIds.clear();
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
               list.append(exprList.expressions.back());
               $$ = convert(list);
               exprList.expressions.erase(exprList.expressions.end());
               cout << "expressionlistparentheses reads " << $$ << endl;
             }
           | expressionlistbrackets {
               string list;
               list.append(exprList.expressions.back());
               $$ = convert(list);
               exprList.expressions.erase(exprList.expressions.end());
               cout << "expressionlistbrackets reads " << $$ << endl;
             }
           ;

expressionlistparentheses : '(' enclosedlist ')' {
                              int exprSize = exprList.expressions.size();
                              exprList.expressions[exprSize - 1].insert(0, "(");
                              exprList.expressions[exprSize - 1].append(")");
                              $$ = &exprList;
                            }
                          ;

expressionlistbrackets : '[' enclosedlist ']' {
                           int exprSize = exprList.expressions.size();
                           exprList.expressions[exprSize - 1].insert(0, "[");
                           exprList.expressions[exprSize - 1].append("]");
                           $$ = &exprList;
                         }
                       ;

enclosedlist : expression expressionlistenclosed {
                 expr.assign($1);
                 int exprSize = exprList.expressions.size();
                 exprList.expressions[exprSize - 1].insert(0, " ");
                 exprList.expressions[exprSize - 1].insert(0, expr);
                 free($1);
               }
             | expression expressionlistcomma {
                 expr.assign($1);
                 int exprSize = exprList.expressions.size();
                 exprList.expressions[exprSize - 1].insert(0, " ");
                 exprList.expressions[exprSize - 1].insert(0, expr);
                 free($1);
              }
             | expression {
                 expr.assign($1);
                 exprList.expressions.push_back(expr);
                free($1);
               }
             | /* empty */ {
                 expr.clear();
               }
             ;

expressionlistenclosed : expression {
                           expr = $1;
                           //expr.assign($1);
                           cout << "one element list = " << $1 << endl;
                           free($1);
                           exprList.expressions.push_back(expr);
                           $$ = &exprList;
                         }
                       | expressionlistenclosed expression {
                           expr.assign($2);
                           int exprSize = exprList.expressions.size();
                           exprList.expressions[exprSize - 1].append(" ");
                           exprList.expressions[exprSize - 1].append(expr);
                           free($2);
                           $$ = &exprList;
                           cout << "expressionlistenclosed reads \"" << exprList.toString() << "\"" << endl;
                         }
                       ;

expressionlistcomma : ',' expression {
                        expr.assign($2);
                        expr.insert(0, ",");
                        cout << "one elem list = " << expr << endl;
                        exprList.expressions.push_back(expr);
                        free($2);
                        $$ = &exprList;
                      }
                    | expressionlistcomma ',' expression {
                        expr.assign($3);
                        int exprSize = exprList.expressions.size();
                        exprList.expressions[exprSize - 1].append(",");
                        exprList.expressions[exprSize - 1].append(expr);
                        free($3);
                        $$ = &exprList;
                        cout << "comma list = " << exprList.toString() << endl;
                      }
                    ;

expressionlist : expression {
                   expr.assign($1);
                   cout << "one element list = " << $1 << endl;
                   exprList.expressions.push_back(expr);
                   free($1);
                   $$ = &exprList;
                 }
               | expressionlist expression {
                   expr.assign($2);
                   int exprSize = exprList.expressions.size();
                   exprList.expressions[exprSize - 1].append(" ");
                   exprList.expressions[exprSize - 1].append(expr);
                   free($2);
                   $$ = &exprList;
                   cout << "condition reads \"" << exprList.toString() << "\"" << endl;
                 }
               ;

patternsequence : variable unitpattern {
                    $$ = wholepat;
                    unitpat.createUnit($1, $2);
                    wholepat->patterns.push_back(unitpat);
                   free($1);
                   free($2);
                    cout << "pattern #" << wholepat->patterns.size() << endl;
                  }
                | patternsequence variable unitpattern {
                    $$ = wholepat;
                    unitpat.createUnit($2, $3);
                    wholepat->patterns.push_back(unitpat);
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
                doubleParentheses = true;
              }
            | '(' ')' {
                $$ = convert("");
              }
            | '(' '(' ')' ')' {
                $$ = convert("");
                doubleParentheses = true;
              }
            | ZZWILDCARD
            ;

%%
/*
function ~parseString~
This function is the only one called by the algebra.

*/
Pattern* stj::parseString(const char* argument) {
  wholepat = new Pattern();
  pattern_scan_string(argument);
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
    result->text.assign(argument);
    wholepat = 0;
  }
  unitpat.labelset.clear();
  return result;
}

/*
function ~getUnit~
Searches var in the pattern and verifies the correct order of variables in the
result pattern. In case of success, the unit pattern gets the suitable values.

*/
void UnitPattern::getUnit(const char *var, bool assignment) {
  if (assignment && firstAssignment) {
    currentPos = 0; // reset counter for first assignment
    firstAssignment = false;
  }
  string varStr;
  varStr.assign(var);
  bool found = false;
  while ((currentPos < wholepat->patterns.size()) && !found) { // look for the variable
    if (!(wholepat->patterns)[currentPos].variable.compare(varStr)) {
      variable.assign((wholepat->patterns)[currentPos].variable);
      intervalset = (wholepat->patterns)[currentPos].intervalset;
      labelset = (wholepat->patterns)[currentPos].labelset;
      wildcard.assign((wholepat->patterns)[currentPos].wildcard);
      found = true;
      currentIntervals = intervalset;
      cout << "variable " << variable << " found in pattern " << currentPos << endl;
    }
    currentPos++;
  }
  if (!found) {
    variable.clear();
    cout << "variable " << varStr << " not found" << endl;
  }
}

void UnitPattern::setUnit(const char *v, const char *i,
                          const char *l, const char *w) {
  string lstr, istr;
  lstr.assign(l);
  istr.assign(i);
  variable.assign(v);
  intervalset = stringToSet(istr);
  labelset = stringToSet(lstr);
  wildcard.assign(w);
}

/*
function ~createUnit~
Modifies the unit pattern according to the parameters.
*/
void UnitPattern::createUnit(const char *var, const char *pat) {
  string patstr, varstr, token;
  if (strlen(pat) > 0) {
    patstr.assign(pat);
  }
  if (strlen(var) > 0) {
    varstr.assign(var);
  }
  int pos = 0;
  while ((pos = varstr.find(' ', pos)) != string::npos) {
    varstr.erase(pos, 1);
  }
  /*pos = patstr.find('{');
  while ((pos = patstr.find(' ', pos)) != string::npos) {
    patstr.erase(pos, 1);
  }*/
  if (strcmp(var, "") && strcmp(var, "*") && strcmp(var, "+")
   && (varstr.at(0) > 64) && (varstr.at(0) < 91)) {
    variable.assign(varstr);
  }
  else {
    variable.clear();
  }
  vector<string> pattern = splitPattern(patstr);
  if (doubleParentheses) {
    wildcard.assign("+");
  }
  if (pattern.size() == 2) {
    if (!pattern[0].compare("_")) { // (_ a)
      intervalset.clear();
    }
    else {
      intervalset = stringToSet(pattern[0]);
    }
    if (!pattern[1].compare("_")) {
      labelset.clear();
    }
    else {
      labelset = stringToSet(pattern[1]);
    }
    if (!doubleParentheses) {
      wildcard.clear();
    }
  }
  else if ((pattern.size() == 1) && (!pattern[0].compare("*")
                                  || !pattern[0].compare("+"))) {
    wildcard.assign(pattern[0]);
    intervalset.clear();
    labelset.clear();
  }
  else if ((pattern.size() == 1) && pattern[0].empty()) {
    wildcard.clear();
  }
  else if ((pattern.size() == 0) || ((pattern.size() == 1) &&
     (!pattern[0].compare("*") || !pattern[0].compare("+")))) {
    intervalset.clear();
    labelset.clear();
  }
  if ((pattern.size() == 0) && !doubleParentheses) {
    wildcard.clear();
  }
  doubleParentheses = false;
}

int findPos(const char *text, const char letter) {
  string textstr;
  textstr.assign(text);
  return textstr.find(letter);
}

/*
function ~substitute~
variable.type is substituted by a value of the same type in order to be able
to test-execute the condition as a query.

*/
void Condition::substitute() {
  string varKey;
  unsigned int i = 0;
  condsubst.assign(condition);
  while (i < keys.size()) {
    varKey.assign(variables[i]);
    varKey.append(types[keys[i]]);
    int pos = condsubst.find(varKey);
    condsubst.replace(pos, varKey.size(), subst[keys[i]]);
    i++;
  }
}

/*
function ~convertVarKey~
Checks whether the variable var occurs in the pattern and whether the key k
is valid; returns the recognized key.

*/
Key Condition::convertVarKey(const char *varKey) {
  string input, varInput, kInput, var;
  Key key;
  input.assign(varKey);
  int dotpos = input.find('.');
  varInput.assign(input.substr(0, dotpos));
  kInput.assign(input.substr(dotpos + 1));
  for (unsigned int i = 0; i < wholepat->patterns.size(); i++) {
    if (!varInput.compare(((wholepat->patterns)[i]).variable)) {
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
      cond.variables.push_back(var);
      cond.keys.push_back(key);
      cond.patternIds.push_back(i);
      cout << varInput << " | pat #" << i << " | cond #" << wholepat->conditions.size() << endl;
      (wholepat->patterns)[i].relatedConditions.insert(wholepat->conditions.size());
      return key;
    }
  }
  cout << "variable " << var << " does not exist in the pattern" << endl;
  return ERROR;
}

void Condition::clear() {
  condition.clear();
  condsubst.clear();
  keys.clear();
  variables.clear();
  patternIds.clear();
}

string ExpressionList::toString() {
  string result;
  result.assign(expressions[0]);
  for (unsigned int i = 1; i < expressions.size(); i++) {
    result.append(" ");
    result.append(expressions[i]);
  }
  return result;
}
