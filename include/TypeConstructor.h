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

April 2006, M. Spiekermann. The file Algebra.h need to be divided into Operators.h. TypeConstructors.h, 
AlgebraClassDef.h and AlgebraInit.h   

*/

#ifndef SEC_TYPECONSTRUCTOR_H
#define SEC_TYPECONSTRUCTOR_H

#include <string>
#include <vector>

#include "NestedList.h"
#include "AlgebraTypes.h"
#include "ConstructorTemplates.h"

// a forward declaration
class AlgebraManager;

using namespace std;

/*
1 Class ~TypeConstructor~

An instance of class ~TypeConstructor~ consists of

  * a name

  * a function (``outFunc'') converting a value's Address-representation to the
    corresponding nested list representation

  * a function (``inFunc'') converting a value's nested list representation to
    the corresponding Address-represantation

  * a function (``createFunc'') allocating main memory for values which can't be
    represented by a single Address (may be valid for all types in the future)

  * a function (``deleteFunc'') releasing the memory previously allocated by
    createFunc.

  * a function (``openFunc'').

  * a function (``saveFunc'').

  * a function (``closeFunc'').

  * a function (``cloneFunc'').

All properties of an instance of class TypeConstructor are set within the
constructor.

*/

struct ConstructorInfo {

  string name;
  string signature;
  string typeExample;
  string listRep;
  string valueExample;
  string remarks;

  ConstructorInfo() :
    name(""),  
    signature(""),  
    typeExample(""),
    listRep(""),
    valueExample(""),
    remarks("")
  {}

  ConstructorInfo( const string& _name,
                   const string& _signature, 
                   const string& _typeExample, 
                   const string& _listRep, 
                   const string& _valueExample,
                   const string& _remarks       )
  {
    name = _name;
    signature = _signature;
    typeExample = _typeExample;
    listRep = _listRep;
    valueExample = _valueExample;
    remarks = _remarks;
  }  

    
  const ListExpr list() const { 
   
     ListExpr headList = 
               nl->FiveElemList( nl->StringAtom("Signature"),
                                 nl->StringAtom("Example Type List"),
                                 nl->StringAtom("List Rep"),
                                 nl->StringAtom("Example List"),
                                 nl->StringAtom("Remarks") );
     ListExpr specList = 
               nl->FiveElemList( nl->StringAtom(signature),
                                 nl->StringAtom(typeExample),
                                 nl->StringAtom(listRep),
                                 nl->StringAtom(valueExample),
                                 nl->StringAtom(remarks) );

    return nl->TwoElemList(headList, specList ); 
  }
};



class TypeConstructor
{
 public:
  TypeConstructor( const string& nm,
                   TypeProperty prop,
                   OutObject out,
                   InObject in,
                   OutObject saveToList,
                   InObject restoreFromList,
                   ObjectCreation create,
                   ObjectDeletion del,
                   ObjectOpen open,
                   ObjectSave save,
                   ObjectClose close,
                   ObjectClone clone,
                   ObjectCast ca,
                   ObjectSizeof sizeOf,
                   TypeCheckFunction tcf );

  template<class T>
  TypeConstructor( const ConstructorInfo ci,
                   ConstructorFunctions<T> cf  )
  {  
    conInfo              = ci;
    name                 = ci.name;
    propFunc             = 0;
    
    outFunc              = cf.out;
    inFunc               = cf.in;
    saveToListFunc       = cf.saveToList;
    restoreFromListFunc  = cf.restoreFromList;
    createFunc           = cf.create;
    deleteFunc           = cf.deletion;
    openFunc             = cf.open;
    saveFunc             = cf.save;
    closeFunc            = cf.close;
    cloneFunc            = cf.clone;
    castFunc             = cf.cast;
    sizeofFunc           = cf.sizeOf;
    typeCheckFunc        = cf.typeCheck;
  }

  
/*
Constructs a type constructor.

*/
  virtual ~TypeConstructor();
/*
Destroys an instance of a type constructor.

*/
  void AssociateKind( const string& kindName );
/*
Associates the kind ~kindName~ with this type constructor.

*/
  ListExpr Property();
  static ListExpr Property(const ConstructorInfo& ci);
/*
Returns the properties of the type constructor as a nested list.

*/
  ListExpr Out( ListExpr type, Word value );
  Word     In( const ListExpr typeInfo,
               const ListExpr value,
               const int errorPos,
               ListExpr& errorInfo,
               bool& correct );
  ListExpr SaveToList( ListExpr type, Word value );
  Word     RestoreFromList( const ListExpr typeInfo,
                            const ListExpr value,
                            const int errorPos,
                            ListExpr& errorInfo,
                            bool& correct );
  Word     Create( const ListExpr typeInfo );
  void     Delete( const ListExpr typeInfo,
                   Word& w );
  bool     Open( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value );
  bool     Save( SmiRecord& valueRecord, 
                 size_t& offset, 
                 const ListExpr typeInfo,
                 Word& value );
  void     Close( const ListExpr typeInfo,
                  Word& w );
  Word     Clone( const ListExpr typeInfo,
                  const Word& w );
  int      SizeOf();
  string&  Name() { return name; }

/*
Are methods to manipulate objects according to the type constructor.

*/
  bool     DefaultOpen( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& value );
  bool     DefaultSave( SmiRecord& valueRecord,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& value );
/*
Default methods for ~Open~ and ~Save~ functions if these are not provided.
These methods use the ~RestoreFromList~ and ~SaveToList~ if provided, and
~In~ and ~Out~ otherwise. 

*/
  static Word DummyCreate( const ListExpr typeInfo );
  static void DummyDelete( const ListExpr typeInfo,
                           Word& w );
  static void DummyClose( const ListExpr typeInfo,
                          Word& w );
  static Word DummyClone( const ListExpr typeInfo,
                          const Word& w );
  static int  DummySizeOf();
  
  bool TypeCheck( ListExpr type, ListExpr& errorInfo )
  {
    if (typeCheckFunc)
      return (*typeCheckFunc)( type, errorInfo);
    else 
      return SimpleCheck( type, errorInfo );
  }


/*
Dummy methods used as placeholders for type constructor functions.

*/
 private:
 
  inline bool SimpleCheck( ListExpr type, ListExpr& errorInfo )
  {
    return (nl->IsEqual( type, name ));
  }

  ConstructorInfo          conInfo;
  string                   name;   // Name of type constr.
  TypeProperty             propFunc;
  OutObject                outFunc;
  InObject                 inFunc;
  OutObject                saveToListFunc;
  InObject                 restoreFromListFunc;
  ObjectCreation           createFunc;
  ObjectDeletion           deleteFunc;
  ObjectOpen               openFunc;
  ObjectSave               saveFunc;
  ObjectClose              closeFunc;
  ObjectClone              cloneFunc;
  ObjectCast               castFunc;
  ObjectSizeof             sizeofFunc;
  TypeCheckFunction        typeCheckFunc;
  vector<string>           kinds;  // Kinds of type constr.

  friend class AlgebraManager;
};


#endif
