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

3.5 Configuring m-trees

December 2007, Mirko Dibbert

3.5.1 Overview

TODO

3.5.2 Definition part (file: MTreeConfig.h)

*/
#include <string>
#include <map>
#include "MTSplitpol.h"

namespace MT
{

/*
Struct ~MTreeConfig~ :

This struct contains some config parameter, which allows it to optimize the
mtree datastructure.

*/
struct MTreeConfig
{
  unsigned maxNodeEntries;
/*
This parameter adjust the maximum count of entries, wich should be stored
within a m-tree node when the associated metric is used.

A limiting value could make sense, if the DistData values are very short and
the cost of distance computations is (much) higher than the cost of the
additional I/O access duo to the growing count of nodes.

*/

  PROMOTE promoteFun;
  PARTITION partitionFun;
/*
This parameters contain the promote and partition functions, which should be
used.

*/

  MTreeConfig()
  : maxNodeEntries ( 200 ),
    promoteFun( RANDOM ),
    partitionFun( BALANCED ) {}
/*
Constructor (creates object with default values).

*/

  MTreeConfig( unsigned maxNodeEntries_,
               PROMOTE promoteFun_,
               PARTITION partitionFun_ )
  : maxNodeEntries ( (maxNodeEntries_ < 2) ? 2 : maxNodeEntries_ ),
    promoteFun( promoteFun_ ),
    partitionFun( partitionFun_ ) {}
/*
Constructor (creates objects with the given parameters).

*/
};

/*
Class ~MTreeConfigReg~ :

TODO insert description

*/
class MTreeConfigReg
{
  static map< string, MTreeConfig > mTreeConfig_map;
  static bool initialized;

public:
  static MTreeConfig getMTreeConfig( const string& name );
/*
This method returns the MTreeConfig object, that belongs to the specified
metric. If no such object is registered, the method returns a new object with
default values.

*/

  static void initialize();
/*
This method registeres all defined distance functions.

*/

  static void registerMTreeConfig( const string& name,
                                   const MTreeConfig& config );
/*
This method is used to register a "MTreeConfig"[4] object for the
associatedmetric.

*/
};

} // namespace
