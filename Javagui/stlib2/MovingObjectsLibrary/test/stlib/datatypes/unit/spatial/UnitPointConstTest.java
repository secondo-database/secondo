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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;

import org.junit.BeforeClass;
import org.junit.Test;

import stlib.datatypes.interval.Period;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.time.TimeInstant;
import stlib.datatypes.unit.spatial.UnitPointConst;
import stlib.datatypes.unit.spatial.UnitPointLinear;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * Tests for the 'UnitPointConst' class
 * 
 * @author Markus Fuessel
 */
public class UnitPointConstTest {

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Test
   public void testUnitPointConst_UndefinedObject() {
      UnitPointIF undefinedUPoint = new UnitPointConst();

      assertFalse(undefinedUPoint.isDefined());
   }

   @Test
   public void testGetValue_AtValidInstant() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      Point point = new Point(2.0d, 3.0d);
      UnitPointIF upoint = new UnitPointConst(period, point);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-05 00:00:00:000");
      TimeInstantIF instant3 = new TimeInstant("2018-01-09 23:59:59:999");

      assertEquals(point, upoint.getValue(instant1));
      assertEquals(point, upoint.getValue(instant2));
      assertEquals(point, upoint.getValue(instant3));

   }

   @Test
   public void testGetValue_AtInvalidInstant_ShouldReturnUndefinedPoint() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Point origPoint = new Point(2.0d, 3.0d);
      UnitPointIF upoint = new UnitPointConst(period, origPoint);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-09 23:59:59:999");
      TimeInstantIF instant3 = new TimeInstant("2017-01-01 00:00:00:000");
      TimeInstantIF instant4 = new TimeInstant();

      PointIF point1 = upoint.getValue(instant1);
      PointIF point2 = upoint.getValue(instant2);
      PointIF point3 = upoint.getValue(instant3);
      PointIF point4 = upoint.getValue(instant4);

      assertNotEquals(origPoint, point1);
      assertNotEquals(origPoint, point2);
      assertNotEquals(origPoint, point3);
      assertNotEquals(origPoint, point4);

      assertFalse(point1.isDefined());
      assertFalse(point2.isDefined());
      assertFalse(point3.isDefined());
      assertFalse(point4.isDefined());

   }

   @Test
   public void testGetProjectionBoundingBox() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      Point point = new Point(2.0d, 3.0d);
      UnitPointIF upoint = new UnitPointConst(period, point);

      assertEquals(point.getBoundingBox(), upoint.getProjectionBoundingBox());

   }

   @Test
   public void testEqualValue_ValidEqualUnitPointConst_ShouldBeTrue() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitPointConst uPoint1 = new UnitPointConst(period1, new Point(1.0d, 2.3d));
      UnitPointConst uPoint2 = new UnitPointConst(period2, new Point(1.0d, 2.3d));

      assertTrue(uPoint1.equalValue(uPoint2));
   }

   @Test
   public void testEqualValue_ValidNotEqualUnitPointConst_ShouldBeFalse() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitPointConst uPoint1 = new UnitPointConst(period1, new Point(1.0d, 2.3d));
      UnitPointConst uPoint2 = new UnitPointConst(period2, new Point(1.0d, 2.4d));

      assertFalse(uPoint1.equalValue(uPoint2));
   }

   @Test
   public void testEqualValue_UndefinedUnitPointConst_ShouldBeFalse() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitPointConst uPoint1 = new UnitPointConst(period1, new Point(1.0d, 2.3d));
      UnitPointConst uPoint2 = new UnitPointConst(period2, new Point());

      assertFalse(uPoint1.equalValue(uPoint2));
   }

   @Test
   public void testEqualValue_UnitPointLinear_ShouldBeFalse() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitPointConst uPoint1 = new UnitPointConst(period1, new Point(1.0d, 2.3d));
      UnitPointIF uPoint2 = new UnitPointLinear(period2, new Point(1.0d, 2.3d), new Point(1.0d, 2.3d));

      assertFalse(uPoint1.equalValue(uPoint2));
   }

   @Test
   public void testAtPeriod_IntersectingPeriod() {

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point point = new Point(2.0d, 3.0d);
      UnitPointIF oldUPoint = new UnitPointConst(period, point);

      Period newPeriod = new Period("2018-01-05 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitPointIF newUPoint = oldUPoint.atPeriod(newPeriod);

      assertEquals(period.intersection(newPeriod), newUPoint.getPeriod());
      assertEquals(point, newUPoint.getInitial());
      assertEquals(point, newUPoint.getFinal());

   }

   @Test
   public void testAtPeriod_NoIntersectingPeriod() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point point = new Point(2.0d, 3.0d);
      UnitPointIF oldUPoint = new UnitPointConst(period, point);

      Period newPeriod = new Period("2018-01-15 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitPointIF newUPoint = oldUPoint.atPeriod(newPeriod);

      assertFalse(newUPoint.isDefined());

   }

}
