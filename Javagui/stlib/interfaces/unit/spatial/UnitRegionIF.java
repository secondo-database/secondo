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
package stlib.interfaces.unit.spatial;

import java.util.List;

import stlib.interfaces.features.MovableSpatial;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.RegionIF;
import stlib.interfaces.unit.UnitObjectIF;
import stlib.interfaces.unit.spatial.util.MovableSegmentIF;

/**
 * Interface that should be provided by UnitRegionIF objects
 * 
 * @author Markus Fuessel
 */
public interface UnitRegionIF extends UnitObjectIF<RegionIF>, MovableSpatial {

   /*
    * (non-Javadoc)
    * 
    * @see stlib.datatypes.unit.UnitObject#atPeriod(stlib.datatypes.interval.Period)
    */
   UnitRegionIF atPeriod(PeriodIF period);

   /**
    * Get the number of moving faces in this 'UnitRegion' object
    * 
    * @return number of moving faces
    */
   int getNoMovingFaces();

   /**
    * Get the all moving segments
    * 
    * @return list of 'MovableSegment' objects
    */
   List<MovableSegmentIF> getMovingSegments();

}