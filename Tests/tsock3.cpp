using namespace std;
#include "SocketIO.h"
#include <time.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

int main( int argc, char* argv[] )
{
  cout << "Socket test program 3 (Rule set test)" << endl;

  SocketRuleSet ruleSet;
  if ( ruleSet.LoadFromFile( "tsock3.dat" ) )
  {
    cout << "RuleSet read from file 'tsock3.dat'." << endl;
  }
  else
  {
    ruleSet.AddRule( "132.176.69.0", "255.255.255.0" );
    cout << "Rule (132.176.69.0/255.255.255.0/ALLOW) added." << endl;
  }
  cout << "Enter IP address which should be checked: ";
  string ipAddr;
  cin >> ipAddr;
  if ( ruleSet.Ok( ipAddr ) )
  {
    cout << "Access for IP address " << ipAddr << " is allowed." << endl;
  }
  else
  {
    cout << "Access for IP address " << ipAddr << " is denied." << endl;
  }
  if ( ruleSet.StoreToFile( "tsock3.dat" ) )
  {
    cout << "RuleSet stored to file 'tsock3.dat'." << endl;
  }
  else
  {
    cout << "RuleSet could NOT be stored to file 'tsock3.dat'." << endl;
  }
  return EXIT_SUCCESS;
}

