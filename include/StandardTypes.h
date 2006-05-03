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

//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

1 Header File: Standard Data Types

December 1998 Friedhelm Becker

2002-2003 U. Telle. Diploma thesis "reimplementation of SECONDO"

Nov. 2004. M. Spiekermann. Modifications in CcInt. Using inline directives
and avoiding to dereference pointers in the ~Compare~ method improves performance.

April 2006. M. Spiekermann. A new struct StdTypes was added. It offers static methods for retrieving integer or string arguments from a given Word value. Moreover, counters for calls of ~Compare~ and ~HashValue~ are implemented. 

1.1 Overview

This file defines four classes: CcInt, CcReal, CcBool and CcString. They
are the data types which are provided by the Standardalgebra.

*/

#ifndef STANDARDTYPES_H
#define STANDARDTYPES_H

#include <string>
#include "StandardAttribute.h"
#include "NestedList.h"
#include "Counter.h"

/*
2.1 CcInt

*/

class CcInt : public StandardAttribute
{
 public:
  
  inline CcInt()
  {
    intsCreated++; 
  }
 
  inline CcInt( bool d, int v )
  { 
    defined = d; intval = v;  
    intsCreated++; 
  }
 
  inline ~CcInt()
  {
    intsDeleted++;
  }

  inline void Initialize() 
  {
  }

  inline void Finalize() 
  {
    intsDeleted++;
  }

   
  inline bool IsDefined() const 
  { 
    return (defined); 
  }
  
  inline void SetDefined(bool defined) 
  { 
    this->defined = defined;
  }
    
  inline int GetIntval() const
  { 
    return (intval); 
  }
  
  inline void Set( int v )
  { 
    defined = true, intval = v; 
  }
  
  inline void Set( bool d, int v )
  { 
    defined = d, intval = v; 
  }
  
  inline size_t HashValue() const
  { 
    static long& ctr = Counter::getRef("CcInt::HashValue");
    ctr++;
    return (defined ? intval : 0); 
  }
  
  inline void CopyFrom(const StandardAttribute* right)
  {
    const CcInt* r = (const CcInt*)right;
    defined = r->defined;
    intval = r->intval;
  }
  
  inline int Compare(const Attribute *arg) const
  {

    static long& ctr = Counter::getRef("CcInt::Compare");
    ctr++;
    if(!defined) 
      return -1;
    const CcInt* p = (const CcInt*)arg;
    if(!defined && !p->defined) 
      return 0;
    if(!p->defined) 
      return 1;
    if( !p )
      return -2;
    if ( intval < p->intval ) 
      return -1;
    if ( intval > p->intval) 
      return 1;
    return 0;
  }

  inline bool Adjacent(const Attribute *arg) const
  {
    int a = GetIntval(),
        b = ((const CcInt *)arg)->GetIntval();

    return( a == b || a == b + 1 || b == a + 1 );
  }
  
  inline CcInt* Clone() const
  { 
    return (new CcInt( this->defined, this->intval )); 
  }
  
  inline ostream& Print( ostream &os ) const { return (os << intval); }

  ListExpr CopyToList( ListExpr typeInfo )
  {
      cout << "CcInt CopyToList" << endl;
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId = nl->IntValue( nl->First( nl->First( typeInfo ) ) ),
          typeId = nl->IntValue( nl->Second( nl->First( typeInfo ) ) );

      return (algMgr->OutObj(algId, typeId))( typeInfo, SetWord(this) );
  }

  Word CreateFromList( const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
  {
      cout << "CcInt CreateFromList" << endl;
      NestedList *nl = SecondoSystem::GetNestedList();
      AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
      int algId = nl->IntValue( nl->First( nl->First( typeInfo ) ) ),
          typeId = nl->IntValue( nl->Second( nl->First( typeInfo ) ) );

      Word result = (algMgr->InObj(algId, typeId))( typeInfo, 
                         instance, errorPos, errorInfo, correct );
      if( correct )
        return result;
      return SetWord( Address(0) );
  }

  static long intsCreated;
  static long intsDeleted;

 private:
  bool defined;
  int  intval;
};

/*
3.1 CcReal

*/

class CcReal : public StandardAttribute
{
 public:
  inline CcReal()
  { 
     realsCreated++; 
  }

  inline CcReal( bool d, float v ) 
  { 
    defined = d; 
    realval = v; 
    realsCreated++; 
  }

  inline ~CcReal() 
  { 
    realsDeleted++; 
  }

  inline void Initialize() 
  {}

  inline void Finalize() 
  {
    realsDeleted++; 
  }

  inline bool IsDefined() const 
  { 
    return defined; 
  }

  inline void SetDefined(bool defined) 
  { 
    this->defined = defined; 
  }

  inline float GetRealval() const
  { 
    return realval;
  }
 
  inline void CcReal::Set( float v ) 
  { 
    defined = true, 
    realval = v; 
  }

  inline void Set( bool d, float v ) 
  { 
    defined = d;
    realval = v; 
  }

  inline CcReal* Clone() const
  { 
    return (new CcReal(this->defined, this->realval)); 
  }

  inline size_t HashValue() const
  {
    if(!defined)
      return 0;

    unsigned long h = 0;
    char* s = (char*)&realval;
    for(unsigned int i = 1; i <= sizeof(float) / sizeof(char); i++)
    {
      h = 5 * h + *s;
      s++;
    }
    return size_t(h);
  }

  inline void CopyFrom(const StandardAttribute* right)
  {
    const CcReal* r = (const CcReal*)right;
    defined = r->defined;
    realval = r->realval;
  }

  inline int Compare( const Attribute * arg ) const
  {
    if(!defined)
      return -1;
    const CcReal *p = (const CcReal*)arg;
    if(!defined && !p->defined)
      return 0;
    if(!p->defined)
      return 1;
    if ( !p ) 
      return -2;
    if ( realval < p->realval ) 
      return -1;
    if ( realval > p->realval ) 
      return 1;
    return 0;
  }

  inline bool Adjacent( const Attribute *arg ) const
  {
    return( realval == ((const CcReal *)arg)->realval );
  }

  inline ostream& Print( ostream &os ) const { return (os << realval); }

  static long realsCreated;
  static long realsDeleted;

 private:
  bool  defined;
  float realval;
};

/*
4.1 CcBool

*/

class CcBool : public StandardAttribute
{
 public:
  inline CcBool()
  { 
    boolsCreated++; 
  }

  inline CcBool( bool d, int v )
  { 
    defined  = d; 
    boolval = v; 
    boolsCreated++; 
  }

  inline ~CcBool() 
  { 
    boolsDeleted++; 
  }
 
  inline void Initialize() 
  {}

  inline void Finalize() 
  { 
    boolsDeleted++; 
  }

  inline void Set( bool d, bool v )
  { 
    defined = d;
    boolval = v; 
  }

  inline bool IsDefined() const 
  { 
    return defined; 
  }

  inline void SetDefined(bool defined) 
  { 
    this->defined = defined; 
  }

  inline bool GetBoolval() const
  { 
    return boolval; 
  }

  inline CcBool* Clone() const
  { 
    return new CcBool(this->defined, this->boolval); 
  }

  inline size_t HashValue() const
  { 
    return (defined ? boolval : false); 
  }

  inline void CopyFrom(const StandardAttribute* right)
  {
    const CcBool* r = (const CcBool*)right;
    defined = r->defined;
    boolval = r->boolval;
  }

  inline int Compare( const Attribute* arg ) const
  {
    if(!defined)
      return -1;
    const CcBool *p = (const CcBool*)arg;
    if(!defined && !p->defined)
      return 0;
    if(!p->defined)
      return 1;
    if ( !p )
      return -2;
    if ( boolval < p->boolval ) 
      return -1;
    if ( boolval > p->boolval ) 
      return 1;
    return 0;
  }

  inline bool Adjacent( const Attribute* arg ) const
  {
    return 1;
  }

  inline ostream& Print( ostream &os ) const
  {
    if (boolval == true) return (os << "TRUE");
    else return (os << "FALSE");
  }

  static long boolsCreated;
  static long boolsDeleted;

 private:
  bool defined;
  bool boolval;
};

/*
5.1 CcString

*/

typedef char STRING[MAX_STRINGSIZE+1];

class CcString : public StandardAttribute
{
 public:
  inline CcString() 
  { 
    stringsCreated++; 
  }

  inline CcString( bool d, const STRING* v ) 
  { 
    defined = d; 
    strcpy( stringval, *v); 
    stringsCreated++; 
  }

  inline CcString::~CcString() 
  { 
    stringsDeleted++; 
  }

  inline void Initialize() 
  {} 

  inline void Finalize() 
  { 
    stringsDeleted++; 
  }

  inline bool IsDefined() const 
  { 
    return defined; 
  }

  inline void SetDefined(bool defined) 
  { 
    this->defined = defined; 
  }

  inline const STRING* GetStringval() const
  { 
    return &stringval; 
  }

  inline CcString* Clone() const
  { 
    return (new CcString( this->defined, &this->stringval )); 
  }

  inline void Set( bool d, const STRING* v ) 
  { 
    defined = d; 
    strcpy( stringval, *v); 
  }

  inline size_t HashValue() const
  {
    static long& ctr = Counter::getRef("CcString::HashValue");
    ctr++;
    if(!defined)
      return 0;

    unsigned long h = 0;
    const char* s = stringval;
    while(*s != 0)
    {
      h = 5 * h + *s;
      s++;
    }
    return size_t(h);
  }

  inline void CopyFrom(const StandardAttribute* right)
  {
    const CcString* r = (const CcString*)right;
    defined = r->defined;
    strcpy(stringval, r->stringval);
  }

  inline int Compare( const Attribute* arg ) const
  {
    static long& ctr = Counter::getRef("CcString::Compare");
    ctr++;
    if(!defined)
      return -1;
    const CcString* p = (const CcString*)(arg);
    if(!defined && !p->defined)
      return 0;
    if(!p->defined)
      return 1;
    if ( !p ) 
      return -2;
    if ( strcmp(stringval , p->stringval) < 0) 
      return -1;
    if ( !strcmp(stringval , p->stringval)) 
      return 0;
    return 1;
  }

  bool Adjacent( const Attribute* arg ) const;

  inline ostream& Print( ostream &os ) const { 
    return (os << "\"" << stringval << "\""); 
  }

  static long stringsCreated;
  static long stringsDeleted;

 private:
  bool   defined;
  STRING stringval;
};

void ShowStandardTypesStatistics( const bool reset );

/*
6 Some Functions Prototypes

*/
Word InCcBool( ListExpr typeInfo, ListExpr value, 
               int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutCcBool( ListExpr typeinfo, Word value );
Word InCcInt( ListExpr typeInfo, ListExpr value, 
              int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutCcInt( ListExpr typeinfo, Word value );
Word InCcReal( ListExpr typeInfo, ListExpr value, 
               int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutCcReal( ListExpr typeinfo, Word value );
Word InCcString( ListExpr typeInfo, ListExpr value,
                 int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutCcString( ListExpr typeinfo, Word value );

/*
Functions which convert a ~Word~ value (argument of an operator)
into the corresponding C++ type
   
*/

struct StdTypes
{
  static int GetInt(const Word& w); 
  static int RequestInt(const Word& w); 

  static string GetString(const Word& w); 
  static string RequestString(const Word& w); 

  private:
  static string GetString(Word w, const bool doRequest); 
  static int GetInt(Word w, const bool doRequest); 
};

#endif

