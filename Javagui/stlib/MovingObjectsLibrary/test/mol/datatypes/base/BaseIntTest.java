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

package mol.datatypes.base;

import static org.junit.Assert.*;

import org.junit.Test;

import mol.interfaces.base.BaseIntIF;

/**
 * Tests for class 'BaseInt'
 * 
 * @author Markus Fuessel
 */
public class BaseIntTest {

   @Test
   public void testBaseIntConstructor_UndefinedObject() {
      BaseInt undefinedInt = new BaseInt();

      assertFalse(undefinedInt.isDefined());
   }

   @Test
   public void testBaseIntCopyConstructor() {
      BaseInt baseInt = new BaseInt(5);

      BaseIntIF copybaseInt = new BaseInt(baseInt);

      assertEquals(baseInt, copybaseInt);
   }

   @Test
   public void testHashCode_DifferentObjectSameValue_HashCodeShouldBeEqual() {
      BaseIntIF posIntegerA = new BaseInt(1);
      BaseIntIF posIntegerB = new BaseInt(1);
      BaseIntIF negIntegerA = new BaseInt(-1);
      BaseIntIF negIntegerB = new BaseInt(-1);

      assertTrue(posIntegerA.hashCode() == posIntegerB.hashCode());
      assertTrue(negIntegerA.hashCode() == negIntegerB.hashCode());
   }

   @Test
   public void testHashCode_DifferentValue_HashCodeShouldBeNonEqual() {
      BaseIntIF posInteger = new BaseInt(1);
      BaseIntIF negInteger = new BaseInt(-1);

      assertFalse(posInteger.hashCode() == negInteger.hashCode());
   }

   @Test
   public void testCompareTo_LowVsHighValue_ShouldBeLowerZero() {
      BaseInt posInteger = new BaseInt(1);
      BaseInt negInteger = new BaseInt(-1);

      assertTrue(negInteger.compareTo(posInteger) < 0);

   }

   @Test
   public void testCompareTo_HighVsLowValue_ShouldBeGreaterZero() {
      BaseInt posInteger = new BaseInt(1);
      BaseInt negInteger = new BaseInt(-1);

      assertTrue(posInteger.compareTo(negInteger) > 0);

   }

   @Test
   public void testCompareTo_SameValue_ShouldBeZero() {
      BaseInt posIntegerA = new BaseInt(1);
      BaseInt posIntegerB = new BaseInt(1);
      BaseInt negIntegerA = new BaseInt(-1);
      BaseInt negIntegerB = new BaseInt(-1);

      assertTrue(posIntegerA.compareTo(posIntegerB) == 0);
      assertTrue(negIntegerA.compareTo(negIntegerB) == 0);

   }

   @Test
   public void testEquals_EqualValue_ShouldBeTrue() {
      BaseIntIF posIntegerA = new BaseInt(1);
      BaseIntIF posIntegerB = new BaseInt(1);
      BaseIntIF negIntegerA = new BaseInt(-1);
      BaseIntIF negIntegerB = new BaseInt(-1);

      assertTrue(posIntegerA.equals(posIntegerB));
      assertTrue(negIntegerA.equals(negIntegerB));

   }

   @Test
   public void testEquals_NonEqualValue_ShouldBeFalse() {
      BaseIntIF posInteger = new BaseInt(1);
      BaseIntIF negInteger = new BaseInt(-1);

      assertFalse(posInteger.equals(negInteger));

   }

   @Test
   public void testEquals_NullObject_ShouldBeFalse() {
      BaseIntIF baseInt = new BaseInt(5);
      Object object = null;

      assertFalse(baseInt.equals(object));

   }

   @Test
   public void testEquals_SameObject_ShouldBeTrue() {
      BaseIntIF baseInt = new BaseInt(5);

      Object object = baseInt;

      assertTrue(baseInt.equals(object));

   }

   @Test
   public void testBefore_LowerVsHigherInteger_ShouldBeTrue() {
      BaseInt int1 = new BaseInt(1);
      BaseInt int2 = new BaseInt(2);

      assertTrue(int1.before(int2));

   }

   @Test
   public void testBefore_EqualInteger_ShouldBeFalse() {
      BaseInt int1A = new BaseInt(1);
      BaseInt int1B = new BaseInt(1);

      assertFalse(int1A.before(int1B));

   }

   @Test
   public void testAfter_HigherVsLowerInteger_ShouldBeTrue() {
      BaseInt int1 = new BaseInt(1);
      BaseInt int2 = new BaseInt(2);

      assertTrue(int2.after(int1));

   }

   @Test
   public void testAfter_EqualInteger_ShouldBeFalse() {
      BaseInt int1A = new BaseInt(1);
      BaseInt int1B = new BaseInt(1);

      assertFalse(int1A.after(int1B));

   }

   @Test
   public void testAdjacent_AdjacentIntegers_ShouldBeTrue() {
      BaseInt int1 = new BaseInt(1);
      BaseIntIF int2 = new BaseInt(2);

      assertTrue(int1.adjacent(int2));

   }

   @Test
   public void testAdjacent_NonAdjacentOrEqualIntegers_ShouldBeFalse() {
      BaseInt int1A = new BaseInt(1);
      BaseIntIF int1B = new BaseInt(1);
      BaseIntIF int5 = new BaseInt(5);

      assertFalse(int1A.adjacent(int5));
      assertFalse(int1A.adjacent(int1B));

   }

}
