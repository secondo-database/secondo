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
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header File of the MapMatching Algebra

April, 2012. Matthias Roth

[TOC]

1 Overview

This header file essentially contains the definition of the class ~MGPointCreator~.

2 Defines and includes

*/

#ifndef MAPMATCHINGMHTORELCREATOR_H_
#define MAPMATCHINGMHTORELCREATOR_H_

#include "MapMatchingMHT.h"
#include "MHTRouteCandidate.h"

class TupleBuffer;
class GenericRelationIterator;


namespace mapmatch {

/*
3 class OEdgeTupleStreamCreator

*/
class OEdgeTupleStreamCreator : public IMapMatchingMHTResultCreator
{
public:
    OEdgeTupleStreamCreator();
    virtual ~OEdgeTupleStreamCreator();

    virtual bool CreateResult(const std::vector<MHTRouteCandidate*>&
                                                           rvecRouteCandidates);

    Tuple* GetNextTuple(void) const;

private:

    TupleBuffer* m_pTupleBuffer;
    mutable GenericRelationIterator* m_pTupleIterator;
};




} // end of namespace mapmatch


#endif /* MAPMATCHINGMHTORELCREATOR_H_ */
