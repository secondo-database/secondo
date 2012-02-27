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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

1 Struct ~AttrType~

The following type definition indicates the structure of the ~attr~ value associated with
half segments. This attribute is utilized only when we are handling regions. In line values
this attibute is ignored.

*/
#ifndef ATTRTYPE_H
#define ATTRTYPE_H


struct AttrType
{
  inline AttrType() { }

  explicit inline AttrType( int dummy ) :
    faceno(-999999),
    cycleno(-999999),
    edgeno(-999999),
    coverageno(-999999),
    insideAbove(false),
    partnerno(-999999){ }

/*
The simple constructor.

*/
  inline AttrType( const AttrType& at ):
  faceno( at.faceno ),
  cycleno( at.cycleno ),
  edgeno( at.edgeno ),
  coverageno( at.coverageno ),
  insideAbove( at.insideAbove ),
  partnerno( at.partnerno )
  {}
/*
The copy constructor.

*/
  inline AttrType& operator=( const AttrType& at )
  {
    faceno = at.faceno;
    cycleno = at.cycleno;
    edgeno = at.edgeno;
    coverageno = at.coverageno;
    insideAbove = at.insideAbove;
    partnerno = at.partnerno;
    return *this;
  }
/*
Redefinition of the assignement operator.

6.1 Attributes

*/
  int faceno;
/*
The face identifier

*/
  int cycleno;
/*
The cycle identifier

*/
  int edgeno;
/*
The edge (segment) identifier

*/
  int coverageno;
/*
Used for fast spatial scan of the inside[_]pr algorithm

*/
  bool insideAbove;
/*
Indicates whether the region's area is above or left of its segment

*/
  int partnerno;
/*
Stores the position of the partner half segment in half segment ordered array

*/
};

#endif
