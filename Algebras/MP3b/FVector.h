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

*/





#ifndef _FVector_h_
#define _FVector_h_

#include "NestedList.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "Tools/Flob/DbArray.h"
#include "Tools/Flob/Flob.h"
#include "GenericTC.h"

using namespace std;


class FVector : public Attribute{
public:
    // constructors
    FVector();
  FVector(double dummy );

  // copy constructor
  FVector (const FVector &c);

  // assignment operator
  FVector &operator =( const FVector &src );

  // desctructor
  ~ FVector ();

  // auxiliary functions
  static const string BasicType(){ return "fvector" ; }

  static const bool checkType(const ListExpr list) {
    return listutils::isSymbol(list , BasicType ());
  }

  void append(CcReal *i);

  void append(double i);

  int getDim() const;

  double getElem(int index) const;


  // NumOfFLOBs
  // this class has one FLOB in form of a DbArray
  inline virtual int NumOfFLOBs() const {
    return 1;
  }
  // return the flob if index is correct
  inline virtual Flob* GetFLOB( const int i ) {
    assert (i ==0);
    return &content;
  }


  // compare
  int Compare( const Attribute *arg ) const;

  // there is no meaningful Adjacent implementation
  bool Adjacent(const Attribute *arg ) const;

  // standard implementation of Sizeof
  size_t Sizeof() const;

  // hash function
  size_t HashValue() const;

  //copyfrom
  void CopyFrom( const Attribute *arg );

  //clone
  Attribute* Clone() const;

  // Additionall functions
  // static ListExpr Property ();

  static ListExpr Property (){
    return gentc::GenProperty ( " -> DATA " , // signature
      BasicType () ,
      // type description
      " ( real real ...) " , // list rep
      " (1.0 2.0 3.0) " ); // example list
  }

  // Type check function
  // static bool CheckKind( ListExpr type , ListExpr & errorInfo );

  static bool CheckKind( ListExpr type , ListExpr & errorInfo ){
      return checkType( type );
  }



  // replacement for the IN function
  bool ReadFrom( ListExpr LE , const ListExpr typeInfo );

  // replacement for the out function
  ListExpr ToListExpr(ListExpr typeInfo ) const;

private :
  DbArray <double> content ;
};


#endif
