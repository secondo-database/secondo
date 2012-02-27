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

Nov. 2004. M. Spiekermann. Modifications in ~CcInt~. Using inline directives
and avoiding to dereference pointers in the ~Compare~ method improves performance.

April 2006. M. Spiekermann. A new struct StdTypes was added. It offers static
methods for retrieving integer or string arguments from a given Word value.
Moreover, counters for calls of ~Compare~ and ~HashValue~ are implemented for types
~CcInt~ and ~CcString~.

May 2006. M. Spiekermann. The implementation of ~Compare~ for ~CcInt~ has been
changed.  Now first the case that both values are IsDefined is handled and
equality is tested before unequality. This makes the number of integer
comparisons for recognizing $A > B$ or $A < B$ symmetric and in the average the
same since we need 1 for validating $A = B$ and 2 for $A {<,>} B$. Before it was
1 for $A > B$ and 2 for $A {<,=} B$.


1.1 Overview

This file defines four classes which represent the data types provided
by the ~StandardAlgebra~:

----
     C++       |   SECONDO
     ======================
     CcInt     |   int
     CcReal    |   real
     CcBool    |   bool
     CcString  |   string
----

*/

#ifndef STANDARDTYPES_H
#define STANDARDTYPES_H



#include <string>
#include <sstream>

#undef TRACE_ON
#include "Trace.h"

#include "Attribute.h"
#include "NestedList.h"
#include "Counter.h"
#include "Symbols.h"
#include "Serialize.h"
#include "ListUtils.h"
#include "StringUtils.h"


/*
~Auxiliary Functions~

~trim~

This function removes whitespaces from the start and the end of ~value~.

*/
 void trimstring(string& str);





/*
2.1 CcInt

*/



class CcInt : public Attribute
{
 public:

  inline CcInt():Attribute()
  {
    intsCreated++;
  }

  explicit inline CcInt( bool d, int v = 0 ) : Attribute(d),intval(v)
  {
    SetDefined(d);
    intsCreated++;
  }

  explicit inline CcInt( int v, bool d=true ) : Attribute(d),intval(v)
  {
    SetDefined(d);
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

  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline int GetIntval() const
  {
    return (intval);
  }

  inline int GetValue() const
  {
    return (intval);
  }

  inline void Set( int v )
  {
    SetDefined(true);
    intval = v;
  }

  inline void Set( bool d, int v )
  {
    SetDefined(d); intval = v;
  }

  inline size_t HashValue() const
  {
    static long& ctr = Counter::getRef(Symbol::CTR_INT_HASH());
    ctr++;
    return (IsDefined() ? intval : 0);
  }

  inline void CopyFrom(const Attribute* right)
  {
    const CcInt* r = (const CcInt*)right;
    SetDefined(r->IsDefined());
    intval = r->intval;
  }

  inline int Compare(const Attribute* arg) const
  {
    const CcInt* rhs = dynamic_cast<const CcInt*>( arg );
    static long& ctr = Counter::getRef(Symbol::CTR_INT_COMPARE());
    ctr++;

    return Attribute::GenericCompare<CcInt>( this, rhs,
                             IsDefined(), rhs->IsDefined() );
  }

  inline int Compare(const CcInt arg) const
  {
    const Attribute* rhs = (Attribute*) &arg;
    return Compare(rhs);
  }

  inline virtual bool Equal(const CcInt* rhs) const
  {
    static long& ctr = Counter::getRef(Symbol::CTR_INT_EQUAL());
    ctr++;

    return Attribute::GenericEqual<CcInt>( this, rhs,
                           IsDefined(), rhs->IsDefined() );
  }

  inline virtual bool Less(const CcInt* rhs) const
  {
    static long& ctr = Counter::getRef(Symbol::CTR_INT_LESS());
    ctr++;

    return Attribute::GenericLess<CcInt>( this, rhs,
                          IsDefined(), rhs->IsDefined() );
  }


  inline bool Adjacent(const Attribute* arg) const
  {
    static long& ctr = Counter::getRef(Symbol::CTR_INT_ADJACENT());
    ctr++;

    int a = GetIntval(),
        b = dynamic_cast<const CcInt*>(arg)->GetIntval();

    return( a == b || a == b + 1 || b == a + 1 );
  }

  inline CcInt* Clone() const
  {
    return (new CcInt( this->IsDefined(), this->intval ));
  }

  inline ostream& Print( ostream& os ) const
  {
     if ( IsDefined() )
       return (os << intval);
     else
       return (os << "UNDEFINED");
  }

  inline ostream& operator<<(ostream& os) const
  {
    return Print(os);
  }

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

  static ListExpr Out(const ListExpr typeInfo, Word value);

  static Word In(ListExpr typeInfo, ListExpr value,
                 int errorPos, ListExpr& errorInfo, bool& correct);

  inline bool operator==(const CcInt& rhs) const
  {
    return intval == rhs.intval;
  }

  inline bool operator<(const CcInt& rhs) const
  {
    return intval < rhs.intval;
  }

  inline void operator=(const CcInt& rhs)
  {
    intval = rhs.intval;
    SetDefined( rhs.IsDefined() );
  }

  virtual string getCsvStr() const{
    if(!IsDefined()){
       return "-";
    } else {
       stringstream o;
       o << intval;
       return o.str();
    }
  }


  virtual bool hasDB3Representation() const {return true;}
  virtual unsigned char getDB3Type() const { return 'N'; }
  virtual unsigned char getDB3Length() const { return 15; }
  virtual unsigned char getDB3DecimalCount(){ return 0; }
  virtual string getDB3String() const {
      if(!IsDefined()){
        return "";
      }
      stringstream s;
      s << intval;
      return s.str();
  }

  virtual void ReadFromString(string value){
      trimstring(value);
      if(value.size()==0){
         SetDefined(false);
      } else {
         stringstream ss(value);
         int v=0;
         ss >> v;
         Set(true,v);
      }
  }

  static const string BasicType(){
    return "int";
  }

  static const bool checkType(const ListExpr list){
    return listutils::isSymbol(list, BasicType());
  }

  static const string Example(){
    return "5";
  }
  virtual bool hasTextRepresentation() const{
    return true;
  }
  virtual string toText() const{
    return getCsvStr();
  }

  virtual bool fromText(const string& value){
    ReadFromString(value);
    return IsDefined();
  }



  inline virtual size_t SerializedSize() const
  {
    return sizeof(int32_t) + 1;
  }

  inline virtual void Serialize(char* storage, size_t sz, size_t offset) const
  {
    WriteVar<int32_t>(intval, storage, offset);
    WriteVar<bool>(IsDefined(), storage, offset);
  }

  inline virtual void Rebuild(char* state,  size_t sz )
  {
    size_t offset = 0;
    ReadVar<int32_t>(intval, state, offset);
    ReadVar<bool>(del.isDefined, state, offset);
  }


  inline virtual StorageType GetStorageType() const { return Core; }

  static long intsCreated;
  static long intsDeleted;

 private:
  int32_t intval;
};



/*
3.1 CcReal

*/


#define SEC_STD_REAL double
class CcReal : public Attribute
{
 public:
  inline CcReal()
  {
     realsCreated++;
  }

  explicit inline CcReal( bool d, SEC_STD_REAL v = 0.0 ) : 
      Attribute(d),realval(v)
  {
    SetDefined(d);
    realsCreated++;
  }

  explicit inline CcReal( SEC_STD_REAL v, bool d=true ) : 
      Attribute(d),realval(v)
  {
    SetDefined(d);
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

  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline SEC_STD_REAL GetRealval() const
  {
    return realval;
  }

  inline SEC_STD_REAL GetValue() const
  {
    return realval;
  }


  inline void Set( SEC_STD_REAL v )
  {
    SetDefined(true);
    realval = v;
  }

  inline void Set( bool d, SEC_STD_REAL v )
  {
    SetDefined(d);
    realval = v;
  }

  inline CcReal* Clone() const
  {
    return (new CcReal(this->IsDefined(), this->realval));
  }

  inline size_t HashValue() const
  {
    if(!IsDefined())
      return 0;

    unsigned long h = 0;
    char* s = (char*)&realval;
    for(unsigned int i = 1; i <= sizeof(SEC_STD_REAL) / sizeof(char); i++)
    {
      h = 5 * h + *s;
      s++;
    }
    return size_t(h);
  }

  inline void CopyFrom(const Attribute* right)
  {
    const CcReal* r = (const CcReal*)right;
    SetDefined(r->IsDefined());
    realval = r->realval;
  }

  inline int Compare( const Attribute* arg ) const
  {
    const CcReal* rhs = static_cast<const CcReal*>( arg );
    static long& ctr = Counter::getRef("CcReal::Compare");
    ctr++;
    return Attribute::GenericCompare<CcReal>( this, rhs,
                              IsDefined(), rhs->IsDefined() );
  }

  inline int CompareAlmost( const Attribute* arg ) const
  {
    const CcReal* rhs = static_cast<const CcReal*>( arg );
    static long& ctr = Counter::getRef("CcReal::Compare");
    ctr++;

    double diff = fabs( GetRealval() - rhs->GetRealval() );
    if (diff < FACTOR )
      return 0;
    else
      return
        Attribute::GenericCompare<CcReal>( this, rhs,
                   IsDefined(), rhs->IsDefined() );
  }

  inline bool Adjacent( const Attribute *arg ) const
  {
    // original implementation:
    //    return( realval == ((const CcReal *)arg)->realval );
    // '==' changed to 'CompareAlmost() == 0' to avoid
    // problems with 64bit environments:
    return( CompareAlmost(arg) == 0 );
  }

  inline ostream& Print( ostream &os ) const { return (os << realval); }

  static long realsCreated;
  static long realsDeleted;

  inline bool operator==(const CcReal& rhs) const
  {
    return realval == rhs.realval;
  }
  inline bool operator<(const CcReal& rhs) const
  {
    return realval < rhs.realval;
  }

  virtual string getCsvStr() const{
    if(!IsDefined()){
       return "-";
    } else {
       stringstream o;
       o << realval;
       return o.str();
    }
  }


  virtual bool hasDB3Representation() const {return true;}
  virtual unsigned char getDB3Type() const { return 'N'; }
  virtual unsigned char getDB3Length() const { return 15; }
  virtual unsigned char getDB3DecimalCount(){ return 6; }
  virtual string getDB3String() const {
      if(!IsDefined()){
        return "";
      }
      ostringstream s;
      s.setf(ios::fixed);
      s.setf(ios::showpoint);
      s.setf(ios::left);
      s.width(15);
      s.precision(6);
      s << realval;
      s.flush();
      return s.str();
  }

  virtual void ReadFromString(string value){
      trimstring(value);
      if(value.size()==0){
         SetDefined(false);
      } else {
         stringstream ss(value);
         double v=0.0;
         ss >> v;
         Set(true,v);
      }
  }

  virtual bool hasTextRepresentation() const{
    return true;
  }
  virtual string toText() const{
    return getCsvStr();
  }

  virtual bool fromText(const string& value){
    ReadFromString(value);
    return IsDefined();
  }



  static const string BasicType(){
    return "real";
  }

  static const bool checkType(const ListExpr list){
    return listutils::isSymbol(list, BasicType());
  }

  inline virtual size_t SerializedSize() const
  {
    return sizeof(SEC_STD_REAL) + 1;
  }

  inline virtual void Serialize(char* storage, size_t sz, size_t offset) const
  {
    WriteVar<SEC_STD_REAL>(realval, storage, offset);
    WriteVar<bool>(IsDefined(), storage, offset);
  }

  inline virtual void Rebuild(char* state,  size_t sz )
  {
    size_t offset = 0;
    ReadVar<SEC_STD_REAL>(realval, state, offset);
    ReadVar<bool>(del.isDefined, state, offset);
  }


  inline virtual StorageType GetStorageType() const { return Core; }

 private:
  SEC_STD_REAL  realval;
};



/*
4.1 CcBool

*/

class CcBool : public Attribute
{
 public:
  inline CcBool()
  {
    boolsCreated++;
  }

  explicit inline CcBool( bool d, int v = false ) : Attribute(d),boolval(v)
  {
    SetDefined(d);
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
    SetDefined(d);
    boolval = v;
  }

  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline bool GetBoolval() const
  {
    return boolval;
  }

  inline bool GetValue() const
  {
    return boolval;
  }

  inline CcBool* Clone() const
  {
    return new CcBool(this->IsDefined(), this->boolval);
  }

  inline size_t HashValue() const
  {
    return (IsDefined() ? boolval : false);
  }

  inline void CopyFrom(const Attribute* right)
  {
    const CcBool* r = (const CcBool*)right;
    SetDefined(r->IsDefined());
    boolval = r->boolval;
  }

  inline int Compare( const Attribute* arg ) const
  {
    const CcBool* rhs = static_cast<const CcBool*>( arg );
    return Attribute::GenericCompare<CcBool>( this, rhs,
                              IsDefined(), rhs->IsDefined() );
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

  inline bool operator==(const CcBool& rhs) const
  {
    return boolval == rhs.boolval;
  }

  inline bool operator<(const CcBool& rhs) const
  {
    return boolval < rhs.boolval;
  }


  static long boolsCreated;
  static long boolsDeleted;

  virtual string getCsvStr() const{
    if(!IsDefined()){
       return "-";
    } else {
      return boolval?"true":"false";
    }
  }

  virtual bool hasDB3Representation() const {return true;}
  virtual unsigned char getDB3Type() const { return 'L'; }
  virtual unsigned char getDB3Length() const { return 1; }
  virtual unsigned char getDB3DecimalCount(){ return 0; }
  virtual string getDB3String() const {
      if(!IsDefined()){
        return "?";
      }
      return boolval?"T":"F";
  }

  virtual void ReadFromString(string value){
     string::size_type p = value.find_first_not_of(" \t");
     if(p==string::npos){
       SetDefined(false);
     } else {
       char c = value[p];
       if(c=='F' || c=='f' || c=='0'){
          Set(true,false);
       } else{
          Set(true,true);
       }
     }
  }

  virtual bool hasTextRepresentation() const{
     return true;
  }

  virtual string toText() const{
     return getCsvStr();
  }

  virtual bool fromText(const string& value) {
     ReadFromString(value);
     return IsDefined();
  }


  static const string BasicType(){
     return "bool";
  }

  static const bool checkType(const ListExpr list){
    return listutils::isSymbol(list, BasicType());
  }

 private:
  bool boolval;
};

/*
5.1 CcString

*/

typedef char STRING_T[MAX_STRINGSIZE+1];


#ifdef COMPILE_CCSTRING_WITH_STDSTRING

class CcString : public Attribute
{
 public:
  inline CcString()
  {
    stringsCreated++;
  }

  inline CcString( bool d, const STRING_T* v ):Attribute(d)
  {
    Set(d, v);
    stringsCreated++;
  }

  inline CcString( const bool d, const string& v ):Attribute(d)
  {
    Set(d, v);
    stringsCreated++;
  }

  inline ~CcString()
  {
    stringsDeleted++;
  }

  bool operator==(const CcString& other) const{
    return Compare(&other)==0;
  }


  inline void Initialize()
  {}

  inline void Finalize()
  {
    stringsDeleted++;
  }

  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline const STRING_T* GetStringval() const
  {
#ifndef USE_SERIALIZATION
    return &stringval;
#else
    return (STRING_T*)stringval.c_str();
#endif
  }

  inline const string GetValue() const
  {
    return stringval;
  }

  inline CcString* Clone() const
  {
    return (new CcString( this->IsDefined(), this->stringval.c_str() ));
  }

  inline void Set( bool d, const STRING_T* v )
  {
    SetDefined(d);
#ifndef USE_SERIALIZATION
    strcpy( stringval, *v);
#else
    stringval = (char*)v;
#endif
  }

  inline void Set( const bool d, const string& v )
  {
    SetDefined(d);
#ifndef USE_SERIALIZATION
    memset ( stringval, '\0',     MAX_STRINGSIZE+1);
    strncpy( stringval, v.data(), MAX_STRINGSIZE  );
#else
    stringval = v;
#endif
  }

  inline size_t HashValue() const
  {
    static long& ctr = Counter::getRef("CcString::HashValue");
    ctr++;
    if(!IsDefined())
      return 0;

    size_t h = 0;
#ifndef USE_SERIALIZATION
    const char* s = stringval;
#else
    const char* s = stringval.c_str();
#endif
    while(*s != 0)
    {
      h = 5 * h + *s;
      s++;
    }
    return h;
  }

  inline void CopyFrom(const Attribute* right)
  {
    const CcString* r = (const CcString*)right;
    SetDefined( r->IsDefined() );
#ifndef USE_SERIALIZATION
    strcpy(stringval, r->stringval);
#else
    stringval = r->stringval;
#endif
  }

  inline int Compare( const Attribute* arg ) const
  {
    const CcString* rhs = static_cast<const CcString*>( arg );
    static long& ctr = Counter::getRef("CcString::Compare");
    ctr++;

    if (IsDefined() && rhs->IsDefined())
    {
#ifndef USE_SERIALIZATION
      const int cmp = strcmp(stringval, rhs->stringval);
      if (cmp == 0)
         return 0;
      else
         return (cmp < 0) ? -1 : 1;
#else
      if (stringval == rhs->stringval)
        return 0;
      else
       return (stringval < rhs->stringval) ? -1 : 1;
#endif
    }
    else
    {
      // compare only the IsDefined flags
      if( !IsDefined() ) {
        if ( !rhs->IsDefined() )  // case 00
          return 0;
        else          // case 01
          return -1;
      }
      return 1;       // case 10
    }
  }

  bool Adjacent( const Attribute* arg ) const;

  inline ostream& Print( ostream &os ) const {
    return (os << "\"" << stringval << "\"");
  }

  static long stringsCreated;
  static long stringsDeleted;

  virtual string getCsvStr() const{
    if(!IsDefined()){
       return "-";
    } else {
      return stringval;
    }
  }

  virtual bool hasDB3Representation() const {return true;}
  virtual unsigned char getDB3Type() const { return 'C'; }
  virtual unsigned char getDB3Length() const { return MAX_STRINGSIZE; }
  virtual unsigned char getDB3DecimalCount(){ return 0; }
  virtual string getDB3String() const {
      if(!IsDefined()){
        return "";
      }
      return string(stringval);
  }

  virtual void ReadFromString(string value){
     if(value.size()>MAX_STRINGSIZE){
        value = value.substr(0,MAX_STRINGSIZE);
     }
     Set(true,value);
  }

  virtual bool hasTextRepresentation() const{
    return true;
  }

  virtual string toText() const{
    return getCsvStr();
  }

  virtual bool fromText(const string& value) {
     ReadFromString(value);
     return IsDefined();
  }


  static const string BasicType(){
    return "string";
  }
  static const bool checkType(const ListExpr list){
    return listutils::isSymbol(list, BasicType());
  }

  void trim(){
    if(!IsDefined()){
      return;
    }
    string s = GetValue();
    stringutils::trim(s);
    Set(true,s);
  }


#ifdef USE_SERIALIZATION
  inline virtual size_t SerializedSize() const
  {
    return sizeof(uint8_t) + stringval.size() + 1;
  }

  inline virtual void Serialize(char* storage, size_t sz, size_t offset) const
  {
    WriteVar<uint8_t>(stringval.size(), storage, offset);
    WriteVar<bool>(IsDefined(), storage, offset);
    stringval.copy(&storage[offset], string::npos);
  }

  inline virtual void Rebuild(char* state,  size_t sz )
  {
    size_t offset = 0;
    uint8_t size = 0;
    ReadVar<uint8_t>(size, state, offset);
    ReadVar<bool>(del.isDefined, state, offset);
    char* buffer = new char[size+1];
    memcpy(buffer, &state[offset], size);
    buffer[size] = '\0';
    stringval.assign(buffer);

  }

  inline virtual StorageType GetStorageType() const { return Extension; }
#endif


 private:
#ifdef USE_SERIALIZATION
  string   stringval;
#else
  STRING_T stringval;
#endif
};

#endif


class CcString : public Attribute
{
 public:
  inline CcString()
  {
    stringsCreated++;
  }

  void ShowMem() {

    cout << Var2HexStr(del);
    cout << Var2HexStr(stringval);
    cout << Var2HexStr(size);
  }

  inline CcString( bool d, const STRING_T* v ) : Attribute(d)
  {
    Set(d, v);
    //cout << "Cc1" << endl;
    stringsCreated++;
  }

  inline CcString( const bool d, const string& v ) : Attribute(d)
  {
    Set(d, v);
    //cout << "Cc2" << endl;
    stringsCreated++;
  }

  explicit inline CcString( const string& v) : Attribute(true)
  {
    Set(true, v);
    //cout << "Cc3" << endl;
    stringsCreated++;
  }

  bool operator==(const CcString& other) const{
    return Compare(&other)==0;
  }



  inline ~CcString()
  {
    stringsDeleted++;
  }

  inline void Initialize()
  {}

  inline void Finalize()
  {
    stringsDeleted++;
  }


  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline const STRING_T* GetStringval() const
  {
    return &stringval;
  }

  inline const string GetValue() const
  {
    return stringval;
  }

  inline CcString* Clone() const
  {
    return (new CcString( IsDefined(), &this->stringval ));
  }

  inline void Set( bool d, const STRING_T* v )
  {
    SetDefined( d );
    memset ( stringval, '\0',     MAX_STRINGSIZE+1);
    strcpy( stringval, *v);
#ifdef USE_SERIALIZATION
    for( size = 0; (*v)[size] != '\0'; size++ );
#endif
    //cerr << "Set 1: stringval = " << stringval << endl;
    //cerr << "Set 1: stringval.size = " << size << endl;
  }

  inline void Set( const bool d, const string& v )
  {
    SetDefined( d );
    memset ( stringval, '\0',     MAX_STRINGSIZE+1);
    strncpy( stringval, v.data(), MAX_STRINGSIZE  );
#ifdef USE_SERIALIZATION
    size = v.size();
#endif
    //cerr << "Set 2: stringval = " << stringval << endl;
    //cerr << "Set 2: stringval.size = " << size << endl;
  }

  inline size_t HashValue() const
  {
    static long& ctr = Counter::getRef("CcString::HashValue");
    ctr++;
    if(!IsDefined())
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

  inline void CopyFrom(const Attribute* right)
  {
    const CcString* r = static_cast<const CcString*>( right );
    Set( r->IsDefined(), r->stringval );
  }

  inline int Compare( const Attribute* arg ) const
  {
    const CcString* rhs = static_cast<const CcString*>( arg );
    static long& ctr = Counter::getRef("CcString::Compare");
    ctr++;

    if ( IsDefined() && rhs->IsDefined() )
    {
      const int cmp = strcmp(stringval, rhs->stringval);
      if (cmp == 0)
         return 0;
      else
         return (cmp < 0) ? -1 : 1;
    }
    else
    {
      // compare only the defined flags
      if( !IsDefined() ) {
        if ( !rhs->IsDefined() )  // case 00
          return 0;
        else          // case 01
          return -1;
      }
      return 1;       // case 10
    }
  }

  bool Adjacent( const Attribute* arg ) const;

  inline ostream& Print( ostream &os ) const {
    return (os << "\"" << stringval << "\"");
  }

  static long stringsCreated;
  static long stringsDeleted;

  virtual string getCsvStr() const{
    if(!IsDefined()){
       return "-";
    } else {
      return stringval;
    }
  }

  virtual bool hasDB3Representation() const {return true;}
  virtual unsigned char getDB3Type() const { return 'C'; }
  virtual unsigned char getDB3Length() const { return MAX_STRINGSIZE; }
  virtual unsigned char getDB3DecimalCount(){ return 0; }
  virtual string getDB3String() const {
      if(!IsDefined()){
        return "";
      }
      return string(stringval);
  }

  virtual void ReadFromString(string value){
     if(value.size()>MAX_STRINGSIZE){
        value = value.substr(0,MAX_STRINGSIZE);
     }
     Set(true,value);
  }

  virtual bool hasTextRepresentation() const{
    return true;
  }

  virtual string toText() const{
     return getCsvStr();
  }

  virtual bool fromText(const string& value) {
    ReadFromString(value);
    return IsDefined();
  }



  static const string BasicType(){
    return "string";
  }
  static const bool checkType(const ListExpr list){
    return listutils::isSymbol(list, BasicType());
  }


  void trim(){
    if(!IsDefined()){
      return;
    }
    string s = GetValue();
    stringutils::trim(s);
    Set(true,s);
  }

#ifdef USE_SERIALIZATION
  inline virtual size_t SerializedSize() const
  {
    return  sizeof(uint8_t) + 1 /* defined */ + size;
  }

  inline virtual void Serialize(char* storage, size_t sz, size_t offset) const
  {
    WriteVar<uint8_t>(size, storage, offset);
    WriteVar<bool>(IsDefined(), storage, offset);
    memcpy(&storage[offset], stringval, size);
  }

  inline virtual void Rebuild(char* state,  size_t sz )
  {
    size_t offset = 0;
    ReadVar<uint8_t>(size, state, offset);
    ReadVar<bool>(del.isDefined, state, offset);
    memcpy(stringval, &state[offset], size);
    stringval[size] = '\0';
  }

  inline virtual StorageType GetStorageType() const { return Extension; }
#endif


 private:
#ifdef USE_SERIALIZATION
  uint8_t  size;
#endif
  STRING_T stringval;
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
  static SEC_STD_REAL GetReal(const Word& w);
  static bool GetBool(const Word& w);
  static string GetString(const Word& w);

  static void InitCounters(bool show);
  static void SetCounterValues(bool show);
  static long UpdateBasicOps(bool reset=false);
  static void ResetBasicOps();
};

#endif

