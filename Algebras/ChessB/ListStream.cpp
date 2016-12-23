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

//characters [1] verbatim:  [\verb@]         [@]
//paragraph  [1] title:     [{\Large \bf ]   [}]


[1] ListStream.cpp

1 Defines and includes

*/

#include <stdexcept>
#include "QueryProcessor.h"
#include "ListStream.hpp"


/*

2. helper functions to avoid redundancy when dealing
     with secondo ListExpr: Extract elementary types int, double,
     bool, string from a ListExpr

*/

template<> int from_atom<int>( ListExpr instance )
{
    if ( ! nl->IsAtom( instance ) || nl->AtomType( instance ) != IntType )
        throw std::runtime_error( "Atom with type INT expected!" );

    return nl->IntValue( instance );
}

template<> uint8_t from_atom<uint8_t>( ListExpr instance )
{
    return uint8_t( from_atom<int>( instance ) );
}

template<> double from_atom<double>( ListExpr instance )
{
    if ( ! nl->IsAtom( instance ) || nl->AtomType( instance ) != RealType )
        throw std::runtime_error( "Atom with type REAL expected!" );

    return nl->RealValue( instance );
}

template<> bool from_atom<bool>( ListExpr instance )
{
    if ( ! nl->IsAtom( instance ) || nl->AtomType( instance ) != BoolType )
        throw std::runtime_error( "Atom with type BOOL expected!" );

    return nl->BoolValue( instance );
}

template<> std::string from_atom<std::string>( ListExpr instance )
{
    if ( nl->IsAtom( instance ) )
    {
        NodeType type = nl->AtomType( instance );
        if ( StringType == type )
            return nl->StringValue( instance );
        else if ( SymbolType == type )
            return nl->SymbolValue( instance );
        else if ( TextType == type )
        {
            std::string tmp;
            TextScan ts = nl->CreateTextScan( instance );
            nl->GetText( ts, nl->TextLength( instance ), tmp );
            nl->DestroyTextScan( ts );
            return tmp;
        }
    }
    throw std::runtime_error( "Atom convertable to string expected!" );
}
