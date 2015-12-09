/*

Jan 2005, M. Spiekermann. The class ~CTestFrame~ provides some useful facilities
which should be used when writing little test programs. Just use it as base 
class for your test cases.


1 ~CBool~ a wrapper for the built in type ~bool~ 

*/

#include <string>
#include <iostream>


#define CHECK(a,b) CheckResult(#a, a, b)



class CBool { // a wrapper class useful for pretty printing bool values

public:
 bool value;
 CBool(bool b) { value=b; }
 ~CBool(){}

};

std::ostream& operator<<(std::ostream& os, CBool b) { 
// output format in streams


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


  bool BeginCheck(const std::string& str) 
  {
    std::cout << "-- Testing:" << str << std::endl;
    
    return false;
  }

  void EndCheck(bool result) 
  {
    std::cout << "-- Result: ";
    if ( result ) {
      std::cout << "OK!" << std::endl;
    } else {
      std::cout << "ERROR!" << std::endl;
      errCtr++;
    } 
  }

  void ShowErrors() 
  {
    std::cout << std::endl << "SUMMARY:" 
	 << std::endl << "--------"
	 << std::endl << "  There were "; 

    if (errCtr) {
      std::cout << errCtr;
    } else {
      std::cout << "no";
    }
    std::cout << " errors." << std::endl;

  }


  void TestCase(const std::string& title) {

    const std::string sep(60,fillChar);
    std::stringstream caseNr;
    caseNr << ++caseCtr << ". ";

    const std::string white(60-4-(title.length()+caseNr.str().length()), ' ');
    std::cout << std::endl
	 << sep << std::endl
	 << fillChar << "  " << caseNr.str() 
         << title << white << fillChar << std::endl
	 << sep << std::endl; 
  }

/*

2.1 The method below can validate an expected result of an operation.

*/

  bool CheckResult(const std::string& operation, bool result, bool expected) {

    std::cout << "*** " << operation << "    [" << CBool(result);
    bool ok = (result == expected);
    if (ok)
    { 
      std::cout << " -> OK ]";
    
    } else {
      std::cout << " -> ERROR ] " 
           << CBool(expected) << " expected!" << std::endl;
      errCtr++;
    }
    std::cout << std::endl;
    return ok;
  }


  void pause()
  {
     char buf[80];
     std::cout << std::endl << "<<< continue? [y,n]: >>>" << std::endl;
     std::cin.getline( buf, sizeof(buf) );
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

