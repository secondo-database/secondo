/*

*/

#include "SpecParser.tab.hpp"
#include "NestedText.h"

using namespace std;

int yylval;
int yylex();

int main()
{
  int token;
  yylval = 0;
    
  token = yylex();
  while (token != 0)
  {
    cout << token << " "; 
    if (yylval != 0)
    {
      cout << yylval;
      yylval = 0;
    }
    cout << endl;
    token = yylex();
  }
  return (0);
}

