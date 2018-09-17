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

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Halfsegment;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.time.TimeInstant;
import mol.datatypes.unit.UnitObject;
import mol.datatypes.unit.UnitObjectConst;
import mol.datatypes.unit.spatial.util.MovableSegment;

/**
 * This class represents 'UnitRegionConst' objects and is used for
 * 'MovingRegion' objects with a constant location over a certain period of
 * time.
 * 
 * @author Markus Fuessel
 */
public class UnitRegionConst extends UnitRegion {

   /**
    * Constant value for a constant 'UnitRegion' object
    */
   private final Region constRegion;

   /**
    * Constructor for an undefined 'UnitRegionConst' object
    */
   public UnitRegionConst() {
      this.constRegion = new Region(false);
   }

   /**
    * Constructor for an 'UnitRegionConst' object with a maximum Period
    */
   public UnitRegionConst(Region region) {
      super(Period.MAX);
      this.constRegion = region;
      setDefined(region.isDefined());
   }

   /**
    * Constructor to create a 'UnitRegion' object with a constant region value
    * 
    * @param period
    * @param region
    */
   public UnitRegionConst(final Period period, final Region region) {

      super(period);
      this.constRegion = Objects.requireNonNull(region, "'region' must not be null");
      setDefined(period.isDefined() && region.isDefined());

   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#atPeriod(mol.datatypes.interval.Period)
    */
   @Override
   public UnitRegionConst atPeriod(Period period) {
      Period newPeriod = this.getPeriod().intersection(period);

      if (!newPeriod.isDefined()) {
         return new UnitRegionConst();
      }

      return new UnitRegionConst(newPeriod, getConstRegion());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#equalValue(mol.datatypes.unit.UnitObject)
    */
   @Override
   public boolean equalValue(UnitObject<Region> otherUnitObject) {

      if (!(otherUnitObject instanceof UnitObjectConst<?>) || !otherUnitObject.isDefined()) {
         return false;
      }

      UnitObjectConst<Region> otherUnitRegionConstant = (UnitObjectConst<Region>) otherUnitObject;

      return constRegion.equals(otherUnitRegionConstant.getValue());

   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.unit.UnitObject#finalEqualToInitialValue(mol.datatypes.unit.
    * UnitObject)
    */
   @Override
   public boolean finalEqualToInitialValue(UnitObject<Region> otherUnitObject) {
      return equalValue(otherUnitObject);
   }

   /**
    * Get the projection bounding box for this 'UnitRegionConst'
    */
   @Override
   public Rectangle getProjectionBoundingBox() {
      return constRegion.getBoundingBox();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#getValue(mol.datatypes.time.TimeInstant)
    */
   @Override
   public Region getValue(TimeInstant instant) {
      Period period = getPeriod();

      if (isDefined() && period.contains(instant)) {
         return getConstRegion();
      } else {
         return new Region(false);
      }

   }

   /**
    * Getter for the constant value object
    * 
    * @return the value
    */
   protected Region getConstRegion() {
      return constRegion;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#getInitial()
    */
   @Override
   public Region getInitial() {
      return getConstRegion();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#getFinal()
    */
   @Override
   public Region getFinal() {
      return getConstRegion();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.UnitRegion#getMovingSegments()
    */
   @Override
   public List<MovableSegment> getMovingSegments() {
      List<MovableSegment> movingsegments = new ArrayList<>();

      for (Halfsegment halfsegment : constRegion.getHalfsegments()) {
         if (halfsegment.isLeftDominating()) {

            movingsegments.add(new MovableSegment(halfsegment));
         }
      }

      return movingsegments;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.UnitRegion#getNoMovingFaces()
    */
   @Override
   public int getNoMovingFaces() {
      return constRegion.getNoComponents();
   }

}
