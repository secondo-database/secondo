/*

*/

using namespace std;

int yyparse();

int main()
{
  int error = yyparse();
  return (error);
}

