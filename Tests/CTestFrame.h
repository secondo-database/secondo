/*

Jan 2005, M. Spiekermann. The class ~CTestFrame~ provides some useful facilities
which should be used when writing little test programs. Just use it as base class
for your test cases.


1 ~CBool~ a wrapper for the built in type ~bool~ 

*/

#include <string>
#include <iostream>

using namespace std;


class CBool { // a wrapper class useful for pretty printing bool values

public:
 bool value;
 CBool(bool b) { value=b; }
 ~CBool(){}

};

ostream& operator<<(ostream& os, CBool b) { // output format in streams

  if (b.value == true)
    os << "TRUE";
  else
    os << "FALSE";

  return os;

}

/*

2 Class ~CTestFrame~

*/

class CTestFrame {

public:

  CTestFrame(char x = '*') : caseCtr(0), errCtr(0), fillChar(x) {}
  ~CTestFrame() {}


  bool BeginCheck(const string& str) 
  {
    cout << "-- Testing:" << str << endl;
    
    return false;
  }

  void EndCheck(bool result) 
  {
    cout << "-- Result: ";
    if ( result ) {
      cout << "OK!" << endl;
    } else {
      cout << "ERROR!" << endl;
      errCtr++;
    } 
  }

  void ShowErrors() 
  {
    cout << endl << "SUMMARY:" 
	 << endl << "--------"
	 << endl << "  There were "; 

    if (errCtr) {
      cout << errCtr;
    } else {
      cout << "no";
    }
    cout << " errors." << endl;

  }


  void TestCase(const string& title) {

    const string sep(60,fillChar);
    stringstream caseNr;
    caseNr << ++caseCtr << ". ";

    const string white(60-4-(title.length()+caseNr.str().length()), ' ');
    cout << endl
	 << sep << endl
	 << fillChar << "  " << caseNr.str() << title << white << fillChar << endl
	 << sep << endl; 
  }

/*

2.1 The method below can validate an expected result of an operation.

*/

  bool CheckResult(const string& operation, bool result, bool expected) {

    cout << operation << " = " << CBool(result);
    bool ok = (result == expected);
    if (ok)
    { 
      cout << " as expected.";
    
    } else {
      cout << " error! " << CBool(expected) << "is expected.";
      errCtr++;
    }
    cout << endl;
    return ok;
  }


  void pause()
  {
     char buf[80];
     cout << endl << "<<< continue? [y,n]: >>>" << endl;
     cin.getline( buf, sizeof(buf) );
     if ( buf[0] != 'y' && buf[0] != 'Y' ) {
     exit(0);
     } 
  }

  int GetNumOfErrors() { return errCtr; }

private:

  unsigned int caseCtr;
  int errCtr;
  const char fillChar;

};

