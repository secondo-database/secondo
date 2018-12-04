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

package mmdb.data.attributes.moving;

import java.util.List;

import mmdb.data.attributes.unit.AttributeUpoint;
import stlib.datatypes.moving.MovingPoint;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.range.PeriodsIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * Object representation for database attributes of type 'mpoint'.
 *
 * @author Alexander Castor
 */
public class AttributeMpoint extends MovingAttribute<AttributeUpoint> implements MovingPointIF {

   private MovingPointIF mpoint;

   /**
    * Constructor
    */
   public AttributeMpoint() {
      mpoint = new MovingPoint(0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mmdb.data.attributes.instant.InstantAttribute#injectValue()
    */
   @Override
   protected AttributeUpoint getMovableObject() {

      return new AttributeUpoint();
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mmdb.data.attributes.moving.MovingAttribute#addUnit(mmdb.data.attributes.unit
    * .UnitAttribute)
    */
   @Override
   public void addUnit(AttributeUpoint unit) {
      super.addUnit(unit);
      add(unit);
   }

   @Override
   public boolean add(UnitPointIF newUnit) {
      return mpoint.add(newUnit);
   }

   @Override
   public int getNoUnits() {
      return mpoint.getNoUnits();
   }

   @Override
   public PointIF getValue(TimeInstantIF instant) {
      return mpoint.getValue(instant);
   }

   @Override
   public UnitPointIF getUnit(TimeInstantIF instant) {
      return mpoint.getUnit(instant);
   }

   @Override
   public UnitPointIF getUnit(int position) {
      return mpoint.getUnit(position);
   }

   @Override
   public int getUnitPosition(TimeInstantIF instant) {
      return mpoint.getUnitPosition(instant);
   }

   @Override
   public boolean isClosed() {
      return mpoint.isClosed();
   }

   @Override
   public boolean isDefined() {
      return mpoint.isDefined();
   }

   @Override
   public void setDefined(boolean defined) {
      mpoint.setDefined(defined);
   }

   @Override
   public RectangleIF getProjectionBoundingBox() {
      return mpoint.getProjectionBoundingBox();
   }

   @Override
   public PeriodsIF getPeriods() {
      return mpoint.getPeriods();
   }

   @Override
   public boolean add(PeriodIF period, PointIF startPoint, PointIF endPoint) {
      return mpoint.add(period, startPoint, endPoint);
   }

   @Override
   public List<UnitPointIF> getUnits() {
      return mpoint.getUnits();
   }
}