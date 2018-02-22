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

1 Headerfile "MTreeConfig.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "MTreeConfigReg"[4] class, which provides a set of configurations. Each configuration is identified with a unique name and sets the used split policy as well as the min/max count of entries and max count of pages per node.

All avaliable config objects are defined in the initialize function (file "MTreeConfig.cpp"[4]) and could be selected with the createmtree2 or createmtree3 operator (createmtree uses the default values).

1.1 Includes and defines

*/
#ifndef __MTREE_CONFIG_H__
#define __MTREE_CONFIG_H__

#include "MTreeAlgebra.h"
#include "Algebras/GeneralTree/GeneralTreeAlgebra.h"

namespace mtreeAlgebra
{

// name of the default config
const std::string CONFIG_DEFAULT("default");

/********************************************************************
1.1 Struct ~MTreeConfig~

********************************************************************/
struct MTreeConfig
{
/*
Config objects for all node types.

*/
   gtree::NodeConfig leafNodeConfig;
   gtree::NodeConfig internalNodeConfig;

/*
This parameters contain the promote and partition functions, which should be used.

*/
   PROMOTE promoteFun;
   PARTITION partitionFun;

/*
Constructor (creates object with default values).

*/
   MTreeConfig()
        : leafNodeConfig(
                LEAF, leafPrio, minLeafEntries,
                maxLeafEntries, maxLeafPages, leafCacheable),
          internalNodeConfig(
                INTERNAL, internalPrio, minIntEntries,
                maxIntEntries, maxIntPages, internalCacheable),
          promoteFun(defaultPromoteFun),
          partitionFun(defaultPartitionFun)
    {}

/*
Constructor (creates objects with the given parameters).

*/
    MTreeConfig(
            gtree::NodeConfig _leafNodeConfig,
            gtree::NodeConfig _internalNodeConfig,
            PROMOTE _promoteFun,
            PARTITION _partitionFun)
        : leafNodeConfig(_leafNodeConfig),
          internalNodeConfig(_internalNodeConfig),
          promoteFun(_promoteFun),
          partitionFun(_partitionFun)
    {}
}; // struct MTreeConfig



/********************************************************************
1.1 Class "MTreeConfigReg"[4]

********************************************************************/
class MTreeConfigReg
{

public:
/*
This method returns the specified "MTreeConfig"[4] object. If no such object could be found, the method returns a new object with default values.

*/
    static MTreeConfig getConfig(const std::string &name);

/*
Returns true, if the specified "MTreeConfig"[4] object is defiend.

*/
    static bool isDefined(const std::string &name);

/*
Returns a string with the names of all defined config objects.

*/
    static std::string definedNames();

/*
Registeres all config objects.

*/
    static void initialize();

private:
    static std::map<std::string, MTreeConfig> configs;
    static bool initialized;
}; // class MTreeConfigReg

} // namespace mtreeAlgebra
#endif // #ifdef __MTREE_CONFIG_H__
