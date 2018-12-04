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

package mmdb.data.attributes.unit;

import mmdb.data.attributes.util.SpatialObjects.Segment;
import mmdb.data.attributes.util.TemporalObjects.Period;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.unit.spatial.UnitPointLinear;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitObjectIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * Object representation for database attributes of type 'upoint'.
 *
 * @author Alexander Castor
 */
/**
 * @author Markus Fuessel
 *
 */
public class AttributeUpoint extends UnitAttribute<Segment> implements UnitPointIF {

   private UnitPointIF upoint;

   /*
    * (non-Javadoc)
    * 
    * @see mmdb.data.attributes.instant.InstantAttribute#injectValue()
    */
   @Override
   protected Segment getUnitObject() {
      return new Segment();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mmdb.data.attributes.unit.UnitAttribute#setValue(mmdb.data.attributes.
    * MemoryAttribute)
    */
   @Override
   public void setValue(Segment value) {
      super.setValue(value);

      Period periodMMDB = getPeriodMMDB();

      PointIF startPoint = new Point(value.getxValue1(), value.getyValue1());
      PointIF endPoint = new Point(value.getxValue2(), value.getyValue2());

      upoint = new UnitPointLinear(periodMMDB.toPeriodIF(), startPoint, endPoint);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.UnitObjectIF#before(stlib.interfaces.unit.UnitObjectIF)
    */
   @Override
   public boolean before(UnitObjectIF<?> otherUnitObject) {
      return upoint.before(otherUnitObject);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.UnitObjectIF#periodEndsWithin(stlib.interfaces.unit.
    * UnitObjectIF)
    */
   @Override
   public boolean periodEndsWithin(UnitObjectIF<?> otherUnitObject) {
      return upoint.periodEndsWithin(otherUnitObject);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#setPeriod(stlib.interfaces.interval.
    * PeriodIF)
    */
   @Override
   public void setPeriod(PeriodIF period) {
      upoint.setPeriod(period);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getValue(stlib.interfaces.time.
    * TimeInstantIF)
    */
   @Override
   public PointIF getValue(TimeInstantIF instant) {
      return upoint.getValue(instant);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getValue(stlib.interfaces.time.
    * TimeInstantIF, boolean)
    */
   @Override
   public PointIF getValue(TimeInstantIF instant, boolean ignoreClosedFlags) {
      return upoint.getValue(instant, ignoreClosedFlags);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#contains(stlib.interfaces.time.
    * TimeInstantIF)
    */
   @Override
   public boolean contains(TimeInstantIF instant) {
      return upoint.contains(instant);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getInitial()
    */
   @Override
   public PointIF getInitial() {
      return upoint.getInitial();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getFinal()
    */
   @Override
   public PointIF getFinal() {
      return upoint.getFinal();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#equalValue(stlib.interfaces.unit.
    * UnitObjectIF)
    */
   @Override
   public boolean equalValue(UnitObjectIF<PointIF> otherUnitObject) {
      return upoint.equalValue(otherUnitObject);
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(UnitObjectIF<PointIF> o) {
      return upoint.compareTo(o);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.GeneralTypeIF#isDefined()
    */
   @Override
   public boolean isDefined() {
      return upoint.isDefined();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.GeneralTypeIF#setDefined(boolean)
    */
   @Override
   public void setDefined(boolean defined) {
      upoint.setDefined(defined);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.features.MovableSpatial#getProjectionBoundingBox()
    */
   @Override
   public RectangleIF getProjectionBoundingBox() {
      return upoint.getProjectionBoundingBox();
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.spatial.UnitPointIF#finalEqualToInitialValue(stlib.
    * interfaces.unit.UnitObjectIF)
    */
   @Override
   public boolean finalEqualToInitialValue(UnitObjectIF<PointIF> otherUnitObject) {
      return upoint.finalEqualToInitialValue(otherUnitObject);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.spatial.UnitPointIF#atPeriod(stlib.interfaces.interval.
    * PeriodIF)
    */
   @Override
   public UnitPointIF atPeriod(PeriodIF period) {
      return upoint.atPeriod(period);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getPeriod()
    */
   @Override
   public PeriodIF getPeriod() {
      return upoint.getPeriod();
   }

}
