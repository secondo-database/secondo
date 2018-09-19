//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
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
package mol.interfaces.spatial.util;

import java.util.List;

import mol.interfaces.spatial.LineIF;

/**
 * Interface for cycle line objects
 * 
 * @author Markus Fuessel
 */
public interface CycleIF extends LineIF {

   /**
    * Set the 'Cycle' object by passing a list of 'SegmentIF' objects.<br>
    * To create a valid 'Cycle' object the passed list have to contain three
    * segments at least. Already existing segments in this 'Cycle' will be
    * replaced.
    * 
    * @param segments
    *           - List of segments
    */
   boolean setCycleBySegmentList(List<SegmentIF> segments);

}