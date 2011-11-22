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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] FText Algebra

March - April 2003 Lothar Sowada

The algebra ~FText~ provides the type constructor ~text~ and two operators:

(i) ~contains~, which search text or string in a text.

(ii) ~length~ which give back the length of a text.

*/

#ifndef __F_TEXT_ALGEBRA__
#define __F_TEXT_ALGEBRA__

#include <iostream>

#undef TRACE_ON
#include "Trace.h"

#include "IndexableAttribute.h"
#include "../../Tools/Flob/Flob.h"
#include "ListUtils.h"
#include "StringUtils.h"

// switch off log messages
#undef LOGMSG
#define LOGMSG(a, b)

class FText: public IndexableAttribute
{
public:

  inline FText();
  inline FText( bool newDefined, const char *newText = NULL) ;
  inline FText( bool newDefined, const string& newText );
  inline FText(const FText&);
  inline ~FText();
  inline void Destroy();

  inline bool  SearchString( const string subString );
  inline void  Set( const char *newString );
  inline void  Set( bool newDefined, const char *newString );
  inline void  Set( bool newDefined, const string& newString );
  inline int TextLength() const;
  inline char *Get() const;
  inline const string GetValue() const;

/*************************************************************************

  The following virtual functions:
  IsDefined, SetDefined, HashValue, CopyFrom, Compare, Sizeof, Clone, Print, Adjacent
  need to be defined if we want to use ~text~ as an attribute type in tuple definitions.

*************************************************************************/

  inline size_t Sizeof() const;
  size_t HashValue() const;
  void CopyFrom(const Attribute* right);
  int Compare(const Attribute * arg) const;
  inline FText* Clone() const;
  ostream& Print(ostream &os) const;
  bool Adjacent(const Attribute * arg) const;

  inline int NumOfFLOBs() const;
  inline Flob* GetFLOB( const int );

/*************************************************************************

  For use with Btree-Indices, we need text to be in kind INDEXABLE.
  therefore, we need to inherit from IndexableAttribute

*************************************************************************/

  virtual void WriteTo( char *dest ) const;
  virtual void ReadFrom( const char *src );
  virtual SmiSize SizeOfChars() const;

  virtual bool hasDB3Representation() const {return true;}
  virtual unsigned char getDB3Type() const { return 'M'; }
  virtual unsigned char getDB3Length() const { return 0; }
  virtual unsigned char getDB3DecimalCount(){ return 0; }
  virtual string getDB3String() const { return GetValue(); }

  virtual void ReadFromString(string value){
    Set(true,value);
  }

  static const string BasicType(){
     return "text";
  }

  static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type, BasicType());
  }

  void trim() {
    if(!IsDefined()){
      return;
    }
    string  s = GetValue();
    stringutils::trim(s);
    Set(true,s);
  }

  virtual bool hasTextRepresentation() const{
    return true;
  }

  virtual string toText() const{
    return GetValue();
  }

  virtual bool fromText(const string& value) {
    Set(true,value);
    return true;
  }

private:
  Flob theText;
};

/*
2 Inline Functions

*/
const string typeName="text";
const bool traces=false;

inline FText::FText()
{
  LOGMSG( "FText:Trace", cout << '\n' <<"Start FText()"<<'\n'; )
  LOGMSG( "FText:Trace",  cout <<"End FText()"<<'\n'; )
}

inline FText::FText( bool newDefined, const char *newText /* =0*/ ) :
IndexableAttribute(newDefined),
theText( 0 )
{
  LOGMSG( "FText:Trace",
           cout << '\n'
                <<"Start FText(bool newDefined, textType newText)"
                <<'\n'; )
  if(newDefined && newText){
     Set( newDefined, newText );
  }
  LOGMSG( "FText:Trace",
           cout <<"End FText(bool newDefined, textType newText)"
                <<'\n'; )
}

inline FText::FText( bool newDefined, const string& newText ) :
    IndexableAttribute(newDefined),
    theText( 0 )
{
  LOGMSG( "FText:Trace",
          cout << '\n'
              <<"Start FText( bool newDefined, const string newText )"
              <<'\n'; )
  if(newDefined){
      Set( newDefined, newText );
  }
  LOGMSG( "FText:Trace",
          cout <<"End FText( bool newDefined, const string newText )"
              <<'\n'; )
}


inline FText::FText( const FText& f ) :
IndexableAttribute(f.IsDefined()),
theText( f.theText.getSize() )
{
  LOGMSG( "FText:Trace", cout << '\n' <<"Start FText(FText& f)"<<'\n'; )
  //SPM? Assuming Flob fits into memory
  //const char* s = new char(f.theText.getSize());
  theText.copyFrom( f.theText );
  SetDefined( f.IsDefined() );
  LOGMSG( "FText:Trace",  cout <<"End FText(FText& f)"<<'\n'; )

}

inline FText::~FText()
{
  LOGMSG( "FText:Trace",  cout << '\n' <<"Start ~FText()"<<'\n'; )
  LOGMSG( "FText:Trace",  cout <<"End ~FText()"<<'\n'; )
}

inline void FText::Destroy()
{
  theText.destroy();
  SetDefined(false);
}

inline bool FText::SearchString( string subString )
{
  string s = GetValue();
  return s.find(subString) != string::npos;
}

inline void FText::Set( const char *newString )
{
  LOGMSG( "FText:Trace",
           cout << '\n'
                << "Start Set with *newString='"<< newString << endl; )

  Set( true, newString );

  LOGMSG( "FText:Trace", cout <<"End Set"<<'\n'; )
}

inline void FText::Set( bool newDefined, const char *newString )
{
  LOGMSG( "FText:Trace",
          cout << '\n' << "Start Set with *newString='"
               << newString << endl; )

  theText.clean();
  if( newString != NULL )
  {
    SHOW(theText)
    SmiSize sz = strlen( newString ) + 1;
    if(sz>0){
      assert(newString[sz-1]==0);
      theText.write( newString, sz);
    } else {
      char d=0;
      theText.write(&d,1);
    }
  }
  SetDefined( newDefined );

  LOGMSG( "FText:Trace", cout <<"End Set"<<'\n'; )
}

inline void FText::Set( bool newDefined, const string& newString )
{
  LOGMSG( "FText:Trace",
          cout << '\n' << "Start Set with newString='"
              << newString << endl; )
  Set(newDefined,newString.c_str());
}


inline int FText::TextLength() const
{
  return theText.getSize() - 1;
}

// SPM: caller is responsible for delete
inline char *FText::Get() const
{
  assert(IsDefined());
  SmiSize sz = theText.getSize();
  if(sz==0){
    char* s = new char[1];
    s[0] = 0;
    return s;
  }

  char* s = new char[sz];
  bool ok = theText.read(s, sz);
  assert(ok);
  if(s[sz-1] !=0) {
    cerr << "Warning: last char of a text flob is not 0." << endl;
    size_t sl = strlen(s);
    cerr << " sz = " << sz << ", strlen = " << sl << endl;
    assert(sl <= sz);
  }
  return s;
}

inline const string FText::GetValue() const
{
  char* s = Get();
  string res(s);
  delete [] s;
  return res;
}

inline size_t FText::Sizeof() const
{
  if(traces)
    cout << '\n' << "Start Sizeof" << '\n';
  return sizeof( *this );
}

inline FText* FText::Clone() const
{
  return new FText( *this );
}

inline int FText::NumOfFLOBs() const
{
  return 1;
}

inline Flob* FText::GetFLOB( const int i )
{
  assert( i == 0 );
  return &theText;
}


namespace ftext{

Word CreateFText( const ListExpr typeInfo );

void DeleteFText( const ListExpr typeInfo, Word& w );

void CloseFText( const ListExpr typeInfo, Word& w );

Word CloneFText( const ListExpr typeInfo, const Word& w );

int SizeOfFText();

void* CastFText( void* addr );

ListExpr OutFText( ListExpr typeInfo, Word value );

Word InFText( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );

bool OpenFText( SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo, Word& value );

bool SaveFText( SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo, Word& value );

} // end namespace ftext

namespace SVG {
  const string BasicType();
}
#endif

