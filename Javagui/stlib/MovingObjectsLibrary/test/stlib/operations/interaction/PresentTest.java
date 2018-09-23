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
import stlib.datatypes.time.TimeInstant;

/**
 * Tests for 'Present' class methods
 * 
 * @author Markus Fuessel
 */
public class PresentTest {

   /**
    * [2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000), TRUE<br>
    * [2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000), FALSE<br>
    * [2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000), TRUE
    */
   private MovingBool mbool;

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
   }

   @Test
   public void testPresent_PassDefinedTimeInstantInsideDefinedPeriod_ShouldBeTrue() {

      assertTrue(Present.execute(mbool, new TimeInstant("2018-01-20 00:00:00:000")));
      assertTrue(Present.execute(mbool, new TimeInstant("2018-01-25 00:00:00:000")));
   }

   @Test
   public void testPresent_PassDefinedTimeInstantOutsideDefinedPeriod_ShouldBeFalse() {

      assertFalse(Present.execute(mbool, new TimeInstant("2017-12-31 23:59:59:999")));
      assertFalse(Present.execute(mbool, new TimeInstant("2018-01-30 00:00:00:000")));
   }

   @Test
   public void testPresent_PassUndefinedTimeInstant_ShouldBeFalse() {

      assertFalse(Present.execute(mbool, new TimeInstant()));
   }

}
