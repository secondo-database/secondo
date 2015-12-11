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

1 Headerfile "XTreeConfig.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "XTreeConfigReg"[4] class, which provides a set of configurations. Each configuration is identified with a unique name and sets the min/max count of entries and max count of pages per node.

All avaliable config objects are defined in the initialize function (file "MTreeConfig.cpp"[4]) and could be selected with the creatextree2 operator (creatextree uses the default values).

1.1 Includes and defines

*/
#ifndef XTREE_CONFIG_H
#define XTREE_CONFIG_H

#include "XTreeAlgebra.h"

namespace xtreeAlgebra
{


// name of the default config
const std::string CONFIG_DEFAULT("default");

/********************************************************************
1.1 Struct ~XTreeConfig~

********************************************************************/
struct XTreeConfig
{
/*
Config objects for all node types.

*/
    gtree::NodeConfig leafNodeConfig;
    gtree::NodeConfig internalNodeConfig;

/*
Constructor (creates object with default values).

*/
    XTreeConfig()
        : leafNodeConfig(
                LEAF, leafPrio, minLeafEntries,
                maxLeafEntries, maxLeafPages, leafCacheable),
          internalNodeConfig(
                INTERNAL, internalPrio, minIntEntries,
                maxIntEntries, maxIntPages, internalCacheable)
  {}

/*
Constructor (creates objects with the given parameters).

*/
    XTreeConfig(
            gtree::NodeConfig _leafNodeConfig,
            gtree::NodeConfig _internalNodeConfig)
        : leafNodeConfig(_leafNodeConfig),
          internalNodeConfig(_internalNodeConfig)
  {}
}; // struct XTreeConfig



/********************************************************************
1.1 Class "XTreeConfigReg"[4]

********************************************************************/
class XTreeConfigReg
{

public:
/*
This method returns the specified "XTreeConfig"[4] object. If no such object could be found, the method returns a new object with default values.

*/
    static XTreeConfig getConfig(const std::string &name);

/*
Returns true, if the specified "XTreeConfig"[4] object is defiend.

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
    static std::map<std::string, XTreeConfig> configs;
    static bool initialized;
}; // class XTreeConfigReg

} // namespace xtreeAlgebra

#endif
