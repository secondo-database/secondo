using namespace std;

#include "NestedList.h"
#include "NLParser.h"
#include "NLScanner.h"

NLParser::NLParser( NestedList* nestedList, istream* ip, ostream* op )
  : isp( ip ), osp( op ), nl( nestedList )
{
  lex = new NLScanner( nestedList, isp, osp );
}

NLParser::~NLParser()
{
  delete lex;
}

int
NLParser::yylex()
{
  return (lex->yylex());
}

void
NLParser::yyerror( char* s )
{
  cerr << s << " processing '" << lex->YYText() << "'!" << endl;
}

/*
std::istream& Parser::operator>>(double& val) {
    lex->yyrestart(isp);
    flexLexer = lex;
    yyparse();
    val = parse_value;
    return *isp;
}
*/
