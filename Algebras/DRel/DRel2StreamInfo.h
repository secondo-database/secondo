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

*/
#ifndef _DRel2StreamInfo_h_
#define _DRel2StreamInfo_h_

#include "Algebras/Distributed2/DArray.h"

namespace distributed2 {
    template<class A>
    class dsummarizeRelInfo;
}

namespace drel {
    /*
    1 ~DRel2StreamInfo~

    DRel2StreamInfo is a container for a drel and a 
    
    */
    template<class T>
    class DRel2StreamInfo {

    public:
        /*
        1.1 Constructor

        */
        DRel2StreamInfo( distributed2::dsummarizeRelInfo<T>* _relInfo, 
            QueryProcessor* _qp, OpTree _tree ) :
            relInfo( _relInfo ), qp( _qp ), tree( _tree ) {
        }
        
        /*
        1.2 Destructor

        */
        ~DRel2StreamInfo( ) {
            //delete darray;
            //delete qp;
        }
       
        /*
        1.3 Members
        
        */
        distributed2::dsummarizeRelInfo<T>* relInfo;
        QueryProcessor* qp;
        OpTree tree;
    };
    
} // end of namespace drel

#endif // _DRel2StreamInfo_h_