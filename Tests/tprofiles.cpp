#include <iostream>
#include <string>
#include "Profiles.h"

int main()
{
  string section;
  string keyName;
  string keyValue;
  string fileName( "test.ini" );
  cout << "Test of profile access" << endl;
  cout << "(Enter 'quit' as section name to exit program.)" << endl;
  while (section != "quit")
  {
    cout << "Section: ";
    getline( cin, section );
    cout << "KeyName: ";
    getline( cin, keyName );
    keyValue = SmiProfile::GetParameter( section, keyName, "n/a", fileName );
    cout << "KeyValue=<" << keyValue << ">" << endl;
    if (keyValue == "n/a")
      SmiProfile::SetParameter( section, keyName, keyValue, fileName );
  }
  return 0;
}
