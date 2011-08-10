
/*



Reimplementation of some functions known from the
Listutils. There are two reasons for reimplementing these
functions. First, the Listutils environment used 
the NestedList pointer nl which is available on the kernel.
Here, we use plnl available on prolog environments. 
Furthermore, in this implementation some checks are
ommitted requiring access to the algebra manager (e.g. if a type 
is in kind data). At this place these checks are not nessecary.

*/

#include "NestedList.h"
#include "SecondoInterface.h"


#include "OptSecUtils.h"

extern NestedList* plnl;       // use global si
extern SecondoInterface* si;  // use the same si as the rest of prolog


namespace optutils{


/*
Simple copying lists between environments.


*/
void copyList(const NestedList* srcEnv, const ListExpr src,
              NestedList* targetEnv, ListExpr& target){

    NodeType type = srcEnv->AtomType(src);
    switch(type){
       case IntType:      target = targetEnv->IntAtom(srcEnv->IntValue(src));
                          break;         
       case StringType:   target = targetEnv->StringAtom(
                                                   srcEnv->StringValue(src));
                          break;
       case BoolType:     target = targetEnv->BoolAtom(
                                                   srcEnv->BoolValue(src));
                          break;
       case RealType:     target = targetEnv->RealAtom(
                                                 srcEnv->RealValue(src));
                          break;
       case SymbolType:   target = targetEnv->SymbolAtom(
                                                  srcEnv->SymbolValue(src));
                          break;
       case TextType:     target = targetEnv->TextAtom( 
                                                  srcEnv->Text2String(src));
                          break;
       case  NoAtom:      if(srcEnv->IsEmpty(src)){
                             target = targetEnv->TheEmptyList();
                          } else {
                             ListExpr elem; // list element in targetEnv
                             ListExpr rest = src; // remaining list in src env

                             // convert the first element
                             copyList(srcEnv, srcEnv->First(rest), 
                                      targetEnv, elem);

                             target = targetEnv->OneElemList(elem);
                             ListExpr last = target;

                             rest = srcEnv->Rest(rest);

                             while(!srcEnv->IsEmpty(rest)){
                                copyList(srcEnv, srcEnv->First(rest), 
                                         targetEnv, elem);
                                last = targetEnv->Append(last,elem);
                                rest = srcEnv->Rest(rest);
                             }
                          }
                          break;
       default: assert(false);
    } 
}

/*
~dbOpen~

Checks whether a database is opened. If so, its name is returns 
via the name argument. If not, error Message is set to an
appropriate message.

*/
  
  bool isDatabaseOpen(string& name, string& errorMsg ){
    string command = "query getDatabaseName()";
    ListExpr result1;
    int errorCode =0;
    int errorPos = 0;
     
    si->Secondo(command,
                plnl->TheEmptyList(),
                1,                    // command level
                true,                 // command as text
                false,                // result as text
                result1,   
                errorCode,
                errorPos,
                errorMsg);

    ListExpr result;
    copyList(si->GetNestedList() ,result1, plnl, result);

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

Checks whether the list is a symbol and holds the given value.

*/

bool isSymbol(ListExpr l,const string& value){
  if(!plnl->AtomType(l)==SymbolType){
    return false;
  }
  return (plnl->SymbolValue(l) == value);
}

/*
~isSymbol~

Checks whether a list is a symbol.

*/

bool isSymbol(ListExpr l){
  if(!plnl->AtomType(l)==SymbolType){
    return false;
  }
  return true;
}


/*
Retrieves the n-th element from the list. If it is not possible
 (too less elements or list is an atom), the result is false. 
Counting starts at zero.

*/
bool getElement(const ListExpr list, const int index, ListExpr& element){

   if(plnl->IsAtom(list)){
      return false;
   }
   int i=0;
   ListExpr rest = list;
   while(i<index && !plnl->IsEmpty(rest)){
      i++;
      rest = plnl->Rest(rest);
   }
   if(!plnl->HasMinLength(rest,1)){
     return false;
   }
   element = plnl->First(rest);
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
   if(!plnl->HasMinLength(list,1)){
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

bool isRelDescription(const ListExpr list, const string reltype /*="rel" */){

  if(!plnl->HasLength(list,2)){
     return false;
  }
  ListExpr first = plnl->First(list);
  if(!isSymbol(first,reltype)){
    return false;
  }

  
  bool res =  isTupleDescription(plnl->Second(list));
  return res;

}

void getAttributeNames(const ListExpr type, set<string>& names){

  if(!isRelDescription(type)){
    return;
  }
  ListExpr attrList = plnl->Second(plnl->Second(type));
  while(!plnl->IsEmpty(attrList)){
   ListExpr first = plnl->First(attrList);
   attrList = plnl->Rest(attrList);
   names.insert(plnl->SymbolValue(plnl->First(first)));
  }

}

  void getAttributeNames(const string& name, set<string>& names){
    string n2;
    ListExpr type;
    if(!isObject(name,n2,type)){
       return;
    }
    getAttributeNames(type,names);
  }


/*
Checks whether two strings are equal. Using the boolean parameter,
the case sensitivity of the comparison can be controlled.

*/

bool strequal(const string& s1, const string& s2, const bool case_sensitive){
   if(s1.length() != s2.length()){
      return false;
   }
   if(case_sensitive){
     return s1 == s2;
   }
   for (string::const_iterator c1 = s1.begin(), c2 = s2.begin();
        (c1 != s1.end());  ++c1, ++c2) {
        if (tolower(*c1) != tolower(*c2)) {
            return false;
         }
   }
   return true;
}

/*
Searches within a type description for the attribute with  name attrName and 
returns the index of this attribute (or -1 if not found). The index counting
starts at zero. If the search is successful, the type of the attribute is 
returned in type.

Note that the attrList is not checked to be a valid attribute list. 
Check it before calling this function.

*/

int find(const string& attrName, const ListExpr attrList, 
         ListExpr& type, const bool case_sensitive = true){

  ListExpr rest = attrList;
  int i = 0;
  while(!plnl->IsEmpty(rest)){
    ListExpr current = plnl->First(rest);
    rest = plnl->Rest(rest);
    if(strequal(plnl->SymbolValue(plnl->First(current)), 
                attrName, case_sensitive)){
        type = plnl->Second(current);
        return i;
    }
    i++;
  } 
  type = plnl->TheEmptyList();
  return  -1;

}




/*
~isObject~

Checks whether in the opened database an object with the given name exists.
The name is checked case insensitive. If more than one object exists, 
the first one is selected. If a corresponding object was found, its type 
is returned via the type argument.

*/

  bool isObject(const string& name, string& realName, ListExpr& type){
      
    string command = "query getcatalog()  filter[strequal(.ObjectName, \"" + 
                      name+"\", FALSE)] head[1] tconsume";
    ListExpr result1;
    int errorCode =0;
    int errorPos = 0;
    string errorMsg; 
    si->Secondo(command,
                plnl->TheEmptyList(),
                1,                    // command level
                true,                 // command as text
                false,                // result as text
                result1,   
                errorCode,
                errorPos,
                errorMsg);

    ListExpr result;
    copyList(si->GetNestedList(),result1, plnl, result);


    if(errorCode!=0){
      cerr << "Problem in querying catalog" << endl;
      cerr << "Error: " << errorMsg;
      return false;
    } 

    // try to analyse the result list
    if(!plnl->HasLength(result,2)){
        cerr << "result list is not well formatted (# 2 elements)" << endl;
        return false;
    }

    ListExpr typelist = plnl->First(result);
    ListExpr value = plnl->Second(result);

    if(!plnl->HasLength(value,1)){ // should contain a tuple
       return false;
    }

    if(!isRelDescription(typelist,"trel")){
       cerr << "tconsume does not return a trel " << endl;
       cerr << "received: " << plnl->ToString(typelist) << endl; 
       return false;
    }
    ListExpr typeExpr_type;
    ListExpr attrList = plnl->Second(plnl->Second(typelist)); 
    int index = find("TypeExpr",attrList,typeExpr_type, true);
    if(index<0){
      cerr << "internal error, TypeExpr not found in output of getcatalog()"
           << endl;
      return false;
    }
  
    if(!isSymbol(typeExpr_type,"text")){
      cerr << "internal error, TypeExpr is not of type text" << endl;
      return false;
    } 

    // get the value for typeExpr
    ListExpr tupleValue = plnl->First(value);

    if(plnl->AtomType(tupleValue)!=NoAtom){
      cerr << "internal error, invalid representation of a tuple value" << endl;
      return false;
    }

    ListExpr typeExpr_value;
    if(!getElement(tupleValue,index,typeExpr_value)){
      cerr << "internal error, value of tuple contains too less entries" 
           << endl;
      return false;
    }

    if(plnl->AtomType(typeExpr_value)!=TextType){
      cerr << "internal error, wrong representation for a text"
           << " attribute, or undefined" 
           << endl;
      return false;
    }
    if(!plnl->ReadFromString(plnl->Text2String(typeExpr_value),type)){
       cerr << "internal error: could not parse the" 
            << " list for the type expression" 
            << endl;
       return false;
    }

    // determine the real name of the object
    ListExpr ObjectName_type;
    index = find("ObjectName",attrList, ObjectName_type, true);    
    if(index < 0){
       cerr << "internal error: could not determine index"
            << " of ObjectName from getcatalog operation" << endl;
       return false;
    }
    if(!isSymbol(ObjectName_type,"string")){
       cerr << "internal error: type of ObjectName is not string" << endl;
       return false;
    }
    ListExpr objectName;

    if(!getElement(tupleValue,index,objectName)){
       cerr << "internal error: could not retrieve object"
            << " name from tuple representation" << endl;
       return false;
    }
    if(plnl->AtomType(objectName)!=StringType){
       cerr << "internal error: invalid representation for an object name"
            << endl;
       return false;
    }
    realName = plnl->StringValue(objectName);
    return true;
  }



/*
~isValidId~

This function checks whether a string can be used as a name 
(for an attribute, an object etc.). If this name is 
not valid, the result of this function will be false. 
If the boolean parameter is set to true, the result will also
be false, if an object with this name is present in the currently
opened database. 

*/

bool isValidID(const string& id, const bool checkObject /*= false*/ ){
    string b = checkObject?"TRUE":"FALSE";
    string command = "query isValidID(\"" + id  +"\" , "+ b+")";
    ListExpr result1;
    int errorCode =0;
    int errorPos = 0;
    string errorMsg; 
    si->Secondo(command,
                plnl->TheEmptyList(),
                1,                    // command level
                true,                 // command as text
                false,                // result as text
                result1,   
                errorCode,
                errorPos,
                errorMsg);
    ListExpr result;
    copyList(si->GetNestedList(),result1, plnl, result);
    if(errorCode!=0){
      cerr << "Problem in checking for validID " << errorMsg << endl;
      return false;
    }
    if(!plnl->HasLength(result,2)){
       cerr << "isValidID returns a crazy result" << endl;
       return false;
    }
    if(!plnl->IsEqual(plnl->First(result),"bool") ){
       cerr << "isValidId does not return a boolean value" << endl;
       return false;
    }
    if(!plnl->AtomType(plnl->Second(result)==BoolType)){
       cerr << "isValidID does not return a defined boolean value" << endl;
       return  false;
    }
    return plnl->BoolValue(plnl->Second(result));
}

/*
~checkKind~

Checks wether the type is member of the kind

*/
bool checkKind(const string& type, const string& kind){
    string command = "query \"" + type  +
                     "\" kinds transformstream filter[.elem = \""+ 
                      kind+"\" ] count";
    ListExpr result1;
    int errorCode =0;
    int errorPos = 0;
    string errorMsg; 
    si->Secondo(command,
                plnl->TheEmptyList(),
                1,                    // command level
                true,                 // command as text
                false,                // result as text
                result1,   
                errorCode,
                errorPos,
                errorMsg);
    ListExpr result;
    copyList(si->GetNestedList(),result1, plnl, result);
    if(errorCode!=0){
      cerr << "Problem in checking kind " << errorMsg << endl;
      return false;
    }
    if(!plnl->HasLength(result,2)){
       cerr << "kindcheck returns a crazy result" << endl;
       return false;
    }
    if(!plnl->IsEqual(plnl->First(result),"int") ){
       cerr << "isValidId does not return an integer value" << endl;
       return false;
    }
    if(!plnl->AtomType(plnl->Second(result)==IntType)){
       cerr << "isValidID does not return a defined integer value" << endl;
       return  false;
    }
    return plnl->IntValue(plnl->Second(result))>0;
}






} // end of namespace


