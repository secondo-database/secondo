using namespace std;

#include "SecondoInterface.h"

NestedList*
SecondoInterface::GetNestedList()
{
  return (nl);
}

/*
3.2 Error Messages

For a description of error handling see the definition module. 

Procedure ~InitErrorMessages~ should be copied after any changes into the definition module.

*/

bool SecondoInterface::errMsgInitialized = false;
map<int,string> SecondoInterface::errors;

void
SecondoInterface::InitErrorMessages()
{
  errors[1] = "Command not recognized. ";
  errors[2] = "Error in (query) expression. ";
  errors[3] = "Expression not evaluable. (Operator not recognized or stream?)";
  errors[4] = "Error in type expression. No object created. ";
  errors[5] = "Error in type expression. No type defined. ";
  errors[6] = "No database open. ";
  errors[7] = "A database is open. ";
  errors[8] = "Undefined object value in (query) expression";
  errors[9] = "Syntax error in command/expression";

  errors[10] = "Identifier already used. ";
  errors[11] = "Identifier is not a known type name. ";
  errors[12] = "Identifier is not a known object name. ";
  errors[13] = "Type of expression is different from type of object. ";
  errors[14] = "Type name is used by an object. Type not deleted. ";

  errors[20] = "Transaction already active. ";
  errors[21] = "No transaction active. ";
  errors[22] = "Begin transaction failed. ";
  errors[23] = "Commit/Abort transaction failed. ";

  errors[24] = "Error in type or object definitions in file. ";
  errors[25] = "Identifier is not a known database name. ";
  errors[26] = "Problem in writing to file. ";
  errors[27] = "Database name in file different from identifier. ";
  errors[28] = "Problem in reading from file. ";
  errors[29] = "Error in the list structure in the file. ";

  errors[30] = "Command not yet implemented. ";
  errors[31] = "Command level not yet implemented. ";

  errors[40] = "Error in type definition. ";                // (40 i)
  errors[41] = "Type name doubly defined. ";                // (41 i n)
  errors[42] = "Error in type expression. ";                // (42 i n)

  errors[50] = "Error in object definition. ";              // (50 i)
  errors[51] = "Object name doubly defined. ";              // (51 i n)
  errors[52] = "Wrong type expression for object. ";        // (52 i n)
  errors[53] = "Wrong list representation for object. ";    // (53 i n)

  errors[60] = "Kind does not match type expression. ";     // (60 k t)
  errors[61] = "Specific kind checking error for kind. ";   // (61 k j ...)

  errors[70] = "Value list is not a representation for type constructor. ";
                                                            // (70 tc v)
  errors[71] = "Specific error for type constructor in value list. ";
                                                            // (71 tc j ...)
  errors[72] = "Value list is not a representation for type constructor. ";
                                                            // (72 tc)
  errors[73] = "Error at a position within value list for type constructor. ";
                                                            // (73 pos)
}

/*
1.4 Procedure ~GetErrorMessage~

*/
string
SecondoInterface::GetErrorMessage( const int errorCode )
{
  if ( !errMsgInitialized )
  {
    InitErrorMessages();
    errMsgInitialized = true;
  }
  map<int,string>::iterator errPos = errors.find( errorCode );
  if ( errPos != errors.end() )
  {
    return (errPos->second);
  }
  return ("Unknown error code. ");
}

