#include <iostream>
#include <string>

void text2list( string& in, string& out, string& err );

extern int xxdebug;

int main()
{
  char intext[1000];
  xxdebug = 0;
//  string in = "query Staedte feed filter[.Bev > 100000] consume";
  string in = "query Staedte select[.pop > 100000].";
  string out = "";
  string err = "";
  cout << "?> " << in << endl;
  while( cout << "?> " << flush, cin.getline(intext, 1000))
  {
    in = intext; out = ""; err = "";
    text2list( in, out, err );
    cout << "=> " << out << "\n";
  }
  return (0);
}

