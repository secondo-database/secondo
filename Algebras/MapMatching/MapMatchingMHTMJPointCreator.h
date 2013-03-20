/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012, November Simone Jandt

1 Defines and Includes

*/

#ifndef MAPMATCHINGMHTMJPOINTCREATOR_H
#define MAPMATCHINGMHTMJPOINTCREATOR_H


#include "MapMatchingMHT.h"
#include "MHTRouteCandidate.h"
#include <TemporalAlgebra.h>
#include "JNetwork.h"
#include "MJPoint.h"
#include "RouteLocation.h"
#include "JNetworkAdapter.h"

using namespace jnetwork;

namespace datetime
{
  class DateTime;
}


namespace mapmatch {
  class JNetworkAdapter;

/*
1 Class MapMatchingMHTMJPointCreator

*/

class MJPointCreator : public IMapMatchingMHTResultCreator
{
public:

  MJPointCreator(JNetworkAdapter* pJNetAdapter, MJPoint* pResMJPoint);
  virtual ~MJPointCreator();

  virtual bool CreateResult(const std::vector<MHTRouteCandidate*>&
                                                          rvecRouteCandidates);

private:

  JNetwork* jnet;
  MJPoint* resMJPoint;

  bool Init(void);
  void Finalize(void);

  RouteLocation* GetRouteLocation(const MHTRouteCandidate::PointDataPtr pData)
                                                                        const;

};

} //end of namespache mapmatch

#endif // MAPMATCHINGMHTMJPOINTCREATOR_H
