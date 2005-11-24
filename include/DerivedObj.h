/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//characters    [1]    verbatim:       [\verb@] [@]

1 DerivedObj, a class for objects created by the user command derive 

May 06, 2004. M. Spiekermann: Initial Version

May 20, 2004. M. Spiekermann: A bug during Berkeley-DB environment close has
been fixed.  Now the relation object will be closed properly in the destructor

This class creates and maintains the system table 

SEC\_DERIVED\_OBJ(name: string, value: text, usedObj: text)

which will be created in a database when a user decides to create an object via
the derive command. The derive command has the same syntax as the let command
but the created objects are not saved in a textual nested list representation
when a database is saved. When a database contains derived objects these are
recreated after all non-derived objects are restored.

This allows to restore btree objects which have no nested list representation or
to keep experimental results created from querys without storing them explicitly
in files. Since the objects are created by user interaction in the correct order
it is not necessary to do topological sorting at the dependency graph. 

If an object is deleted a warning message will list all the derived objects which 
use this object.

*/

#ifndef CLASS_DERIVEDOBJ_H
#define CLASS_DERIVEDOBJ_H

#include <SecondoSystem.h>
#include <SecondoCatalog.h>
#include <QueryProcessor.h>
#include <SecondoInterface.h>
#include <NestedList.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>

using namespace std;

class DerivedObj {

public:
   DerivedObj() : 
     tableExists(false),
     derivedObjRelName("SEC_DERIVED_OBJ"),
     derivedObjTypeStr("(rel(tuple((name string)(value text)(usedObjs text))))"),
     nl( *SecondoSystem::GetNestedList() ),
     ctlg( *SecondoSystem::GetCatalog( ExecutableLevel ) ),
     qp( *SecondoSystem::GetQueryProcessor() )
   {
     // check if the system tables are already present 
     // and read in the stored values
     if ( ctlg.IsObjectName(derivedObjRelName) ) {
     
       bool defined = false;
       bool ok = false; 
       ok = ctlg.GetObject( derivedObjRelName, derivedObjWord, defined );
       assert( ok && defined );
       derivedObjValueList = ctlg.OutObject(typeExpr(), derivedObjWord);
       DerivedObjMemoryRep( derivedObjValueList );
       tableExists = true;
     }
   
   }
   ~DerivedObj(){ // close relation object
  
     if ( tableExists ) {
        ctlg.CloseObject( typeExpr(), derivedObjWord );
     }

     // free allocated memory
     for ( vector<ObjRecord*>::const_iterator it = derivedObjRecords.begin();
           it != derivedObjRecords.end();
           it++ )
     {
       delete (*it);   
     } 
    }   


/*
1.1 Creation of relation objects

*/

   void createTableIfNecessary() {
   
     if ( !ctlg.IsObjectName(derivedObjRelName) ) { 
     
        string typeName = "";
        if ( ctlg.CreateObject( derivedObjRelName, typeName, typeExpr(), 0 ) ) {
	  tableExists = true;
	} else {
	  cerr << "Error: Creation of " << derivedObjRelName << " failed!" << endl;
	}	  
     }
   } 
      
      
   
/*

The next function stores information of the value expression of the derive
command and extracts the object dependencies from the annotated query.  

*/

   void addObj(string& objName, const ListExpr valueExpr ) {
       
      createTableIfNecessary();
      
      // extract dependent objects
      bool defined = false;	  
      ListExpr annotatedList = qp.AnnotateX( ExecutableLevel, valueExpr, defined);
      //nl.WriteListExpr(annotatedList);
      vector<ListExpr> atoms;
      nl.ExtractAtoms(annotatedList, atoms);
      				        	
      string valueExprStr = "";
      nl.WriteToString( valueExprStr, valueExpr );	  
	  
      // create new entries for derived objects
      ObjRecord* newObj = new ObjRecord(objName, valueExprStr);
      int& objId = newObj->id;
      derivedObjNames[objName] = objId - 1;
      
      for ( vector<ListExpr>::const_iterator it = atoms.begin();
            it != atoms.end();
	    it++ )
      { 
        if ( nl.AtomType(*it) == SymbolType && nl.SymbolValue(*it) == "object" ) {
          string val = nl.SymbolValue(*(it - 1));
          newObj->addDepObj( val );
	  usedObjs.insert( val );
	}  
      }
      derivedObjRecords.push_back( newObj );  
      updateTable();    
  }
  
  
  void reportObjDeps(const string& objName) {
    
    set<string>::const_iterator it = usedObjs.find(objName);
    
    if ( it == usedObjs.end() )
      return;
      
    cout << "Warning: dependent objects ";
 
    // to do: iterate over all dependent obj. 
    
    cout << "can not be restored after save database." << endl;
  }
  
   
  bool deleteObj(const string& objName, bool internal=false) {
  
    bool deleted = false;
    map<string,int>::iterator it = derivedObjNames.find(objName);
    
    if ( it != derivedObjNames.end() ) {
    
      ObjRecord& objRec = *(derivedObjRecords[it->second]);
      objRec.deleted = true;
      deleted = true; 
      derivedObjNames.erase(it);

      if (!internal) { // in the rebuild phase no messages are printed
         reportObjDeps(objName);
         updateTable();
      }
    }
    
    return deleted;
  }


  bool isDerived(const string& objName) const {
  
     map<string,int>::const_iterator it = derivedObjNames.find(objName);
     return ( it != derivedObjNames.end() );
  }
 
  // called after a database is restored.
  void rebuildObjs() {
  
    if ( !tableExists )
       return;
       
    cout << endl << "Rebuilding derived objects ..." << endl;
    for ( vector<ObjRecord*>::const_iterator it = derivedObjRecords.begin();
          it != derivedObjRecords.end();
          it++ )
    {
      assert( !(*it)->deleted );
      ListExpr valueList = nl.TheEmptyList();
      nl.ReadFromString( (*it)->value, valueList );
      
      cout << "  " << (*it)->name << " ... ";
      SecondoSystem::BeginTransaction();
      int rc = createObj( (*it)->name, valueList );
      
      if (rc != 0) {
        cout << "failed." << endl;
	const string sep = "    ";
	const string& errMsg = SecondoInterface::GetErrorMessage(rc);
	cerr << sep << "Could not rebuild object. Error msg: " << errMsg << endl;
	cerr << sep << "Maybe objects which are used for the derived object were deleted!" << endl; 
	cerr << sep << "ValueExpr: " << (*it)->value << endl << endl;
        SecondoSystem::AbortTransaction();
	deleteObj((*it)->name,true);
	
      }	else {
      
        SecondoSystem::CommitTransaction();
	cout << "created." << endl;
      }	
    }
  }


  void updateObj() {
  
   // what happens when an Secondo object is updated. Currently Nothing
   //
   // (i) derived object  => new creation expr and recreation of all dependent objects 
   // (ii) regular object => recreation of all dependent objects
  
  }
  


  int createObj(string& objName, ListExpr valueExpr) {
  
    OpTree tree;
    Word result;
    bool correct=false, evaluable=false, defined=false, isFunction=false;
    ListExpr resultType = nl.TheEmptyList();

          int rc = 0;
          if ( ctlg.IsObjectName(objName) )
          {
            rc = ERR_IDENT_USED;   // identifier is already used
          }
          else
          {
            qp.Construct( ExecutableLevel, valueExpr, correct, evaluable, defined,
                          isFunction, tree, resultType );
            if ( !defined )
            {
              rc = 8;      // Undefined object value in expression
            }
            else if ( correct )
            {
              if ( evaluable || isFunction )
              {
		  string typeName = "";
		  ctlg.CreateObject(objName, typeName, resultType, 0);
              }
              if ( evaluable )
              {
                  qp.Eval( tree, result, 1 );

                if( IsRootObject( tree ) && !IsConstantObject( tree ) )
                {
                  ctlg.CloneObject( objName, result );
                  qp.Destroy( tree, true );
                }
                else
                {
                  ctlg.UpdateObject( objName, result );
                  qp.Destroy( tree, false );
                }
              }
              else if ( isFunction )   // abstraction or function object
              {
                if ( nl.IsAtom( valueExpr ) )  // function object
                {
                   ListExpr functionList = ctlg.GetObjectValue( nl.SymbolValue( valueExpr ) );
                   ctlg.UpdateObject( objName, SetWord( functionList ) );
                }
                else
                {
                  ctlg.UpdateObject( objName, SetWord( valueExpr ) );
                }
              }
              else
              {
                rc = 3;   // Expression not evaluable
              }
            }
            else
            {
              rc = 2;    // Error in expression
            }
          }  
	  return rc;
	  
  }

  // return all derived object names
  const set<string>& getObjNames() {
  
    static set<string> nameSet;
    nameSet.clear();
    
    for ( map<string,int>::const_iterator it = derivedObjNames.begin();
          it != derivedObjNames.end();
	  it++ )
    {	  
       nameSet.insert(it->first);	  
    }
    return nameSet;
  }


private:

  // the typeExpr needs to be stored in a string since ListExpr are only
  // valid inside a call of SecondoInterface::Secondo(...)
  ListExpr& typeExpr() {
  
     nl.ReadFromString( derivedObjTypeStr, derivedObjTypeList ); 
     return derivedObjTypeList;		
  }

  // build a nested list from the memory structure
  ListExpr DerivedObjList() {

    ListExpr result = nl.TheEmptyList();
    ListExpr last = nl.TheEmptyList();

    for ( vector<ObjRecord*>::const_iterator it = derivedObjRecords.begin();
          it != derivedObjRecords.end();
          it++ )
    {
    
      if ( !(*it)->deleted) { // omit deleted objects
      
      // create list expr. for tuples
      ListExpr stringAtom = nl.StringAtom( (*it)->name );
      ListExpr textAtom = nl.TextAtom();
      ListExpr textAtom2 = nl.TextAtom();
      nl.AppendText( textAtom, (*it)->value );
      string objListStr = "";
      (*it)->depObjListStr(objListStr);
      nl.AppendText( textAtom2, objListStr );
      
      ListExpr newTuple = nl.ThreeElemList( stringAtom, nl.OneElemList(textAtom), nl.OneElemList(textAtom2) ); 
      
      if ( result == nl.TheEmptyList() ) {
      
        result = nl.Cons( newTuple, nl.TheEmptyList() );
	last = result;
      } else {
        last = nl.Append( last, newTuple );
      }
      }
    }
    return result;

  }

  // initialize a memory representation from a nested list rep.
  void DerivedObjMemoryRep(ListExpr list) {
  
    // insert the tuple values into memory structures   
    while ( list != nl.TheEmptyList() ) {
      
      ListExpr tuple = nl.First(list);      
      list = nl.Rest(list);
      string name = nl.StringValue( nl.First(tuple) );
      
      string value = "", depListStr = "";     
      nl.Text2String( nl.First( nl.Second(tuple) ), value );
      nl.Text2String( nl.First( nl.Third(tuple) ), depListStr);
      
      // convert a list stored in a textatom into a ListExpr
      ListExpr depList = nl.TheEmptyList();   
      nl.ReadFromString(depListStr, depList); 
                
      ObjRecord* newObjRec = new ObjRecord( name, value );
      while ( depList != nl.TheEmptyList() ) {
         string symbol =  nl.SymbolValue( nl.First(depList) );
         newObjRec->addDepObj( symbol );
	 usedObjs.insert( symbol );
	 depList = nl.Rest(depList);
      }
      derivedObjNames[name] = newObjRec->id - 1;
      derivedObjRecords.push_back( newObjRec );       
    }
  }

   // converts the memory representation into a nested list
   // and updates the relation object.
   void updateTable() {
   
      OpTree tree;
      Word result;
      bool correct=false, evaluable=false, defined=false, isFunction=false;
      ListExpr newValue = DerivedObjList(), resultType;
      
      qp.Construct( ExecutableLevel, nl.TwoElemList(typeExpr(), newValue), correct, evaluable, defined,
                    isFunction, tree, resultType );
		    
      assert( correct );
      assert( evaluable );
      assert( defined );
      assert( !isFunction );
      
      qp.Eval( tree, result, 1);
      ctlg.UpdateObject(derivedObjRelName, result );
      qp.Destroy( tree, false );         
   }



  // Below a container for information about derived objects
  // is declared.
  class ObjRecord {
 
  public: 
    string name;
    string value;
    set<string> depSet;
    bool deleted;
    static int id;

    ObjRecord(const string& nameStr, const string& val) : name(nameStr), value(val), deleted(false) { 
      id++; 
    }
    ~ObjRecord(){
     //cerr << "~ObjRecord() called!" << endl;
    }
    void addDepObj(string& symbol) { depSet.insert(symbol); }
    
    void depObjListStr(string& listStr) {   
      listStr = "(";
      for ( set<string>::const_iterator it = depSet.begin(); it != depSet.end(); it++ ) {
        listStr += (" " + *it + " ");
      }
      listStr += ")";
    }

  };
  
  bool tableExists;
  Word derivedObjWord;
  string derivedObjRelName;
  string derivedObjTypeStr;
  
  ListExpr derivedObjTypeList;
  ListExpr derivedObjValueList;
  
  // the map and vector below contain information 
  // about objects created with the derive command. They are
  // used to create the system table SEC_DERIVED_OBJ. 

  vector<ObjRecord*> derivedObjRecords;
  map<string,int> derivedObjNames;   
  set<string> usedObjs; 

  // references to global instances
  NestedList& nl;
  SecondoCatalog& ctlg;
  QueryProcessor& qp;
  
};

#endif

