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

1 Implementation file "MTreeConfig.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "MTreeConfig.h"

using namespace mtreeAlgebra;
using namespace std;
using namespace gtree;

/*
Initialize static members :

*/
bool MTreeConfigReg::initialized = false;
map<string, MTreeConfig> MTreeConfigReg::configs;


/*
Method ~getConfig~ :

*/
MTreeConfig MTreeConfigReg::getConfig(const string &name)
{
  if (!initialized)
    initialize();

  map< string, MTreeConfig >::iterator pos =
      configs.find(name);

  if (pos != configs.end())
    return pos->second;
  else
    return MTreeConfig();
}

/*
Method ~isDefined~ :

*/
bool MTreeConfigReg::isDefined(const string &name)
{
  if (!initialized)
    initialize();

  map< string, MTreeConfig >::iterator pos =
      configs.find(name);

  if (pos != configs.end())
    return true;
  else
    return false;
}

/*
Method ~definedNames~ :

*/
string MTreeConfigReg::definedNames()
{
    if(!initialized)
        initialize();

    ostringstream result;
    map<string, MTreeConfig>::iterator iter = configs.begin();
    result << "\"" << CONFIG_DEFAULT << "\" ";
    while (iter != configs.end())
    {
        if (iter->first != CONFIG_DEFAULT)
            result << "\"" << iter->first << "\" ";
        ++iter;
    }
    return result.str();
}

/*
Method ~initialize~ :

*/
void
MTreeConfigReg::initialize()
{
  NodeConfig defaultleafConfig(
        LEAF, leafPrio, minLeafEntries, maxLeafEntries,
        maxLeafPages, leafCacheable);

  NodeConfig defaultinternalConfig(
        INTERNAL, internalPrio, minIntEntries, maxIntEntries,
        maxIntPages, internalCacheable);

  NodeConfig leafConfig20(
        LEAF, leafPrio, minLeafEntries, 20,
        maxLeafPages, leafCacheable);

  NodeConfig internalConfig20(
        INTERNAL, internalPrio, minIntEntries, 20,
        maxIntPages, internalCacheable);

  NodeConfig leafConfig40(
        LEAF, leafPrio, minLeafEntries, 40,
        maxLeafPages, leafCacheable);

  NodeConfig internalConfig40(
        INTERNAL, internalPrio, minIntEntries, 40,
        maxIntPages, internalCacheable);

  NodeConfig leafConfig80(
        LEAF, leafPrio, minLeafEntries, 80,
        maxLeafPages, leafCacheable);

  NodeConfig internalConfig80(
        INTERNAL, internalPrio, minIntEntries, 80,
        maxIntPages, internalCacheable);

/*
Set default config

*/
  configs[CONFIG_DEFAULT] =  MTreeConfig();

/*
Add config objects with unlimited entries per node.

*/
  configs["random_Bal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      RANDOM, BALANCED);

  configs["mRad_Bal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      m_RAD, BALANCED);

  configs["mMRad_Bal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      mM_RAD, BALANCED);

  configs["mlb_Bal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      M_LB_DIST, BALANCED);

  configs["random_HP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      RANDOM, GENERALIZED_HYPERPLANE);

  configs["mRad_HP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      m_RAD, GENERALIZED_HYPERPLANE);

  configs["mMRad_HP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      mM_RAD, GENERALIZED_HYPERPLANE);

  configs["mlb_HP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      M_LB_DIST, GENERALIZED_HYPERPLANE);

/*
Add config objects with limited entries per node (default splitpol)

*/
  configs["limit20e"] =  MTreeConfig(
      leafConfig20, internalConfig20,
      defaultPromoteFun, defaultPartitionFun);

  configs["limit40e"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      defaultPromoteFun, defaultPartitionFun);

  configs["limit80e"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      defaultPromoteFun, defaultPartitionFun);

  initialized = true;
}
