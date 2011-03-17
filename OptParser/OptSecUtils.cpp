
/*



*/

#include "NestedList.h"
#include "SecondoInterface.h"

extern NestedList* plnl;       // use global si
extern SecondoInterface* si;  // use the same si as the rest of prolog


namespace optutils{

/*
~dbOpen~

Checks whether a database is opened. If so, its name is returns 
via the name argument.

*/
  
  bool isDatabaseOpen(string& name, string& errorMsg ){
    string command = "query getDatabaseName()";
    ListExpr result;
    int errorCode =0;
    int errorPos = 0;
     
    si->Secondo(command,
                plnl->TheEmptyList(),
                1,                    // command level
                true,                 // command as text
                false,                // result as text
                result,   
                errorCode,
                errorPos,
                errorMsg);

    if(errorCode!=0){
      return false;
    } else {
      errorMsg = "";
      if(!plnl->HasLength(result,2)){
          errorMsg = "invalid result list for dbname";
          return false;
      }   

      ListExpr type = plnl->First(result);
      if(plnl->AtomType(type) != SymbolType ||
        plnl->SymbolValue(type) != "string"){
        errorMsg = "invalid return type for db name";
        return false;
      }

      ListExpr value = plnl->Second(result);

      if(plnl->AtomType(value)!=StringType){
        errorMsg = "invalid value for database name ";
        return false;
      }

      errorMsg = "";
      name = plnl->StringValue(value);
      return true;
    }
    
  }


/*
~isSymbol~

*/

bool isSymbol(ListExpr l,const string& value){
  if(!plnl->AtomType(l)==SymbolType){
    return false;
  }
  return (plnl->SymbolValue(l) == value);
}


bool isSymbol(ListExpr l){
  if(!plnl->AtomType(l)==SymbolType){
    return false;
  }
  return true;
}



/*
~isAttr~

*/

bool isAttr(ListExpr attr){
   if(!plnl->HasLength(attr,2)){
     return  false;
   }
   if(!isSymbol(plnl->First(attr))){
     // not an attr name
     return false;
   }
   // here, we cannot check the DATA property of a type, 
   // hence, we have to assume that all the things are ok.
   return true;
}


/*
~IsAttrList~

*/

bool isAttrList(ListExpr list){
   if(plnl->HasMinLength(list,1)){
     return false;
   }
   while(!plnl->IsEmpty(list)){
      if(!isAttr(plnl->First(list))){
         return false;
      }
      list = plnl->Rest(list);
   }
   return  true;
}



/*
~isTupleDescription~

*/

bool isTupleDescription(ListExpr list){
  if(!plnl->HasLength(list,2)){
     return false;
  }
  ListExpr first = plnl->First(list);
  if(!isSymbol(first,"tuple")){
    return false;
  }
  return isAttrList(plnl->Second(list));
}


/*
~isRelDescription~

Checks whether the list is a description of a relation with given relation type;

*/

bool isRelDescription(ListExpr list, string reltype = "rel"){

  if(!plnl->HasLength(list,2)){
     return false;
  }
  ListExpr first = plnl->First(list);
  if(!isSymbol(first,reltype)){
    return false;
  }

  return isTupleDescription(plnl->Second(list));

}




/*
~isObject~

Checks whether in the opened database an object with the given name exists.
The name is checked case insensitive. If more than one object exists, the first one
is selected. If a corresponding object was found, its type is returned via
the type argument.

*/

  bool isObject(const string& name, string& realName, ListExpr& type){
      
    string command = "query getcatalog()  filter[strequal(.ObjectName, \"" + 
                      name+"\", FALSE)] head[1] tconsume";
    ListExpr result;
    int errorCode =0;
    int errorPos = 0;
    string errorMsg; 
    si->Secondo(command,
                plnl->TheEmptyList(),
                1,                    // command level
                true,                 // command as text
                false,                // result as text
                result,   
                errorCode,
                errorPos,
                errorMsg);

    if(errorCode!=0){
      cerr << "Problem in sending command" << endl;
      cerr << "Error: " << errorMsg;
      return false;
    } 
    // try to analyse the result list
    if(!plnl->HasLength(result,2)){
        cerr << "result list is not well formatted" << endl;
        return false;
    }
    ListExpr typelist = plnl->First(result);
    ListExpr value = plnl->Second(result);

    if(!plnl->HasLength(value,1)){ // should contain a tuple
       return false;
    }

    if(!isRelDescription(typelist,"trel")){
       cerr << "tconsume does not return a trel " << endl;
       return false;
    }




    
    return false;  
         
 
  }







} // end of namespace


