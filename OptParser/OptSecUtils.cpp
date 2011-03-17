
/*



*/

#include "NestedList.h"
#include "SecondoInterface.h"

extern NestedList* nl;       // use global si
extern SecondoInterface* si;  // use the same si as the rest of prolog


namespace optutils{

/*
~dbOpen~

Checks whether a database is opened. If so, its name is returns 
via the name argument.

*/
  
  bool isDatabaseOpen(string& name){
    return true;
  }



/*
~isObject~

Checks whether in the opened database an object with the given name exists.
The name is checked case insensitive. If more than one object exists, the first one
is selected. If a corresponding object was found, its type is returned via
the type argument.

*/

  bool isObject(const string& name, string& realName, ListExpr& type){
     return true;
  }


/*
~isRelationDescription~

*/
  
  bool isRelationDescription(const ListExpr type){
     if(!nl->HasLength(type,2)){
       return false;
     }
     ListExpr first = nl->First(type);
     if(nl->AtomType(first) != SymbolType){
        return false;
     }
     string maintype = nl->SymbolValue(first);
     return (maintype=="rel") || (maintype == "orel") || (maintype =="trel");

  }





} // end of namespace


