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

import stlib.datatypes.base.BaseBool;
import stlib.datatypes.interval.Period;
import stlib.datatypes.moving.MovingBool;
import stlib.datatypes.moving.MovingPoint;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.time.TimeInstant;
import stlib.interfaces.base.BaseBoolIF;
import stlib.interfaces.intime.IntimeIF;
import stlib.interfaces.spatial.PointIF;

/**
 * Tests for 'Initial' class methods
 * 
 * @author Markus Fuessel
 */
public class InitialTest {

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
   public void testInitial_OnDefinedMBool_ShouldReturnDefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool = Initial.execute(mbool);

      assertTrue(ibool.isDefined());
      assertEquals(new BaseBool(true), ibool.getValue());

   }

   @Test
   public void testInitial_OnNullObject_ShouldReturnUndefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool = Initial.execute(null);

      assertFalse(ibool.isDefined());

   }

   @Test
   public void testInitial_OnDefinedMPoint_ShouldReturnDefinedIntimePoint() {

      IntimeIF<PointIF> ipoint = Initial.execute(mpointContinuos);

      assertTrue(ipoint.isDefined());
      assertEquals(new Point(0.0d, 0.0d), ipoint.getValue());

   }

}
