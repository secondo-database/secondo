
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
  
  bool isDatabaseOpen(string& name, string& errorMsg);



/*
~isObject~

Checks whether in the opened database an object with the given name exists.
The name is checked case insensitive. If more than one object exists, the first one
is selected. If a corresponding object was found, its type is returned via
the type argument.

*/

  bool isObject(const string& name, string& realName, ListExpr& type);


/*
~isRelationDescription~

*/
  
  bool isRelDescription(const ListExpr type, const string& reltyp = "rel");


/*
~getAttributeNames~

Inserts all attributes names of a relation description to a set of strings.

*/

  void getAttributeNames(const ListExpr type, set<string>& names);

  void getAttributeNames(const string& name, set<string>& names);


/*
~isValidId~

This function checks whether a string can be used as a name 
(for an attribute, an object etc.). If this name is 
not valid, the result of this function will be false. 
If the boolean parameter is set to true, the result will also
be false, if an object with this name is present in the currently
opened database. 

*/

bool isValidID(const string& id, const bool checkObject = false);



/*
~checkKind~

Checks wether the type is member of the kind

*/
bool checkKind(const string& type, const string& kind);





} // end of namespace


