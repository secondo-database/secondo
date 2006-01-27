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

#include "StandardAttribute.h"
#include "FLOB.h"

class FText: public StandardAttribute
{
public:

  inline FText();
  inline FText(bool newDefined, const char *newText = NULL);
  inline FText(const FText&);
  inline ~FText();
  inline void Destroy();

  inline bool  SearchString( const char* subString );
  inline void  Set( const char *newString );
  inline void  Set( bool newDefined, const char *newString );
  inline int TextLength() const;
  inline const char *Get() const;

/*************************************************************************

  The following virtual functions:
  IsDefined, SetDefined, HashValue, CopyFrom, Compare, Sizeof, Clone, Print, Adjacent
  need to be defined if we want to use ~text~ as an attribute type in tuple definitions.

*************************************************************************/

  inline bool IsDefined() const;
  inline void SetDefined(bool newDefined);
  size_t HashValue() const;
  void CopyFrom(const StandardAttribute* right);
  int Compare(const Attribute * arg) const;
  inline FText* Clone() const;
  ostream& Print(ostream &os) const;
  bool Adjacent(const Attribute * arg) const;

  inline int NumOfFLOBs() const;
  inline FLOB* GetFLOB( const int );

private:
  FLOB theText;
  bool defined;
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

inline FText::FText( bool newDefined, const char *newText ) :
theText( 0 )
{
  LOGMSG( "FText:Trace", cout << '\n' <<"Start FText(bool newDefined, textType newText)"<<'\n'; )
  Set( newDefined, newText );
  LOGMSG( "FText:Trace",  cout <<"End FText(bool newDefined, textType newText)"<<'\n'; )
}

inline FText::FText( const FText& f ) :
theText( 0 )
{
  LOGMSG( "FText:Trace", cout << '\n' <<"Start FText(FText& f)"<<'\n'; )
  Set( f.defined, f.theText.BringToMemory() );
  LOGMSG( "FText:Trace",  cout <<"End FText(FText& f)"<<'\n'; )
}

inline FText::~FText()
{
  LOGMSG( "FText:Trace",  cout << '\n' <<"Start ~FText()"<<'\n'; )
  LOGMSG( "FText:Trace",  cout <<"End ~FText()"<<'\n'; )
}

inline void FText::Destroy()
{
  theText.Destroy();
  defined = false;
}

inline bool FText::SearchString( const char* subString )
{
  const char *text = theText.BringToMemory();
  return strstr( text, subString ) != NULL;
}

inline void FText::Set( const char *newString )
{
  LOGMSG( "FText:Trace", cout << '\n' << "Start Set with *newString='"<< newString << endl; )

  Set( true, newString );

  LOGMSG( "FText:Trace", cout <<"End Set"<<'\n'; )
}

inline void FText::Set( bool newDefined, const char *newString )
{
  LOGMSG( "FText:Trace", cout << '\n' << "Start Set with *newString='"<< newString << endl; )

  if( newString != NULL )
  {
    theText.Resize( strlen( newString ) + 1 );
    theText.Put( 0, strlen( newString ) + 1, newString );
  }
  defined = newDefined;

  LOGMSG( "FText:Trace", cout <<"End Set"<<'\n'; )
}

inline int FText::TextLength() const
{
  return theText.Size() - 1;
}

inline const char *FText::Get() const
{
  return theText.BringToMemory();
}

inline bool FText::IsDefined() const
{
  return defined;
}

inline void FText::SetDefined( bool newDefined )
{
  if(traces)
    cout << '\n' << "Start SetDefined" << '\n';
  defined = newDefined;
}

inline FText* FText::Clone() const
{
  return new FText( *this );
}

inline int FText::NumOfFLOBs() const
{
  return 1;
}

inline FLOB* FText::GetFLOB( const int i )
{
  assert( i == 0 );
  return &theText;
}

#endif 

