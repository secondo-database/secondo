/*
This are some type definitions for the Optimizer syntax parser 



*/



enum OpType{INFIX, PREFIX, POSTFIX,POSTFIXBRACKETS, UNKNOWN};


struct OpPatternType{
  OpPatternType():optype(UNKNOWN), no_args(-1), 
                  isSpecial(false), isImplicit(false){}  
              
   OpType optype;
   int no_args; // number of leading args for postfix
   bool isSpecial;
   bool isImplicit;
  };
