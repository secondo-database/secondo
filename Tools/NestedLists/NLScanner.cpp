using namespace std;

#include "NLScanner.h"

NLScanner::NLScanner( NestedList* nestedList, std::istream* yyin, std::ostream* yyout )
{
  switch_streams( yyin, yyout );
  nl = nestedList;
}
