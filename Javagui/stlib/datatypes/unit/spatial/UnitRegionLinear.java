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
package stlib.datatypes.unit.spatial;

import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.spatial.Region;
import stlib.datatypes.spatial.util.Rectangle;
import stlib.datatypes.unit.UnitObject;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.RegionIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitObjectIF;
import stlib.interfaces.unit.spatial.UnitRegionIF;
import stlib.interfaces.unit.spatial.util.MovableFaceIF;
import stlib.interfaces.unit.spatial.util.MovableSegmentIF;

/**
 * This class represents 'UnitRegionLinear' objects
 * <p>
 * It is used to represent a continuous linear movement of a 'Region' during a
 * certain period of time.
 * 
 * @author Markus Fuessel
 */
public class UnitRegionLinear extends UnitObject<RegionIF> implements UnitRegionIF {

   /**
    * List of 'MovableFace' objects
    */
   private final List<MovableFaceIF> movingFaces;

   /**
    * The minimum bounding box in which this 'UnitRegionLinear' moves and expands
    */
   private RectangleIF objectPBB;

   /**
    * Constructor for an undefined 'UnitRegionLinear' object.
    */
   public UnitRegionLinear() {
      this.movingFaces = new ArrayList<>();
      this.objectPBB = new Rectangle();
   }

   /**
    * Constructor for an empty 'UnitRegionLinear' object.
    * 
    * @param period
    *           - time period for which this unit is defined
    */
   public UnitRegionLinear(final PeriodIF period) {
      super(period);

      this.movingFaces = new ArrayList<>();
      this.objectPBB = new Rectangle();
   }

   /**
    * Constructor for a 'UnitRegionLinear' object with one 'MovableFace'.<br>
    * Creates the object out of the passed 'MovableFace' object.<br>
    * This object would be only set defined if the 'MovableFace' object where added
    * successful.
    * 
    * @param period
    *           - time period for which this unit is defined
    * 
    * @param movingFace
    *           - a 'MovableFace' object
    */
   public UnitRegionLinear(final PeriodIF period, final MovableFaceIF movingFace) {
      this(period);

      setDefined(period.isDefined() && add(movingFace));

   }

   /**
    * Constructor for a 'UnitRegionLinear' object.<br>
    * Creates the object out of the passed list of {@code MovableFace} objects.<br>
    * This object would be only set defined if all 'MovableFace' objects where
    * added successful.
    * 
    * @param period
    *           - time period for which this unit is defined
    * 
    * @param movingFaces
    *           - List of 'MovableFace' objects
    */
   public UnitRegionLinear(final PeriodIF period, final List<MovableFaceIF> movingFaces) {

      this(period);

      boolean success = true;

      for (MovableFaceIF movingFace : movingFaces) {
         if (!add(movingFace)) {
            success = false;
            break;
         }
      }

      setDefined(period.isDefined() && success);
   }

   /**
    * Add a 'MovableFace' object to this 'UnitRegionLinear'.<br>
    * Updates the projection bounding box of this unit object.
    * 
    * @param movingFace
    *           - the 'MovableFace' to add
    * 
    * @return true if adding was successful, false otherwise
    */
   public boolean add(MovableFaceIF movingFace) {
      if (movingFaces.add(movingFace)) {
         objectPBB = objectPBB.merge(movingFace.getProjectionBoundingBox());

         return true;
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getValue(stlib.interfaces.time.
    * TimeInstantIF)
    */
   @Override
   public RegionIF getValue(TimeInstantIF instant) {
      return getValue(instant, false);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getValue(stlib.interfaces.time.
    * TimeInstantIF, boolean)
    */
   @Override
   public RegionIF getValue(TimeInstantIF instant, final boolean ignoreClosedFlags) {
      PeriodIF period = getPeriod();

      if (isDefined() && period.contains(instant, ignoreClosedFlags)) {
         RegionIF region = new Region(true);

         for (MovableFaceIF movingFace : movingFaces) {
            region.add(movingFace.getValue(period, instant));
         }

         region.setDefined(!region.isEmpty());

         return region;

      } else {
         return new Region(false);
      }
   }

   /**
    * Not implementet yet
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#atPeriod(stlib.interfaces.interval.PeriodIF)
    */
   @Override
   public UnitRegionLinear atPeriod(PeriodIF period) {

      // TODO implement
      return new UnitRegionLinear();

   }

   /**
    * Not implemented for 'UnitRegionLinear' objects yet.
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#equalValue(stlib.interfaces.unit.UnitObjectIF)
    */
   @Override
   public boolean equalValue(UnitObjectIF<RegionIF> otherUnitObject) {

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.UnitObjectIF#finalEqualToInitialValue(stlib.interfaces.
    * unit. UnitObjectIF)
    */
   @Override
   public boolean finalEqualToInitialValue(UnitObjectIF<RegionIF> otherUnitObject) {
      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getInitial()
    */
   @Override
   public RegionIF getInitial() {
      Region region = new Region(true);

      for (MovableFaceIF movingFace : movingFaces) {
         region.add(movingFace.getInitial());
      }

      region.setDefined(!region.isEmpty());

      return region;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getFinal()
    */
   @Override
   public Region getFinal() {
      Region region = new Region(true);

      for (MovableFaceIF movingFace : movingFaces) {
         region.add(movingFace.getFinal());
      }

      region.setDefined(!region.isEmpty());

      return region;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.UnitRegionIF#getMovingSegments()
    */
   @Override
   public List<MovableSegmentIF> getMovingSegments() {
      List<MovableSegmentIF> movingsegments = new ArrayList<>();

      for (MovableFaceIF movingFace : movingFaces) {
         movingsegments.addAll(movingFace.getMovingSegments());
      }

      return movingsegments;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.features.MovableSpatial#getProjectionBoundingBox()
    */
   @Override
   public RectangleIF getProjectionBoundingBox() {
      return objectPBB;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.UnitRegionIF#getNoMovingFaces()
    */
   @Override
   public int getNoMovingFaces() {
      return movingFaces.size();
   }

}
