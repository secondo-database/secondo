/*
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]
 
1 Header File for  Standard Data Types 
 
Friedhelm Becker , Dec 98
 
\tableofcontents
 
1 Overview
 
This file defines four classes: CcInt, CcReal, CcBool and CcString. They
are the data types which are provided by the Standardalgebra.

*/

#ifndef STANDARDTYPES_H
#define STANDARDTYPES_H

#include <string>
#include "StandardAttribute.h"

/*
2 CcInt

*/


class CcInt : public StandardAttribute	
{
 public:
  CcInt();
  CcInt( bool d, int v );
  ~CcInt();
  bool     IsDefined();
  int      GetIntval();
  void*    GetValue();
  void     Set( int v );
  void     Set( bool d, int v );
  int      Compare(Attribute * arg);
  int      Sizeof() ;
  CcInt*   Clone() ;
  ostream& Print( ostream &os ) { return (os << intval); }
 private:
  bool defined;
  int  intval;
};

/*
3 CcReal

*/

class CcReal : public StandardAttribute	
{
 public:
  CcReal();
  CcReal( bool d, float v );
  ~CcReal();
  bool     IsDefined();
  float    GetRealval();
  void*    GetValue();
  void     Set( float v );
  void     Set( bool d, float v );
  int      Compare( Attribute* arg );
  int      Sizeof() ;
  CcReal*  Clone() ;
  ostream& Print( ostream &os ) { return (os << realval); }
 private:
  bool  defined;
  float realval;
};

/*
4 CcBool

*/

class CcBool : public StandardAttribute	
{
 public:
  CcBool();
  CcBool( bool d, int v );
  ~CcBool();
  bool     IsDefined();
  bool     GetBoolval();
  void*    GetValue();
  void     Set( bool d, bool v );
  int      Compare( Attribute * arg );
  int      Sizeof()  ;
  CcBool*  Clone() ;
  ostream& Print( ostream &os ) { return (os << boolval); }
 private:
  bool defined;
  bool boolval;
};

/*
5 CcString

*/

class CcString : public StandardAttribute
{
 public:
  CcString();
  CcString( bool d, const string& v );
  ~CcString();
  bool      IsDefined();
  string*   GetStringval();
  void*     GetValue();
  void      Set( bool d, const string& v );
  int       Compare( Attribute* arg );
  int       Sizeof()  ;
  CcString* Clone() ;
  ostream&  Print( ostream &os ) { return (os << stringval); }
 private:
  bool   defined;
  string stringval;
};

#endif

