using namespace std;

#include "NLScanner.h"

NLScanner::NLScanner( NestedList* nestedList, istream* yyin, ostream* yyout )
{
  switch_streams( yyin, yyout );
  nl = nestedList;
}
