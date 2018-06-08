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


1 ~shareInfo~

This is the class from the Distirbuted2Algebra. For more detail look at the 
Distirbuted2Algebra.

*/
#ifndef _ShareInfo_h_
#define _ShareInfo_h_

#include "NestedList.h"
#include "AlgebraTypes.h"
#include "Attribute.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

namespace distributed2 {

    class successListener {
    public:
        virtual void jobDone( int id, bool success ) = 0;
    };

    template<class AType>
    class shareInfo : public successListener {
    public:
        shareInfo( const string& _name, const bool _allowOverwrite,
            AType* _array, FText* _result );
        ~shareInfo( );
        void share( );
        void jobDone( int id, bool success );
    private:
        string name;
        bool allowOverwrite;
        AType* array;
        FText* result;
        bool fileCreated;
        Word value;
        ListExpr typeList;
        set<pair <string, int> > cons;
        string filename;
        bool isRelation;
        bool isAttribute;
        boost::mutex createFileMutex;
        vector<boost::thread*> runners;
        int failed;
        int success;
        void shareArray( );
        void shareUser( );
        void share( ConnectionInfo* ci );
        void createFile( ConnectionInfo* ci );

    };

}

#endif // _ShareInfo_h_