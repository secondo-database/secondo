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

#include "JNetworkAdapter.h"
#include "JNetworkSectionAdapter.h"

using namespace jnetwork;

namespace mapmatch{
/*
1 Implementation of JNetworkAdapter

1.1. Constructors and Deconstructors

*/

JNetworkAdapter::JNetworkAdapter(JNetwork* jnet /*=NULL*/) : pJNet(jnet)
{}

JNetworkAdapter::JNetworkAdapter(const JNetworkAdapter& other) :
   pJNet(other.pJNet)
{}

JNetworkAdapter::~JNetworkAdapter()
{}

/*
1.1. Get Networkinformations

*/

bool JNetworkAdapter::GetSections(const Rectangle< 2 >& rBBox,
             vector< shared_ptr<IMMNetworkSection> >& vecSections) const
{
  vector<TupleId>* listSectTup  = new vector<TupleId> (0);
  pJNet->GetSectionTuplesFor(rBBox, *listSectTup);
  vecSections.clear();
  bool ok = listSectTup->size() > 0;
  if (ok)
  {
    std::vector<TupleId>::const_iterator itEnd = listSectTup->end();
    for (std::vector<TupleId>::const_iterator it = listSectTup->begin();
             it != itEnd; ++it)
    {
      shared_ptr<IMMNetworkSection> sectTupId(new JNetworkSectionAdapter(pJNet,
                                                                         *it));
      vecSections.push_back(sectTupId);
    }
    listSectTup->clear();
  }
  delete listSectTup;
  return ok;
}

Rectangle< 2 > JNetworkAdapter::GetBoundingBox(void ) const
{
  if (pJNet != 0)
    return pJNet->GetBoundingBox();
  else
    return Rectangle<2>(false);
}

double JNetworkAdapter::GetNetworkScale() const
{
  //return pJNet->GetTolerance();
  return 1.0;
}

bool JNetworkAdapter::CanGetRoadType() const
{
  return false;
}

bool JNetworkAdapter::IsDefined( ) const
{
  return ( pJNet != 0 && pJNet->IsDefined());
}

JNetwork* JNetworkAdapter::GetNetwork() const
{
  return pJNet;
}

} // end of namespace mapmatch
