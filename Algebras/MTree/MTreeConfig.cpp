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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

November/December 2007, Mirko Dibbert

5.7 Implementation of class "MT::MTreeConfig"[4] (file: MTreeConfig.cpp)

*/
#include "MTreeConfig.h"

/*
Initialise static members :

*/
bool MT::MTreeConfigReg::initialized = false;
map< string, MT::MTreeConfig > MT::MTreeConfigReg::mTreeConfig_map;

/*
Method ~registerMTreeConfig~ :

*/
void
MT::MTreeConfigReg::registerMTreeConfig( const string& name,
                                         const MTreeConfig& config )
{
  mTreeConfig_map[ name ] = config;
}

/*
Method ~getMTreeConfig~ :

*/
MT::MTreeConfig
MT::MTreeConfigReg::getMTreeConfig( const string& name )
{
  if (!initialized)
    initialize();

  map< string, MTreeConfig >::iterator pos =
      mTreeConfig_map.find( name );

  if ( pos != mTreeConfig_map.end() )
    return pos->second;
  else
    return MTreeConfig();
}

/*
Method ~contains~ :

*/
bool
MT::MTreeConfigReg::contains( const string& name)
{
  if (!initialized)
    initialize();

  map< string, MTreeConfig >::iterator pos =
      mTreeConfig_map.find( name );

  if ( pos != mTreeConfig_map.end() )
    return true;
  else
    return false;
}

/*
Method ~initialize~ :

*/
void
MT::MTreeConfigReg::initialize()
{
  registerMTreeConfig( "default",
      MTreeConfig(
          200,                   // maxNodeEntries
          M_LB_DIST,             // promote function
          GENERALIZED_HYPERPLANE // partition function
      ));

/*
Register config objects with maximum entries per node.

*/
  registerMTreeConfig( "random",
      MTreeConfig( 200, RANDOM, BALANCED ));

  registerMTreeConfig( "mRad",
      MTreeConfig( 200, m_RAD, BALANCED ));

  registerMTreeConfig( "randomHP",
      MTreeConfig( 200, RANDOM, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mMRad",
      MTreeConfig( 200, mM_RAD, BALANCED ));

  registerMTreeConfig( "mlbDist",
      MTreeConfig( 200, M_LB_DIST, BALANCED ));

  registerMTreeConfig( "mRadHP",
      MTreeConfig( 200, m_RAD, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mMRadHP",
      MTreeConfig( 200, mM_RAD, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mlbDistHP",
      MTreeConfig( 200, M_LB_DIST, GENERALIZED_HYPERPLANE ));

/*
Register config objects with max. 80 entries per node.

*/
 registerMTreeConfig( "random80",
      MTreeConfig( 80, RANDOM, BALANCED ));

  registerMTreeConfig( "mRad80",
      MTreeConfig( 80, m_RAD, BALANCED ));

  registerMTreeConfig( "randomHP80",
      MTreeConfig( 80, RANDOM, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mMRad80",
      MTreeConfig( 80, mM_RAD, BALANCED ));

  registerMTreeConfig( "mlbDist80",
      MTreeConfig( 80, M_LB_DIST, BALANCED ));

  registerMTreeConfig( "mRadHP80",
      MTreeConfig( 80, m_RAD, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mMRadHP80",
      MTreeConfig( 80, mM_RAD, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mlbDistHP80",
      MTreeConfig( 80, M_LB_DIST, GENERALIZED_HYPERPLANE ));

/*
Register config objects with max. 40 entries per node.

*/
  registerMTreeConfig( "random40",
      MTreeConfig( 40, RANDOM, BALANCED ));

  registerMTreeConfig( "mRad40",
      MTreeConfig( 40, m_RAD, BALANCED ));

  registerMTreeConfig( "randomHP40",
      MTreeConfig( 40, RANDOM, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mMRad40",
      MTreeConfig( 40, mM_RAD, BALANCED ));

  registerMTreeConfig( "mlbDist40",
      MTreeConfig( 40, M_LB_DIST, BALANCED ));

  registerMTreeConfig( "mRadHP40",
      MTreeConfig( 40, m_RAD, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mMRadHP40",
      MTreeConfig( 40, mM_RAD, GENERALIZED_HYPERPLANE ));

  registerMTreeConfig( "mlbDistHP40",
      MTreeConfig( 40, M_LB_DIST, GENERALIZED_HYPERPLANE ));

  initialized = true;
}
