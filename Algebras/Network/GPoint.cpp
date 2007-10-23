/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1.1 Implementation of GPoint

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

Defines, includes, and constants

*/


#include "TupleIdentifier.h"
#include "GPoint.h"

/*
Functions

*/

/*
Static Functions supporting the type-constructor

~In~-function

*/
Word GPoint::InGPoint( const ListExpr typeInfo, 
                       const ListExpr instance,
                       const int errorPos, 
                       ListExpr& errorInfo, 
                       bool& correct )
{
  if( nl->ListLength( instance ) == 4 )
  {
    if( nl->IsAtom( nl->First(instance) ) &&
        nl->AtomType( nl->First(instance) ) == IntType && 
        nl->IsAtom( nl->Second(instance) ) &&
        nl->AtomType( nl->Second(instance) ) == IntType &&
        nl->IsAtom( nl->Third(instance) ) &&
        nl->AtomType( nl->Third(instance) ) == RealType &&
        nl->IsAtom( nl->Fourth(instance) ) &&
        nl->AtomType( nl->Fourth(instance) ) == IntType )
    {
      GPoint *gp = new GPoint( 
        true,
        nl->IntValue( nl->First(instance) ),
        nl->IntValue( nl->Second(instance) ),
        nl->RealValue( nl->Third(instance) ), 
        (Side)nl->IntValue( nl->Fourth(instance) ) );
      correct = true;
      return SetWord( gp );
    }
  }

  correct = false;
  return SetWord( Address(0) );
}

/*
~Out~-function

*/
ListExpr GPoint::OutGPoint( ListExpr typeInfo, Word value )
{
  GPoint *gp = (GPoint*)value.addr;

  if( gp->IsDefined() )
  {
    return nl->FourElemList(
      nl->IntAtom(gp->GetNetworkId()),
      nl->IntAtom(gp->GetRouteId()),
      nl->RealAtom(gp->GetPosition()),
      nl->IntAtom(gp->GetSide()));
  }
  return nl->SymbolAtom("undef");
}

/*
~Create~-function

*/
Word GPoint::CreateGPoint( const ListExpr typeInfo )
{
  return SetWord( new GPoint( false ) );
}

/*
~Delete~-function

*/
void GPoint::DeleteGPoint( const ListExpr typeInfo, Word& w )
{
  delete (GPoint*)w.addr;
  w.addr = 0;
}

/*
~Close~-function

*/
void GPoint::CloseGPoint( const ListExpr typeInfo, Word& w )
{
  delete (GPoint*)w.addr;
  w.addr = 0;
}

/*
~Clone~-function  

*/
Word GPoint::CloneGPoint( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((GPoint*)w.addr)->Clone() ); 
}

/*
~Cast~-function

*/
void* GPoint::CastGPoint( void* addr )
{
  return new (addr) GPoint;
}

/*
~SizeOf~-function

*/
int GPoint::SizeOfGPoint()
{
  return sizeof(GPoint);
}


/*
Function describing the signature of the type constructor

*/
ListExpr GPoint::GPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("gpoint"),
                             nl->StringAtom("(<network_id> <route_id> "
                                            "<position> <side>)"),
                             nl->StringAtom("(1 1 0.0 0)"))));
}

/*
Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~gpoint~ does not have arguments, this is trivial.

*/
bool GPoint::CheckGPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "gpoint" ));
}
