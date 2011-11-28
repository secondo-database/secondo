#include <vector>

using namespace std;

namespace stj {

struct PrePat {
  PrePat() : left(""), right(""), variable(""), cp_variable(""), wildcard(0) {}; 

  string left, right, variable;
  string cp_variable;
  char wildcard;
  
  void clear();
};


struct bracket_range {

  int start;
  int end;

};


struct SemTime {
  int type;
  int value;
};


struct PatEquation 
{
  int setValue(int key, int op, string const &value, string &errMsg);

  int key; // 1="lbs"; 3="start"; 4="end"; 6="card" 
  int op; // 1="="; 2="<"; 4=">"  

  string value;
};


struct Condition {
  Condition() : variable(""), key(0), op(0), value("") {};
  
  string variable;
  
  int key; // 1="lbs/lb"; 3="start"; 4="end"; 6="card"  
  int op; // 1="="; 2="<"; 4=">"
  string value;
  
  int setKey(string key);
  int set(string cond, int op_pos, int op, string &errMsg);
  string toString();  
};


struct SinglePattern {

  SinglePattern() : lbs(0), trs(0), dtrs(0), sts(0), variable(""), 
           cp_variable(""), wildcard(0), conditions(0), isSeq(false) {};  

  int set(PrePat const &prePat, string &errMsg); 
  string toString();
  int addCondition(Condition const &cond, string &errMsg);  
  bool isSequence();  
  
  vector<string> lbs; // labels
  vector<string> trs; // time ranges 
  vector<string> dtrs; // date ranges
  vector<SemTime> sts; // weekdays, months, datetimes 
  
  string variable;
  string cp_variable;   
  char wildcard;
  
  vector<PatEquation> conditions;  
  bool isSeq;
};


class PatParser {

public:
   PatParser(string const &text);
   PatParser() {};   
     
   string toString(); 	
   inline vector<SinglePattern> getPattern() { return pats_; }
   
   inline bool isValid() { return valid; }
   inline string getErrMsg() { return errMsg; }
   
private:
//   PatParser();
   
   vector<PrePat> prePats_;
   vector<SinglePattern> pats_;   
   vector<Condition> conds_;   
   bool valid;   
   string errMsg;
   
   int setPattern(string const &text, string &errMsg);   
   int getPrePats(string text, string &errMsg);
   void pushAndClearPrePat(PrePat &prePat);
   int getConds(string const &text, string &errMsg);
   int setPats( string &errMsg );      
   bool checkForUniqueVariables();   
};

}
