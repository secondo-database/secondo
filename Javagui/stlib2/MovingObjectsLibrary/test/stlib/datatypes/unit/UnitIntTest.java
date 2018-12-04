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
package stlib.datatypes.unit;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;

import org.junit.BeforeClass;
import org.junit.Test;

import stlib.datatypes.base.BaseInt;
import stlib.datatypes.interval.Period;
import stlib.datatypes.time.TimeInstant;
import stlib.datatypes.unit.UnitInt;
import stlib.interfaces.base.BaseIntIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitIntIF;

/**
 * Tests for the 'UnitInt' class
 * 
 * @author Markus Fuessel
 */
public class UnitIntTest {

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Test
   public void testUnitInt_UndefinedObject() {
      UnitInt uint = new UnitInt();

      assertFalse(uint.isDefined());
   }

   @Test
   public void testBefore_IsBeforeOtherUnit_ShouldBeTrue() {
      UnitInt uint1 = new UnitInt(new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true), 5);
      UnitInt uint2 = new UnitInt(new Period("2018-01-09 23:59:59:999", "2018-01-20 23:59:59:999", false, true), 5);

      assertTrue(uint1.before(uint2));
   }

   @Test
   public void testBefore_IsNotBeforeOtherUnit_ShouldBeTrue() {
      UnitInt uint1 = new UnitInt(new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true), 5);
      UnitInt uint2 = new UnitInt(new Period("2018-01-09 23:59:59:999", "2018-01-20 23:59:59:999", true, true), 5);

      assertFalse(uint1.before(uint2));
   }

   @Test
   public void testPeriodEndsWithin_IsWithinOtherUnit_ShouldBeTrue() {
      UnitInt uint1 = new UnitInt(new Period("2018-01-01 00:00:00:000", "2018-01-20 23:59:59:998", true, true), 5);
      UnitInt uint2 = new UnitInt(new Period("2018-01-04 23:59:59:999", "2018-01-20 23:59:59:999", true, true), 5);

      assertTrue(uint1.periodEndsWithin(uint2));
   }

   @Test
   public void testPeriodEndsWithin_IsBeforeOtherUnit_ShouldBeFalse() {
      UnitInt uint1 = new UnitInt(new Period("2018-01-01 00:00:00:000", "2018-01-08 23:59:59:999", true, true), 5);
      UnitInt uint2 = new UnitInt(new Period("2018-01-09 23:59:59:999", "2018-01-20 23:59:59:999", true, true), 5);

      assertFalse(uint1.periodEndsWithin(uint2));
   }

   @Test
   public void testGetValue_AtValidInstant() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      BaseIntIF origBaseInt = new BaseInt(5);
      UnitInt uint = new UnitInt(period, origBaseInt);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-05 00:00:00:000");
      TimeInstantIF instant3 = new TimeInstant("2018-01-09 23:59:59:999");

      assertEquals(origBaseInt, uint.getValue(instant1));
      assertEquals(origBaseInt, uint.getValue(instant2));
      assertEquals(origBaseInt, uint.getValue(instant3));

   }

   @Test
   public void testGetValue_AtInvalidInstant_ShouldReturnUndefinedBaseInt() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      BaseIntIF origBaseInt = new BaseInt(5);
      UnitInt uint = new UnitInt(period, origBaseInt);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-09 23:59:59:999");
      TimeInstantIF instant3 = new TimeInstant("2017-01-01 00:00:00:000");
      TimeInstantIF instant4 = new TimeInstant();

      BaseIntIF int1 = uint.getValue(instant1);
      BaseIntIF int2 = uint.getValue(instant2);
      BaseIntIF int3 = uint.getValue(instant3);
      BaseIntIF int4 = uint.getValue(instant4);

      assertNotEquals(origBaseInt, int1);
      assertNotEquals(origBaseInt, int2);
      assertNotEquals(origBaseInt, int3);
      assertNotEquals(origBaseInt, int4);

      assertFalse(int1.isDefined());
      assertFalse(int2.isDefined());
      assertFalse(int3.isDefined());
      assertFalse(int4.isDefined());

   }

   @Test
   public void testGetValue_ShouldReturnBaseIntOfUnit() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      int intValue = 5;
      BaseIntIF origBaseInt = new BaseInt(intValue);
      UnitInt uint = new UnitInt(period, intValue);

      BaseIntIF bInt = uint.getValue();

      assertTrue(bInt.isDefined());
      assertEquals(origBaseInt, bInt);

   }

   @Test
   public void testGetInitial_ShouldReturnBaseIntOfUnit() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      BaseIntIF origBaseInt = new BaseInt(5);
      UnitInt uint = new UnitInt(period, origBaseInt);

      BaseIntIF bInt = uint.getInitial();

      assertTrue(bInt.isDefined());
      assertEquals(origBaseInt, bInt);

   }

   @Test
   public void testGetFinal_ShouldReturnBaseIntOfUnit() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      BaseIntIF origBaseInt = new BaseInt(5);
      UnitInt uint = new UnitInt(period, origBaseInt);

      BaseIntIF bInt = uint.getFinal();

      assertTrue(bInt.isDefined());
      assertEquals(origBaseInt, bInt);

   }

   @Test
   public void testEqualValue_ValidEqualUnitInt_ShouldBeTrue() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitInt uint1 = new UnitInt(period1, new BaseInt(5));
      UnitInt uint2 = new UnitInt(period2, new BaseInt(5));

      assertTrue(uint1.equalValue(uint2));
   }

   @Test
   public void testEqualValue_ValidNotEqualUnitInt_ShouldBeFalse() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitInt uint1 = new UnitInt(period1, new BaseInt(5));
      UnitInt uint2 = new UnitInt(period2, new BaseInt(6));

      assertFalse(uint1.equalValue(uint2));
   }

   @Test
   public void testEqualValue_UndefinedUnitInt_ShouldBeFalse() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitInt uint1 = new UnitInt(period1, new BaseInt(5));
      UnitInt uint2 = new UnitInt(period2, new BaseInt());

      assertFalse(uint1.equalValue(uint2));
   }

   @Test
   public void testAtPeriod_IntersectingPeriod() {

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      BaseIntIF origBaseInt = new BaseInt(5);
      UnitIntIF oldUInt = new UnitInt(period, origBaseInt);

      Period newPeriod = new Period("2018-01-05 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitIntIF newUInt = oldUInt.atPeriod(newPeriod);

      assertTrue(newUInt.isDefined());
      assertEquals(period.intersection(newPeriod), newUInt.getPeriod());
      assertEquals(origBaseInt, newUInt.getInitial());
      assertEquals(origBaseInt, newUInt.getFinal());

   }

   @Test
   public void testAtPeriod_NoIntersectingPeriod() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      BaseIntIF origBaseInt = new BaseInt(5);
      UnitIntIF oldUInt = new UnitInt(period, origBaseInt);

      Period newPeriod = new Period("2018-01-15 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitIntIF newUInt = oldUInt.atPeriod(newPeriod);

      assertFalse(newUInt.isDefined());

   }

}
