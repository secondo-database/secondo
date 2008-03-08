/*
\newpage

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

January-February 2008, Mirko Dibbert

1 Implementation file "MTreeConfig.cpp"[4]

This file implements the "MTreeConfig"[4] class.

*/
#include <limits>
#include "MTreeConfig.h"

using namespace mtreeAlgebra;

/*
Initialize static members :

*/
bool MTreeConfigReg::initialized = false;
map<string, MTreeConfig> MTreeConfigReg::configs;
string MTreeConfigReg::defaultConfigName = "undef";


/*
Method ~getConfig~ :

*/
MTreeConfig
MTreeConfigReg::getConfig(const string& name)
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
bool
MTreeConfigReg::isDefined(const string& name)
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
Method ~initialize~ :

*/
void
MTreeConfigReg::initialize()
{
/*
Create node configs

*/
  NodeConfig defaultleafConfig(Leaf,
      leafPrio, minLeafEntries,
      maxLeafEntries, minLeafPages, maxLeafPages);

  NodeConfig defaultinternalConfig(Internal,
      internalPrio, minIntEntries,
      maxIntEntries, minIntPages, maxIntPages);

  NodeConfig leafConfig40(Leaf,
      leafPrio, minLeafEntries, 40, minLeafPages, maxLeafPages);

  NodeConfig internalConfig40(Internal,
      internalPrio, minIntEntries, 40, minIntPages, maxIntPages);

  NodeConfig leafConfig80(Leaf,
      leafPrio, minLeafEntries, 80, minLeafPages, maxLeafPages);

  NodeConfig internalConfig80(Internal,
      internalPrio, minIntEntries, 80, minIntPages, maxIntPages);

/*
Set default config

*/
  defaultConfigName = "mlbdistHP";

/*
Add config objects with unlimited entries per node.

*/
  configs["randomBal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      RANDOM, BALANCED);

  configs["mradBal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      m_RAD, BALANCED);

  configs["mmradBal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      mM_RAD, BALANCED);

  configs["mlbBal"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      M_LB_DIST, BALANCED);

  configs["randomHP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      RANDOM, GENERALIZED_HYPERPLANE);

  configs["mradHP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      m_RAD, GENERALIZED_HYPERPLANE);

  configs["mmradHP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      mM_RAD, GENERALIZED_HYPERPLANE);

  configs["mlbdistHP"] =  MTreeConfig(
      defaultleafConfig, defaultinternalConfig,
      M_LB_DIST, GENERALIZED_HYPERPLANE);

/*
Add config objects with max. 80 entries per node.

*/
  configs["randomBal80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      RANDOM, BALANCED);

  configs["mradBal80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      m_RAD, BALANCED);

  configs["mmradBal80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      mM_RAD, BALANCED);

  configs["mlbBal80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      M_LB_DIST, BALANCED);

  configs["randomHP80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      RANDOM, GENERALIZED_HYPERPLANE);

  configs["mradHP80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      m_RAD, GENERALIZED_HYPERPLANE);

  configs["mmradHP80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      mM_RAD, GENERALIZED_HYPERPLANE);

  configs["mlbdistHP80"] =  MTreeConfig(
      leafConfig80, internalConfig80,
      M_LB_DIST, GENERALIZED_HYPERPLANE);

/*
Add config objects with max. 40 entries per node.

*/
  configs["randomBal40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      RANDOM, BALANCED);

  configs["mradBal40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      m_RAD, BALANCED);

  configs["mmradBal40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      mM_RAD, BALANCED);

  configs["mlbBal40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      M_LB_DIST, BALANCED);

  configs["randomHP40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      RANDOM, GENERALIZED_HYPERPLANE);

  configs["mradHP40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      m_RAD, GENERALIZED_HYPERPLANE);

  configs["mmradHP40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      mM_RAD, GENERALIZED_HYPERPLANE);

  configs["mlbdistHP40"] =  MTreeConfig(
      leafConfig40, internalConfig40,
      M_LB_DIST, GENERALIZED_HYPERPLANE);

  initialized = true;
}
