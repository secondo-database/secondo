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
package mol.datatypes.unit.spatial;

import java.util.List;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Region;
import mol.datatypes.unit.spatial.util.MovableSegment;

/**
 * Abstract base class for 'UnitRegion' objects
 * 
 * @author Markus Fuessel
 */
public abstract class UnitRegion extends UnitSpatial<Region> {

   /**
    * Constructor for an undefined 'UnitRegion' object<br>
    * Required for subclasses
    */
   protected UnitRegion() {
   }

   /**
    * Base constructor for a 'UnitRegion' object<br>
    * Required for subclasses
    * 
    * @param period,
    *           the valid time period of this unit
    */
   protected UnitRegion(Period period) {
      super(period);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#atPeriod(mol.datatypes.interval.Period)
    */
   @Override
   public abstract UnitRegion atPeriod(Period period);

   /**
    * Get the number of moving faces in this 'UnitRegion' object
    * 
    * @return number of moving faces
    */
   public abstract int getNoMovingFaces();

   /**
    * Get the all moving segments
    * 
    * @return list of 'MovableSegment' objects
    */
   public abstract List<MovableSegment> getMovingSegments();
}
