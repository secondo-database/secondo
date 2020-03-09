/*

----
This file is part of SECONDO.

Copyright (C) 2020,
Faculty of Mathematics and Computer Science,
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


This files realizes a balanced dsitribution of slots to workers.


*/

#pragma once
#include <vector>
#include <cstdint>


/*

Computes a balanced assignment of slots to workers.
The argument slotsizes contains the sizes of the all slots,
noWorkers is the number of workers on that the slots will
be distributed.

The result will contain for each slot the assigned worker
starting with zero. 

The algorithms works as follows.
First, the slots are sorted by their size.
Then, we iterator over the slots. The next slot is assigned to that
worker having the smallest load. 

*/


std::vector<uint32_t> getMappingSimple(std::vector<size_t>& slotsizes,
                                       uint32_t noWorkers);


