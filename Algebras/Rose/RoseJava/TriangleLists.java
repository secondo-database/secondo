//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

class TriangleLists {
  //this is a supportive class for computeIntersectionTriangles in class Triangle
  //computeIntersectionTriangles returns three list of triangles which result from
  //intersecting two triangles. Resulting triangles beloning only to the first or
  //second triangle can be found in this.first (this.second resp.)
  //triangles beloning to both triangles can be found in this.both.

  //members
  TriList first = new TriList();
  TriList second = new TriList();
  TriList both = new TriList();

  //constructors

  //methods

}//end class TriangleLists
