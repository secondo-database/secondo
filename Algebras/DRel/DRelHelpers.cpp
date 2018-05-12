/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


//[$][\$]


1 ~DRelHelpers~

Class with usefull helper functions for the DRelAlgebra

*/
#include "DRelHelpers.h"

#include "ListUtils.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"

using namespace std;

namespace drel {

    /*
    1.1 ~findAttribute~

    Return true if the attribute name is found in the attribute list. False 
    otherwise.

    */
    bool DRelHelpers::findAttribute( 
        ListExpr attrList, const string& name, ListExpr& type ) {

        return !( listutils::findAttribute( attrList, name, type ) == 0 );
    }

    /*
    1.2 ~countRecords~

    Counts the records in a given relation. If an error occour false is 
    returned. True otherwise. The number of records is records.

    */
    bool DRelHelpers::countRecords( 
        string attrName, string relName, int& records ) {

        string query = "( count ( filter( feed " + relName + " ) "
            "( fun( t TUPLE ) ( isdefined (attr t " + attrName + " ) ) ) ) )";
        Word result;
        if( !QueryProcessor::ExecuteQuery( query, result ) ) {
            cout << "ExecuteError of " + query << endl;
            records = -1;
            return false;
        }

        CcInt* number = ( CcInt* )result.addr;
        if( !number->IsDefined( ) ) {
            number->DeleteIfAllowed( );
            records = -1;
            return false;
        }

        records = number->GetIntval( );
        number->DeleteIfAllowed( );
        result.setAddr( 0 );

        return true;
    }

    /*bool DRelHelpers::createBoundaries( 
        string attrName, string relName, int records ) {

        string query = "( count ( filter( feed " + relName + " ) 
        ( fun( t TUPLE ) ( isdefined (attr t " + attrName + " ) ) ) ) )";
        Word result;
        if( !QueryProcessor::ExecuteQuery( query, result ) ) {
            cout << "ExecuteError of " + query << endl;
            records = -1;
            return false;
        }

        CcInt* number = ( CcInt* )result.addr;
        if( !number->IsDefined( ) ) {
            number->DeleteIfAllowed( );
            records = -1;
            return false;
        }

        records = number->GetIntval( );
        number->DeleteIfAllowed( );
        result.setAddr( 0 );

        return true;
    }*/

} // end of namespace drel