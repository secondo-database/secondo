/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen,
Faculty of Mathematics and Computer Science,
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

//[secondo] [\textsc{Secondo}]

*/

/*
1 Generic Type Constructors


This file provides some template functions for an easy creation
of [secondo] type constructors.
This method only works for simple attribute types, i.e. non-composite
attributes. But this case covers the most secondo type constructors.

To use the generic approach, the C++ class representing the corrsponding
[secondo] type must provide some funtions described in the following:


1.1 Requirements

1.1.1 ~Attribute~

First of all, the class must be derived from the class __Attribute__ and
implement all functions required by this class.


1.1.2 ~ToListExpr~

----

  ListExpr ToListExpr(const typeInfo ListExpr) const
----
This function converts the calling instance into a nested list.
In the most cases, the argument ~typeInfo~ can be ignored.



1.1.3 ~ReadFrom~

----

  bool ReadFrom(const ListExpr instance, const ListExpr typeInfo)
----
This function sets the value of the calling instance to this one described
by the ~instance~ argument. If the list is not a valid representation for
this class, the result is __false__ otherwise the result is __true__.
The second argument can be ignored in the most cases.


1.1.4 ~BasicType~

----

  static string BasicType()
----

This static function just returns the name for the [secondo] type.


1.1.5 ~Property~

----

  static ListExpr Property()
----

This static function returns the nested list describing the
corresponding [secondo] type. For a simple implementation of this function,
you can use the __GenProperty__ function in the follwing way:

----

   gentc::GenProperty(signature, example_type_list, list_rep, example_list, remarks)
----

e.g.

----

    gentc::GenProperty("-> DATA", "int", "<intvalue>", "-21", "");
----

1.1.6 ~CheckKind~

----

  static bool CheckKind(ListExpr type, ListExpr& errorInfo)
----

This function checks whether the nested list ~type~ represents [secondo]'s type
for this class.


1.2 Creating the Secondo Type Constructor


After these preparations, a [secondo] type constructor is just created by:


----

  GenTC<ClassName> name;
----


*/

#ifndef GENERIC_TC
#define GENERIC_TC

#include "Attribute.h"
#include "NestedList.h"
#include "../Tools/Flob/Flob.h"
#include "Algebra.h"

extern NestedList* nl;



namespace gentc{
/*
1.1 ListConversions

1.1.1 Out function

*/
template<class T>
ListExpr Out(ListExpr typeInfo, Word value){
  return  (static_cast<T*>(value.addr))->ToListExpr(typeInfo);
}

/*
1.1.2 In Function

*/
template<class T>
Word In(const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct ){
   T* t = new T(0);
   if(t->ReadFrom(instance,typeInfo)){
      correct = true;
      return SetWord(t);
   } else {
      correct = false;
      delete t;
      return SetWord(Address(0));
   }
}

/*
1.2  ~Create~ Function

The following function is used to create instances of the
template argument.

*/
template<class T>
Word Create(const ListExpr typeInfo){
  return SetWord(new T(1));
}

/*
1.3 ~Delete~ Functions

The ~delete~ functions can be used to destroy the
instances of types of this algebra.

*/
template<class T>
void Delete(const ListExpr typeInfo,Word &w){
  T* B = static_cast<T*>(w.addr);
  int nof = B->NumOfFLOBs();
  for(int i=0;i< nof; i++){
    (B->GetFLOB(i))->destroy();
  }
  delete B;
  B = NULL;
  w.addr=0;
}


/*
1.4 ~Close~-Functions

The close functions destroy the instances.

*/
template<class T>
void Close(const ListExpr typeInfo, Word& w ){
  delete static_cast<T*>(w.addr);
  w.addr = 0;
}

/*
1.5 ~Clone~-Functions

The clone functions can be used for get a copy
of an existing object.

*/
template<class T>
Word Clone(const ListExpr typeInfo, const Word& w){
   return SetWord( (static_cast<T*>(w.addr))->Clone());
}


/*
1.6 ~SizeOf~-Functions

This functions must be provided for allocating the
correct memory space for an object in query processing.

*/
template<class T>
int SizeOf(){
 return sizeof(T);
}

/*
1.7 ~Cast~-Functions

Some functions proving dynamic casts.

*/
template<class T>
void* Cast(void* addr){
  return new (addr) T;
}


inline ListExpr GenProperty(std::string signature,
                     std::string example_type_list,
                     std::string list_rep,
                     std::string example_list,
                     std::string remarks = ""){
   return ::nl->TwoElemList(
             ::nl->FiveElemList(
                 ::nl->StringAtom("Signature"),
                 ::nl->StringAtom("Example Type List"),
                 ::nl->StringAtom("List Rep"),
                 ::nl->StringAtom("Example List"),
                 ::nl->StringAtom("Remarks")),
             ::nl->FiveElemList(
                ::nl->TextAtom(signature),
                ::nl->TextAtom(example_type_list),
                ::nl->TextAtom(list_rep),
                ::nl->TextAtom(example_list),
                ::nl->TextAtom(remarks))
         );
}


} // end of namespace gentc

/*
5. TypeConstructors

In this section, the type constructors for using in the
algebra are defined.

*/

template<class T>
class GenTC : public TypeConstructor{
 public:
   GenTC()
      :TypeConstructor(T::BasicType(),
                       T::Property,
                       gentc::Out<T >,
                       gentc::In<T >,
                       0,0,
                       gentc::Create<T>,
                       gentc::Delete<T>,
                       OpenAttribute<T>,
                       SaveAttribute<T>,
                       gentc::Close<T>,
                       gentc::Clone<T>,
                       gentc::Cast<T>,
                       gentc::SizeOf<T>,
                       T::CheckKind){}
};


#endif
