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
package mol.datatypes.unit;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;

import org.junit.BeforeClass;
import org.junit.Test;

import mol.datatypes.base.BaseBool;
import mol.datatypes.interval.Period;
import mol.datatypes.time.TimeInstant;
import mol.interfaces.base.BaseBoolIF;
import mol.interfaces.time.TimeInstantIF;
import mol.interfaces.unit.UnitBoolIF;

/**
 * Tests for the 'UnitBool' class
 * 
 * @author Markus Fuessel
 */
public class UnitBoolTest {

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Test
   public void testUnitBool_UndefinedObject() {
      UnitBool ubool = new UnitBool();

      assertFalse(ubool.isDefined());
   }

   @Test
   public void testGetValue_AtValidInstant() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      BaseBoolIF origBaseBool = new BaseBool(true);
      UnitBool ubool = new UnitBool(period, origBaseBool);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-05 00:00:00:000");
      TimeInstantIF instant3 = new TimeInstant("2018-01-09 23:59:59:999");

      assertEquals(origBaseBool, ubool.getValue(instant1));
      assertEquals(origBaseBool, ubool.getValue(instant2));
      assertEquals(origBaseBool, ubool.getValue(instant3));

   }

   @Test
   public void testGetValue_AtInvalidInstant_ShouldReturnUndefinedBaseBool() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      BaseBoolIF origBaseBool = new BaseBool(true);
      UnitBool ubool = new UnitBool(period, origBaseBool);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-09 23:59:59:999");
      TimeInstantIF instant3 = new TimeInstant("2017-01-01 00:00:00:000");
      TimeInstantIF instant4 = new TimeInstant();

      BaseBoolIF bool1 = ubool.getValue(instant1);
      BaseBoolIF bool2 = ubool.getValue(instant2);
      BaseBoolIF bool3 = ubool.getValue(instant3);
      BaseBoolIF bool4 = ubool.getValue(instant4);

      assertNotEquals(origBaseBool, bool1);
      assertNotEquals(origBaseBool, bool2);
      assertNotEquals(origBaseBool, bool3);
      assertNotEquals(origBaseBool, bool4);

      assertFalse(bool1.isDefined());
      assertFalse(bool2.isDefined());
      assertFalse(bool3.isDefined());
      assertFalse(bool4.isDefined());

   }

   @Test
   public void testGetValue_ShouldReturnBaseBoolOfUnit() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      BaseBoolIF origBaseBool = new BaseBool(true);
      UnitBool ubool = new UnitBool(period, origBaseBool);

      BaseBoolIF bBool = ubool.getValue();

      assertTrue(bBool.isDefined());
      assertEquals(origBaseBool, bBool);

   }

   @Test
   public void testGetInitial_ShouldReturnBaseBoolOfUnit() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      BaseBoolIF origBaseBool = new BaseBool(true);
      UnitBool ubool = new UnitBool(period, origBaseBool);

      BaseBoolIF bBool = ubool.getInitial();

      assertTrue(bBool.isDefined());
      assertEquals(origBaseBool, bBool);

   }

   @Test
   public void testGetFinal_ShouldReturnBaseBoolOfUnit() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      BaseBoolIF origBaseBool = new BaseBool(true);
      UnitBool ubool = new UnitBool(period, origBaseBool);

      BaseBoolIF bBool = ubool.getFinal();

      assertTrue(bBool.isDefined());
      assertEquals(origBaseBool, bBool);

   }

   @Test
   public void testEqualValue_ValidEqualUnitBool_ShouldBeTrue() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitBool ubool1 = new UnitBool(period1, new BaseBool(true));
      UnitBool ubool2 = new UnitBool(period2, new BaseBool(true));

      assertTrue(ubool1.equalValue(ubool2));
   }

   @Test
   public void testEqualValue_ValidNotEqualUnitBool_ShouldBeFalse() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitBool ubool1 = new UnitBool(period1, new BaseBool(true));
      UnitBool ubool2 = new UnitBool(period2, new BaseBool(false));

      assertFalse(ubool1.equalValue(ubool2));
   }

   @Test
   public void testEqualValue_UndefinedUnitBool_ShouldBeFalse() {
      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      Period period2 = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", false, false);

      UnitBool ubool1 = new UnitBool(period1, new BaseBool(true));
      UnitBool ubool2 = new UnitBool(period2, new BaseBool());

      assertFalse(ubool1.equalValue(ubool2));
   }

   @Test
   public void testAtPeriod_IntersectingPeriod() {

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      BaseBoolIF origBaseBool = new BaseBool(true);
      UnitBoolIF oldUBool = new UnitBool(period, origBaseBool);

      Period newPeriod = new Period("2018-01-05 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitBoolIF newUBool = oldUBool.atPeriod(newPeriod);

      assertTrue(newUBool.isDefined());
      assertEquals(period.intersection(newPeriod), newUBool.getPeriod());
      assertEquals(origBaseBool, newUBool.getInitial());
      assertEquals(origBaseBool, newUBool.getFinal());

   }

   @Test
   public void testAtPeriod_NoIntersectingPeriod() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      BaseBoolIF origBaseBool = new BaseBool(true);
      UnitBoolIF oldUBool = new UnitBool(period, origBaseBool);

      Period newPeriod = new Period("2018-01-15 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitBoolIF newUBool = oldUBool.atPeriod(newPeriod);

      assertFalse(newUBool.isDefined());

   }
}
