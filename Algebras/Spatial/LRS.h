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

*/




#ifndef LRS_H
#define LRS_H

/*
6 Struct LRS

This struct implements the Linear Referencing System (LRS) ordering for lines. It basicaly contains
a position to the half segment in the line and its offset in the LRS. A line value will contain an
array ordered by these positions.

*/
struct LRS
{
  LRS() {}

  LRS( double lrsPos, int hsPos ):
  lrsPos( lrsPos ), hsPos( hsPos )
  {}

  LRS( const LRS& lrs ):
  lrsPos( lrs.lrsPos ), hsPos( lrs.hsPos )
  {}

  LRS& operator=( const LRS& lrs )
  { lrsPos = lrs.lrsPos; hsPos = lrs.hsPos; return *this; }

  double lrsPos;
  int hsPos;
};

int LRSCompare( const void *a, const void *b );


#endif

