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

1 Headerfile "MTreeConfig.h"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "MTreeConfigReg"[4] class, which provides a set of configurations. Each configuration is identified with a unique name and sets the used split policy by defining the promote and split function, that should be used, as well as the gtaf::NodeConfig objects for internal and leaf nodes, which sets the min/max count entries and pages per node.

All avaliable config objects are defined in the initialize function (file "MTreeConfig.cpp"[4]) and could be set, when using the createmtree2 or createmtree3 operator.

1.1 Includes and defines

*/
#ifndef MTREE_CONFIG_H
#define MTREE_CONFIG_H

#include <string>
#include <map>
#include "MTreeSplitpol.h"
#include "MTreeAlgebra.h"

namespace mtreeAlgebra
{

/*
1.1 Struct ~MTreeConfig~ :

*/
struct MTreeConfig
{
/*
Config objects for all node types.

*/
  NodeConfig leafNodeConfig;
  NodeConfig internalNodeConfig;

/*
This parameters contain the promote and partition functions, which should be used.

*/
  PROMOTE promoteFun;
  PARTITION partitionFun;

/*
Constructor (creates object with default values).

*/
  MTreeConfig()
  : leafNodeConfig(Leaf, 0, 3),
    internalNodeConfig(Internal, 1, 3),
    promoteFun(RANDOM),
    partitionFun(BALANCED)
  {}

/*
Constructor (creates objects with the given parameters).

*/
  MTreeConfig(NodeConfig _leafNodeConfig,
              NodeConfig _internalNodeConfig,
              PROMOTE _promoteFun,
              PARTITION _partitionFun)
  : leafNodeConfig(_leafNodeConfig),
    internalNodeConfig(_internalNodeConfig),
    promoteFun(_promoteFun),
    partitionFun(_partitionFun)
  {}
};

/*
1.1 Class MTreeConfigReg:

*/
class MTreeConfigReg
{
public:
/*
This method returns the specified "MTreeConfig"[4] object. If no such object could be found, the method returns a new object with default values.

*/
    static MTreeConfig getConfig(const string& name);

/*
Returns true, if the specified "MTreeConfig"[4] object is defiend.

*/
    static bool isDefined(const string& name);

/*
Registeres all "MTreeConfig" objects.

*/
    static void initialize();

private:
    static map<string, MTreeConfig> configs;
    static bool initialized;
};

} // namespace mtreeAlgebra

#endif
