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
#ifndef _DRelAlgebra_h_
#define _DRelAlgebra_h_

#include "Algebra.h"

namespace drel {

/*
1 The DRelAlgebra

This algebra provides operators to make it easier for the user to distribute
relations to many worker systems. The algebra proviedes also operators to 
formulate queries to the distributed relations. A distributed relation is
represented by the type DRel. This algebra mainly uses the Distributed2Algebra.

*/

class DRelAlgebra: public Algebra {

  public:

/*
1.1 Constructor

*/
        DRelAlgebra();
        
};

} // end of namespace drel

#endif // _DRelAlgebra_h_