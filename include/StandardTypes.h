/*
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]
 
1 Header File: Standard Data Types 

December 1998 Friedhelm Becker
 
1.1 Overview
 
This file defines four classes: CcInt, CcReal, CcBool and CcString. They
are the data types which are provided by the Standardalgebra.

*/

#ifndef STANDARDTYPES_H
#define STANDARDTYPES_H

#include <string>
#include "StandardAttribute.h"

/*
2.1 CcInt

*/


class CcInt : public StandardAttribute
{
 public:
  CcInt();
  CcInt( bool d, int v );
  ~CcInt();
  bool     IsDefined();
  void     SetDefined(bool defined);
  int      GetIntval();
  void*    GetValue();
  void     Set( int v );
  void     Set( bool d, int v );
  size_t HashValue();
  void CopyFrom(StandardAttribute* right);
  int      Compare(Attribute *arg);
  int      Adjacent(Attribute *arg);
  CcInt*   Clone() ;
  ostream& Print( ostream &os ) { return (os << intval); }

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
  CcReal();
  CcReal( bool d, float v );
  ~CcReal();
  bool     IsDefined();
  void     SetDefined(bool defined);
  float    GetRealval();
  void*    GetValue();
  void     Set( float v );
  void     Set( bool d, float v );
  size_t HashValue();
  void CopyFrom(StandardAttribute* right);
  int      Compare( Attribute* arg );
  int      Adjacent( Attribute* arg );
  CcReal*  Clone() ;
  ostream& Print( ostream &os ) { return (os << realval); }

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
  CcBool();
  CcBool( bool d, int v );
  ~CcBool();
  bool     IsDefined();
  void     SetDefined(bool defined);
  bool     GetBoolval();
  void*    GetValue();
  void     Set( bool d, bool v );
  size_t HashValue();
  void CopyFrom(StandardAttribute* right);
  int      Compare( Attribute * arg );
  int      Adjacent( Attribute * arg );
  CcBool*  Clone() ;
  ostream& Print( ostream &os ) {
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

typedef char STRING[49];

class CcString : public StandardAttribute
{
 public:
  CcString();
  CcString( bool d, const STRING* v );
  ~CcString();
  bool      IsDefined();
  void      SetDefined(bool defined);
  STRING*   GetStringval();
  void*     GetValue();
  void      Set( bool d, const STRING* v );
  size_t HashValue();
  void CopyFrom(StandardAttribute* right);
  int       Compare( Attribute* arg );
  int       Adjacent( Attribute* arg );
  CcString* Clone() ;
  ostream&  Print( ostream &os ) { return (os << "\"" << stringval << "\""); }

  static long stringsCreated;
  static long stringsDeleted;

 private:
  bool   defined;
  STRING stringval;
};

ostream& ShowStandardTypesStatistics( const bool reset, ostream& o );

#endif

