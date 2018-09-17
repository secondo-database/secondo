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
package mol.datatypes.moving;

import mol.datatypes.features.Spatial;
import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.unit.spatial.UnitRegion;
import mol.datatypes.unit.spatial.UnitRegionConst;

/**
 * This class represents moving spatial objects of type 'MovingRegion'
 * 
 * @author Markus Fuessel
 */
public class MovingRegion extends MovingObject<UnitRegion, Region> implements Spatial {

   /**
    * The minimum bounding box in which this 'MovingRegion' moves and expands
    */
   private Rectangle objectPBB;

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
   public MovingRegion(final Region region) {
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
   public MovingRegion(final Period period, final Region region) {
      this(1);
      setDefined(add(period, region));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#add(mol.datatypes.unit.UnitObject)
    */
   @Override
   public boolean add(UnitRegion unit) {
      if (super.add(unit)) {
         objectPBB = objectPBB.merge(unit.getProjectionBoundingBox());

         return true;

      } else {
         return false;
      }
   }

   /**
    * Append this 'MovingRegion' object by a further movement section defined by
    * the passed values.<br>
    * Creates an constant region unit object and append this to this object.
    * 
    * @param period
    *           - time period of this movement section
    * @param region
    *           - the region
    * 
    * @return true if the adding was successful, false otherwise
    */
   public boolean add(final Period period, final Region region) {

      return add(new UnitRegionConst(period, region));

   }

   /**
    * Get the projection bounding box of this 'MovingRegion'
    * 
    * @return a rectangle, the minimum bounding box in which the region moves
    *         and/or expands
    */
   public Rectangle getProjectionBoundingBox() {
      return objectPBB;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.features.Spatial#isEmpty()
    */
   @Override
   public boolean isEmpty() {
      return getNoUnits() == 0;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.features.Spatial#getBoundingBox()
    */
   @Override
   public Rectangle getBoundingBox() {
      return getProjectionBoundingBox();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#getUndefinedUnitObject()
    */
   @Override
   protected UnitRegion getUndefinedUnitObject() {
      return new UnitRegionConst();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#getUndefinedObject()
    */
   @Override
   protected Region getUndefinedObject() {
      return new Region(false);
   }

}
