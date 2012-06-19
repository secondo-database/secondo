/*

*/

#include "NestedList.h"
#include "SymbolicTrajectoryTools.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include <string>
#include <set>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

using namespace std;

union Word;

namespace stj {

class Pattern;

enum Key {LABEL, TIME, START, END, CARD, ERROR};
enum Wildcard {NO, ASTERISK, PLUS};

bool parseString(const char* argument, Pattern** p);
bool evaluate(string conditionString, const bool resultNeeded);

class MLabel : public MString {
  public:
    static const string BasicType() {
      return "mlabel";
    }
    static ListExpr MLabelProperty();
    static bool CheckMLabel(ListExpr type, ListExpr& errorInfo);
};

class ULabel : public UString {
  public:
    static const string BasicType() {
      return "ulabel";
    }
    static ListExpr ULabelProperty();
    static bool CheckULabel(ListExpr type, ListExpr& errorInfo);
};

class ExpressionList {
 public: 
  vector<string> expressions;

  ExpressionList() {}

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
  vector<ConditionType> types;
  vector<Key> keys;
  vector<string> variables;
  vector<int> patternIds;

  Condition() {
    types.push_back(*(new ConditionType(convert(".label"), convert("\"a\""))));
    types.push_back(*(new ConditionType(convert(".time"), convert(
           "[const periods value ((\"2003-11-20-07:01:40\" "
           "\"2003-11-20-07:45\" TRUE TRUE))]"))));
    types.push_back(*(new ConditionType(convert(".start"), convert(
           "[const instant value \"1909-12-19\"]"))));
    types.push_back(*(new ConditionType(convert(".end"), convert(
           "[const instant value \"2012-05-12\"]"))));
    types.push_back(*(new ConditionType(convert(".card"), convert("1"))));
    types.push_back(*(new ConditionType(convert(".ERROR"), convert(""))));
  }

  ~Condition() {
    types.clear();
  }
  
  void toString();
  Key convertVarKey(const char *varKey);
  void clear();
  void substitute();
};

class UnitPattern {
 public:
  string variable;
  set<string> intervalset;
  set<string> labelset;
  string wildcard;
  set<int> relatedConditions;

  UnitPattern() {}

  ~UnitPattern() {}

  UnitPattern(const string v, const string i, const string l, const string w) {
    variable = v;
    intervalset = stringToSet(i);
    labelset = stringToSet(l);
    wildcard = w;
  }

  void setUnit(const char *v, const char *i, const char *l, const char *w);
  void getUnit(const char *var, bool assignment);
  void createUnit(const char *var, const char *pat);
};

class Pattern {
 public:
  vector<UnitPattern> patterns;
  vector<UnitPattern> results;
  vector<UnitPattern> assignments;
  vector<Condition> conditions;

  Pattern() {}

  Pattern(const Pattern& rhs) {
    patterns = rhs.patterns;
    results = rhs.results;
    assignments = rhs.assignments;
    conditions = rhs.conditions;
  }

  Pattern& operator=(const Pattern& rhs){
    patterns = rhs.patterns;
    results = rhs.results;
    assignments = rhs.assignments;
    conditions = rhs.conditions;
    return (*this);
  }  

  ~Pattern() {}

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
  void verifyConditions();
  bool getPattern(string input, Pattern** p);
  bool matches(MLabel const &ml);
};

class NFA {
 private:
  set<int> **transitions; // 1st coord: old state; 2nd coord: unit pattern id;
                          // contents: new state(s).
  set<int> currentStates;
  vector<UnitPattern> nfaPatterns;
  vector<Condition> nfaConditions;
  int numOfStates;
  ULabel curULabel;
  size_t curULabelId;
  size_t maxLabelId;
  set<size_t> *matchings;
  set<size_t> *cardsets;
  set<vector<size_t> > sequences; // all possible matching sequences
  set<vector<size_t> > condMatchings; // for condition evaluation

 public:
  NFA(const int size) {
    numOfStates = size;
    transitions = new set<int>*[numOfStates];
    for (int i = 0; i < numOfStates; i++) {
      transitions[i] = new set<int>[numOfStates];
    }
    currentStates.insert(0);
    matchings = new set<size_t>[numOfStates - 1];
    cardsets = new set<size_t>[numOfStates - 1];
  }

  ~NFA() {
    for (int i = 0; i < numOfStates; i++) {
      delete[] transitions[i];
    }
    delete[] transitions;
    delete[] matchings;
    delete[] cardsets;
  }

  void buildNFA(Pattern p);
  bool match(MLabel const &ml);
  void printCurrentStates();
  void printCards();
  void printSequences(size_t max);
  void printCondMatchings(size_t max);
  void updateStates();
  void storeMatch(int state);
  bool labelsMatch(int pos);
  bool timesMatch(int pos);
  void buildSequences();
  bool conditionsMatch(MLabel const &ml);
  void buildCondMatchings(unsigned int condId, vector<size_t> sequence);
  bool evaluateCond(MLabel const &ml, unsigned int condId,
                    vector<size_t> sequence);
  string getNextSubst(MLabel const &ml, Key key, unsigned int pos);
  string toString();
};

}
