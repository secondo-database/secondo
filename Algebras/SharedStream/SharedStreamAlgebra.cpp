
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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[->] [$\rightarrow$]
 //[TOC] [\tableofcontents]
 //[_] [\_]

*/
#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Algebras/Stream/Stream.h"             // wrapper for secondo streams
//#include "GenericTC.h"          // use of generic type constructors




#include "LogMsg.h"             // send error messages

#include "Tools/Flob/DbArray.h"  // use of DbArrays


#include "Algebras/Relation-C++/RelationAlgebra.h"           // use of tuples
#include "Algebras/Distributed2/ErrorWriter.h"
#include "Algebras/Distributed2/DArray.h"

#include <math.h>               // required for some operators
#include <stack>
#include <limits>

#include "SocketIO.h"

/*
 0.5 Global Variables

 Secondo uses some variables designed as singleton pattern. For accessing these
 global variables, these variables have to be declared to be extern:

*/

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

/*
 0.6 Namespace

 Each algebra file defines a lot of functions. Thus, name conflicts may arise
 with function names defined in other algebra modules during compiling/linking
 the system. To avoid these conflicts, the algebra implementation should be
 embedded into a namespace.

*/

namespace sharedstream {


//alles ausgelagert. Algebraklasse folgt noch.












}//end namespace

