/*

*/

#include "NestedList.h"
#include "SymbolicTrajectoryTools.h"
#include <string>
#include <set>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

using namespace std;

//class MLabel;
union Word;

namespace stj {

class Pattern;

enum Key {LABEL, TIME, START, END, CARD, ERROR};

bool parseString(const char* argument, Pattern** p);

class MLabel : public MString {
  public:
    static const string BasicType() { return "mlabel"; }
    static ListExpr MLabelProperty();
    static bool CheckMLabel( ListExpr type, ListExpr& errorInfo );
};

class ULabel : public UString {
  public:
    static const string BasicType() { return "ulabel"; }
    static ListExpr ULabelProperty();
    static bool CheckULabel( ListExpr type, ListExpr& errorInfo );
};

enum Wildcard {NO, ASTERISK, PLUS};

class ExpressionList {
 public: 
  vector<string>* expressions;

  ExpressionList() {
    expressions = new vector<string>;
  }

  ~ExpressionList() {}

  string toString();
};

class ConditionType {
 public:
  string type;
  string replacement; // e.g., we replace X.card by 1 to check whether the
          // condition is executable by Secondo and yields a boolean value
  ConditionType(const char *t, const char *r) {
    type.assign(t);
    replacement.assign(r);
  }
};

class Condition {
 public:
  string condition;
  string condsubst; // the condition after substituting as mentioned above
  vector<ConditionType>* types;
  vector<Key> keys;
  vector<string> variables;

  Condition() {
    types = new vector<ConditionType>;
    types->push_back(*(new ConditionType(convert(".label"), convert("\"a\""))));
    types->push_back(*(new ConditionType(convert(".time"), convert(
           "[const periods value ((\"2003-11-20-07:01:40\" "
           "\"2003-11-20-07:45\" TRUE TRUE))]"))));
    types->push_back(*(new ConditionType(convert(".start"), convert(
           "[const instant value \"1909-12-19\"]"))));
    types->push_back(*(new ConditionType(convert(".end"), convert(
           "[const instant value \"2012-05-12\"]"))));
    types->push_back(*(new ConditionType(convert(".card"), convert("1"))));
    types->push_back(*(new ConditionType(convert(".ERROR"), convert(""))));
  }

  ~Condition() {}
  
  void toString();
  Key convertVarKey(const char *varKey);
  void clear();
  void substitute();
};

class UnitPattern {
 public:
  string variable;
  string interval;
  vector<string> labelset;
  string wildcard;

  UnitPattern() {
    labelset = *(new vector<string>);
  }

  ~UnitPattern() {}

  UnitPattern(const string v, const string i, const string l, const string w) {
    variable = v;
    interval = i;
    labelset = splitLabel(l);
    wildcard = w;
  }

  void setUnit(const char *v, const char *i, const char *l, const char *w);
  void getUnit(const char *var, bool assignment);
  void createUnit(const char *var, const char *pat);
};

class Pattern {
 public:
  vector<UnitPattern>* patterns;
  vector<UnitPattern>* results;
  vector<UnitPattern>* assignments;
  vector<Condition>* conditions;

  Pattern() {
    patterns = new vector<UnitPattern>;
    results = new vector<UnitPattern>;
    assignments = new vector<UnitPattern>;
    conditions = new vector<Condition>;
  }

  Pattern(vector<UnitPattern> *pats) {
    patterns = pats;
  }

  Pattern(vector<UnitPattern> *pats, vector<Condition> *conds) {
    patterns = pats;
    conditions = conds;
  }

  Pattern(const Pattern& rhs) {
    patterns = rhs.patterns;
    results = rhs.results;
    assignments = rhs.assignments;
    conditions = rhs.conditions;
  }

  ~Pattern() {
    delete patterns;
    delete results;
    delete assignments;
    delete conditions;
  }

  void toString();
  string GetText() const;
   // algebra support functions
  static Word     In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word     Create(const ListExpr typeInfo);
  static void     Delete(const ListExpr typeInfo, Word& w);
  static void     Close(const ListExpr typeInfo, Word& w);
  static Word     Clone(const ListExpr typeInfo, const Word& w);
  static bool     KindCheck(ListExpr type, ListExpr& errorInfo);
  static int      SizeOfObj();
  static ListExpr Property();
  // other functions
  static const string BasicType();
  static const bool checkType(const ListExpr type);
  int checkConditions();
  bool getPattern(string input, Pattern** p);
  bool matches(MLabel const &ml);
};

class NFA {
 private:
  set<int> **transitions; // 1st coord: old state; 2nd coord: unit pattern id;
                          // contents: new state.
  set<int> currentStates;

 public:
  NFA(const int size) {
    transitions = new set<int>*[size];
    for (int i = 0; i < size; i++) {
      transitions[i] = new set<int>[size];
    }
  }

  void buildNFA(Pattern p);
};

}
