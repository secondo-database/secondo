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
package stlib.datatypes.moving;

import stlib.datatypes.spatial.Region;
import stlib.datatypes.spatial.util.Rectangle;
import stlib.datatypes.unit.spatial.UnitRegionConst;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.moving.MovingRegionIF;
import stlib.interfaces.spatial.RegionIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.unit.spatial.UnitRegionIF;

/**
 * This class represents moving spatial objects of type 'MovingRegion'
 * 
 * @author Markus Fuessel
 */
public class MovingRegion extends MovingObject<UnitRegionIF, RegionIF> implements MovingRegionIF {

   /**
    * The minimum bounding box in which this 'MovingRegion' moves and expands
    */
   private RectangleIF objectPBB;

   /**
    * Basic constructor to create a empty 'MovingRegion' object
    * 
    * @param initialSize
    *           - used to initialize the internal used array structures
    */
   public MovingRegion(final int initialSize) {
      super(initialSize);
      objectPBB = new Rectangle();

      this.setDefined(true);
   }

   /**
    * Constructor for a "constant" 'MovingPoint' with a maximum time period
    * 
    * @param point
    */
   public MovingRegion(final RegionIF region) {
      this(1);
      add(new UnitRegionConst(region));
      setDefined(region.isDefined());
   }

   /**
    * Basic constructor to create a 'MovingRegion' object with one constant region
    * 
    * @param period
    *           - time period this constant region is defined
    * @param region
    *           - the region
    */
   public MovingRegion(final PeriodIF period, final RegionIF region) {
      this(1);
      setDefined(add(period, region));
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.moving.MovingRegionIF#add(stlib.interfaces.unit.spatial.
    * UnitRegionIF)
    */
   @Override
   public boolean add(UnitRegionIF unit) {
      if (super.add(unit)) {
         objectPBB = objectPBB.merge(unit.getProjectionBoundingBox());

         return true;

      } else {
         return false;
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingRegionIF#add(stlib.interfaces.interval.
    * PeriodIF, stlib.interfaces.spatial.RegionIF)
    */
   @Override
   public boolean add(final PeriodIF period, final RegionIF region) {

      return add(new UnitRegionConst(period, region));

   }

   /**
    * Get the projection bounding box of this 'MovingRegion'
    * 
    * @return a rectangle, the minimum bounding box in which the region moves
    *         and/or expands
    */
   public RectangleIF getProjectionBoundingBox() {
      return objectPBB;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObject#getUndefinedUnitObject()
    */
   @Override
   protected UnitRegionIF getUndefinedUnitObject() {
      return new UnitRegionConst();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObject#getUndefinedObject()
    */
   @Override
   protected RegionIF getUndefinedObject() {
      return new Region(false);
   }

}
