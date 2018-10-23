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

package stlib.operations.interaction;

import static org.junit.Assert.*;

import java.time.format.DateTimeFormatter;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import stlib.datatypes.interval.Period;
import stlib.datatypes.moving.MovingBool;
import stlib.datatypes.moving.MovingPoint;
import stlib.datatypes.range.Periods;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.time.TimeInstant;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.moving.MovingBoolIF;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.range.PeriodsIF;

/**
 * Tests for 'AtPeriods' class methods
 * 
 * @author Markus Fuessel
 */
public class AtPeriodsTest {

   /**
    * ["2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000"), TRUE<br>
    * ["2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000"), FALSE<br>
    * ["2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000"), TRUE
    */
   private MovingBool mbool;

   private MovingPoint mpointContinuos;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {

      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);

   }

   @Before
   public void setUp() throws Exception {

      mbool = new MovingBool(0);

      mbool.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), true);
      mbool.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), false);
      mbool.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), true);

      mpointContinuos = new MovingPoint(0);

      Point p1 = new Point(0.0d, 0.0d);
      Point p2 = new Point(10.0d, 10.0d);
      Point p3 = new Point(10.0d, 20.0d);
      Point p4 = new Point(20.0d, 5.0d);

      mpointContinuos.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), p1, p2);
      mpointContinuos.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), p2, p3);
      mpointContinuos.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), p3, p4);

   }

   @Test
   public void testAtPeriods_OnMBool_ValidPeriodsObject_DefinedResult() {
      PeriodsIF periods = new Periods(0);

      PeriodIF period1 = new Period("2018-01-05 00:00:00:000", "2018-01-06 00:00:00:000", true, false);
      PeriodIF period2 = new Period("2018-01-25 00:00:00:000", "2018-01-26 00:00:00:000", true, false);

      periods.add(period1);
      periods.add(period2);

      MovingBoolIF mboolResult = AtPeriods.execute(mbool, periods);

      assertTrue(mboolResult.isDefined());
      assertEquals(2, mboolResult.getNoUnits());
      assertEquals(period1, mboolResult.getUnit(0).getPeriod());
      assertEquals(period2, mboolResult.getUnit(1).getPeriod());
   }

   @Test
   public void testAtPeriods_OnMBool_ValidPeriodsObjectWithOnePeriod_DefinedResult() {
      PeriodsIF periods = new Periods(0);

      periods.add(new Period("2017-01-05 00:00:00:000", "2018-03-06 00:00:00:000", true, false));

      MovingBoolIF mboolResult = AtPeriods.execute(mbool, periods);

      assertTrue(mboolResult.isDefined());
      assertEquals(3, mboolResult.getNoUnits());
   }

   @Test
   public void testAtPeriods_OnMBool_InvalidPeriodsObject_UndefinedResult() {
      PeriodsIF periods = new Periods(0);

      MovingBoolIF mboolResult = AtPeriods.execute(mbool, periods);

      assertFalse(mboolResult.isDefined());
   }

   @Test
   public void testAtPeriods_OnMPoint_ValidPeriodsObject_DefinedResult() {
      PeriodsIF periods = new Periods(0);

      PeriodIF period1 = new Period("2018-01-05 00:00:00:000", "2018-01-06 00:00:00:000", true, false);
      PeriodIF period2 = new Period("2018-01-25 00:00:00:000", "2018-01-26 00:00:00:000", true, false);

      periods.add(period1);
      periods.add(period2);

      MovingPointIF mpointResult = AtPeriods.execute(mpointContinuos, periods);

      assertTrue(mpointResult.isDefined());
      assertEquals(2, mpointResult.getNoUnits());
      assertEquals(period1, mpointResult.getUnit(0).getPeriod());
      assertEquals(period2, mpointResult.getUnit(1).getPeriod());
   }

}
