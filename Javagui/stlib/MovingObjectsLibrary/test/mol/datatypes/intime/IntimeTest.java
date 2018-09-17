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
package mol.datatypes.intime;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

import org.junit.BeforeClass;
import org.junit.Test;

import mol.datatypes.GeneralType;
import mol.datatypes.base.BaseBool;
import mol.datatypes.base.BaseInt;
import mol.datatypes.base.BaseReal;
import mol.datatypes.base.BaseString;
import mol.datatypes.spatial.Point;
import mol.datatypes.time.TimeInstant;

/**
 * Tests for class 'Intime'
 * 
 * @author Markus Fuessel
 */
@SuppressWarnings("rawtypes")
public class IntimeTest {

   private static List<Intime> intimeObjects = new ArrayList<Intime>();

   @BeforeClass
   public static void init() {

      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);

      TimeInstant instant = new TimeInstant("2018-01-01 00:00:00:000");

      intimeObjects.add(new Intime<BaseBool>(instant, new BaseBool(true)));
      intimeObjects.add(new Intime<BaseInt>(instant, new BaseInt(1)));
      intimeObjects.add(new Intime<BaseReal>(instant, new BaseReal(1.23456d)));
      intimeObjects.add(new Intime<BaseString>(instant, new BaseString("aBc")));

      intimeObjects.add(new Intime<Point>(instant, new Point(1.0d, 1.0d)));
   }

   @Test
   public void testIntimeConstructor_UndefinedObject() {
      Intime<GeneralType> undefinedIntime1 = new Intime<>();
      Intime<BaseInt> undefinedIntime2 = new Intime<>(new TimeInstant(), new BaseInt(1));
      Intime<BaseInt> undefinedIntime3 = new Intime<>(new TimeInstant("2018-01-01 00:00:00:000"), new BaseInt());

      assertFalse(undefinedIntime1.isDefined());
      assertFalse(undefinedIntime2.isDefined());
      assertFalse(undefinedIntime3.isDefined());
   }

   @Test
   public void testGetInstant() {

      for (int i = 0; i < intimeObjects.size(); i++) {
         assertNotNull(intimeObjects.get(i).getInstant());
      }

   }

   @Test
   public void testGetValue() {
      for (int i = 0; i < intimeObjects.size(); i++) {
         assertNotNull(intimeObjects.get(i).getValue());
      }
   }

   @Test
   public void testCompareTo_LowerVSGreater_ShouldBeLowerThanZero() {

      Intime<BaseInt> iint0 = new Intime<BaseInt>(new TimeInstant("2018-01-01 00:00:00:000"), new BaseInt(1));
      Intime<BaseInt> iint1 = new Intime<BaseInt>(new TimeInstant("2018-01-01 00:00:00:001"), new BaseInt(2));

      assertTrue(iint0.compareTo(iint1) < 0);
   }

   @Test
   public void testCompareTo_GreaterVSLower_ShouldBeGreaterThanZero() {

      Intime<BaseInt> iint0 = new Intime<BaseInt>(new TimeInstant("2018-01-01 00:00:00:000"), new BaseInt(1));
      Intime<BaseInt> iint1 = new Intime<BaseInt>(new TimeInstant("2018-01-01 00:00:00:001"), new BaseInt(2));

      assertTrue(iint1.compareTo(iint0) > 0);
   }

   @Test
   public void testCompareTo_EqualValue_ShouldBeZero() {

      Intime<BaseInt> iint0 = new Intime<BaseInt>(new TimeInstant("2018-01-01 00:00:00:000"), new BaseInt(1));
      Intime<BaseInt> iint1 = new Intime<BaseInt>(new TimeInstant("2018-01-01 00:00:00:000"), new BaseInt(2));

      assertTrue(iint1.compareTo(iint0) == 0);
   }

}
