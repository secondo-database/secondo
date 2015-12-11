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

1 Implementation file "XTreeConfig.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "XTreeConfig.h"

using namespace xtreeAlgebra;
using namespace std;
using namespace gtree;

/*
Initialize static members :

*/
bool XTreeConfigReg::initialized = false;
map<string, XTreeConfig> XTreeConfigReg::configs;


/*
Method ~getConfig~ :

*/
XTreeConfig XTreeConfigReg::getConfig(const string &name)
{
  if (!initialized)
    initialize();

  map< string, XTreeConfig >::iterator pos =
      configs.find(name);

  if (pos != configs.end())
    return pos->second;
  else
    return XTreeConfig();
}

/*
Method ~isDefined~ :

*/
bool XTreeConfigReg::isDefined(const string &name)
{
  if (!initialized)
    initialize();

  map< string, XTreeConfig >::iterator pos =
      configs.find(name);

  if (pos != configs.end())
    return true;
  else
    return false;
}

/*
Method ~definedNames~ :

*/
string XTreeConfigReg::definedNames()
{
    if(!initialized)
        initialize();

    ostringstream result;
    map<string, XTreeConfig>::iterator iter = configs.begin();
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
XTreeConfigReg::initialize()
{
  NodeConfig defaultleafConfig(LEAF, leafPrio, minLeafEntries,
        maxLeafEntries, maxLeafPages, leafCacheable);

  NodeConfig defaultinternalConfig(INTERNAL, internalPrio,
        minIntEntries, maxIntEntries, maxIntPages, internalCacheable);

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
  configs[CONFIG_DEFAULT] =  XTreeConfig(
      defaultleafConfig, defaultinternalConfig);

/*
Add config objects with limited entries per node.

*/
  configs["limit20e"] =  XTreeConfig(
      leafConfig20, internalConfig20);

  configs["limit40e"] =  XTreeConfig(
      leafConfig40, internalConfig40);

  configs["limit80e"] =  XTreeConfig(
      leafConfig80, internalConfig80);

  initialized = true;
}
