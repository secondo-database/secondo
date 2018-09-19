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

import java.util.List;

import mol.datatypes.interval.Period;
import mol.datatypes.intime.Intime;
import mol.datatypes.spatial.Line;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.unit.spatial.UnitPointConst;
import mol.datatypes.unit.spatial.UnitPointLinear;
import mol.interfaces.interval.PeriodIF;
import mol.interfaces.intime.IntimeIF;
import mol.interfaces.moving.MovingPointIF;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.interfaces.unit.spatial.UnitPointIF;

/**
 * This class represents moving spatial objects of type 'MovingPoint'
 * 
 * @author Markus Fuessel
 */
public class MovingPoint extends MovingObject<UnitPointIF, PointIF> implements MovingPointIF {

   /**
    * The minimum projection bounding box in which the point moves
    */
   private RectangleIF objectPBB;

   /**
    * Basic constructor to create a empty 'MovingPoint' object
    * 
    * @param initialSize
    *           - used to initialize the internal used array structures
    */
   public MovingPoint(final int initialSize) {
      super(initialSize);
      objectPBB = new Rectangle();

      this.setDefined(true);
   }

   /**
    * Constructor for a 'MovingPoint' object.<br>
    * Creates the object out of the passed list of {@code Intime<PointIF>} objects.
    * 
    * @param intimePoints
    *           - list of {@code Intime<PointIF>} objects
    */
   public MovingPoint(final List<Intime<PointIF>> intimePoints) {
      this(intimePoints.size());

      int posLastPoint = intimePoints.size() - 1;

      for (int i = 0; i < posLastPoint; i++) {
         IntimeIF<PointIF> ip0 = intimePoints.get(i);
         IntimeIF<PointIF> ip1 = intimePoints.get(i + 1);

         boolean isLastPoint = (i + 1 == posLastPoint);

         Period p = new Period(ip0.getInstant(), ip1.getInstant(), true, isLastPoint);

         add(p, ip0.getValue(), ip1.getValue());
      }
   }

   /**
    * Constructor for a "constant" 'MovingPoint' with a maximum time period
    * 
    * @param point
    */
   public MovingPoint(final PointIF point) {
      this(1);
      add(new UnitPointConst(point));
      setDefined(point.isDefined());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#add(mol.datatypes.unit.UnitObject)
    */
   @Override
   public boolean add(final UnitPointIF unit) {

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
    * @see mol.datatypes.moving.MovingPointIF#add(mol.interfaces.interval.PeriodIF,
    * mol.interfaces.spatial.PointIF, mol.interfaces.spatial.PointIF)
    */
   @Override
   public boolean add(final PeriodIF period, final PointIF startPoint, final PointIF endPoint) {
      UnitPointIF upoint;

      if (startPoint.almostEqual(endPoint)) {
         upoint = new UnitPointConst(period, startPoint);

      } else {
         upoint = new UnitPointLinear(period, startPoint, endPoint);
      }

      return add(upoint);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingPointIF#trajectory()
    */
   @Override
   public Line trajectory() {
      Line line = new Line(true);
      int size = getNoUnits();

      for (int i = 0; i < size; i++) {

         UnitPointIF up = getUnit(i);

         PointIF p0 = up.getInitial();
         PointIF p1 = up.getFinal();

         if (!p0.almostEqual(p1)) {
            line.add(p0, p1);
         }

      }

      return line;
   }

   /**
    * Get the projection bounding box of the moving point
    * 
    * @return a rectangle, the minimum bounding box in which the point moves
    */
   @Override
   public RectangleIF getProjectionBoundingBox() {
      return objectPBB;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#getUndefinedUnitObject()
    */
   @Override
   protected UnitPointIF getUndefinedUnitObject() {
      return new UnitPointConst();
   }

   /*
    * 
    * 
    * /* (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#getUndefinedObject()
    */
   @Override
   protected PointIF getUndefinedObject() {
      return new Point();
   }

}
