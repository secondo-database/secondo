/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] File RegionTools2.h

This file contains functions handling with regions of the Region2-Algebra and 
the MovingRegion3-Algebra.

[TOC]

1 Conversion functions for GMP-type mpq\_class

1.1 Function ~gmpTypeToTextType1()~

Reads from inValue and stores its representation as TextType in resultList.

*/



#ifndef REGION2TOOLS_H
#define REGION2TOOLS_H

#include <limits>
#include <gmp.h>
#include <gmpxx.h>
#include "SpatialAlgebra.h"


void gmpTypeToTextType1
        (const mpq_class& inValue, ListExpr& resultList);

/*
1.1 Function ~gmpTypeToTextType2()~

Reads from inValue and stores its representation as TextType in resultList.

*/
void gmpTypeToTextType2 (const mpq_class& inValue, ListExpr& resultList) ;

/*
1.1 Function ~textTypeToGmpType1()~

Reads from inList and stores its representation as mpq\_class in outValue.

*/
void textTypeToGmpType1
        (const ListExpr& inList, mpq_class& outValue);

/*
1.1 Function ~textTypeToGmpType2()~

Reads from inList and stores its representation as mpq\_class in outValue.

*/
void textTypeToGmpType2
        (const ListExpr& inList, mpq_class& outValue);

/*
1.1 Function ~D2MPQ()~

Converts value of type double to a value of type mpq\_class 

*/
mpq_class D2MPQ( const double d );


/*
1 Functions to write the precise values to and read them from the DbArray 
 
1.1 Function ~SetValueX()~

Stores value z of type mpz\_class in DbArray of type unsigned int, gives back the startposition and 
the number of used unsigned int-values

*/
void SetValueX(mpz_class z, DbArray<unsigned int>* preciseValuesArray, 
                      int& startpos, int& numofInt) ;


/*
1.1 Function ~GetValueX()~

Reads value of type mpz\_class from DbArray of type unsigned int, starting at startposition and 
with number of used unsigned int-values

*/
mpz_class GetValueX(const int startpos, const int numofInt, 
                           const DbArray<unsigned int>* preciseValuesArray);
  

/*
1 Functions to detect an overflow of integer values

1.1 Function ~overflowAsInt~

checks int-overflow of x1 or x2 with scalefactor s

*/
bool overflowAsInt(mpq_class x1, mpq_class x2, int s = 0);


/*
1.1 Function ~overflowAsInt~

checks int-overflow of x1 or x2 by scaling with s

*/
bool overflowAsInt(mpq_class x1, mpq_class x2, double s);


/*
1.1 Function ~checkFactorOverflow~

checks int-overflow of maxI or minI with scalefactor s

*/
bool checkFactorOverflow(int maxI, int minI, int s = 0);


/*
1.1 Function ~checkFactorOverflow~

checks int-overflow of maxI or minI by scaling with s

*/
bool checkFactorOverflow(int maxI, int minI, double s = 1.0);


#endif

