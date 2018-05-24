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

/**
 * Tests for class 'BaseReal'
 * 
 * @author Markus Fuessel
 */
public class BaseRealTest {

   @Test
   public void testHashCode_DifferentObjectSameValue_HashCodeShouldBeEqual() {
      BaseReal posRealA = new BaseReal(1.23456d);
      BaseReal posRealB = new BaseReal(1.23456d);
      BaseReal negRealA = new BaseReal(-1.23456d);
      BaseReal negRealB = new BaseReal(-1.23456d);

      assertTrue(posRealA.hashCode() == posRealB.hashCode());
      assertTrue(negRealA.hashCode() == negRealB.hashCode());
   }

   @Test
   public void testHashCode_DifferentValue_HashCodeShouldBeNonEqual() {
      BaseReal posRealA = new BaseReal(1.23456d);
      BaseReal posRealB = new BaseReal(1.23455d);
      BaseReal negReal = new BaseReal(-1.23456d);

      assertFalse(posRealA.hashCode() == negReal.hashCode());
      assertFalse(posRealA.hashCode() == posRealB.hashCode());
   }

   @Test
   public void testCompareTo_LowVsHighValue_ShouldBeLowerZero() {
      BaseReal posRealA = new BaseReal(1.23456d);
      BaseReal posRealB = new BaseReal(1.23455d);
      BaseReal negRealA = new BaseReal(-1.23456d);
      BaseReal negRealB = new BaseReal(-1.23455d);

      assertTrue(posRealB.compareTo(posRealA) < 0);
      assertTrue(negRealA.compareTo(posRealA) < 0);
      assertTrue(negRealA.compareTo(negRealB) < 0);

   }

   @Test
   public void testCompareTo_HighVsLowValue_ShouldBeGreaterZero() {
      BaseReal posRealA = new BaseReal(1.23456d);
      BaseReal posRealB = new BaseReal(1.23455d);
      BaseReal negRealA = new BaseReal(-1.23456d);
      BaseReal negRealB = new BaseReal(-1.23455d);

      assertTrue(posRealA.compareTo(posRealB) > 0);
      assertTrue(posRealA.compareTo(negRealA) > 0);
      assertTrue(negRealB.compareTo(negRealA) > 0);

   }

   @Test
   public void testCompareTo_SameValue_ShouldBeZero() {
      BaseReal posRealA = new BaseReal(1.23456d);
      BaseReal posRealB = new BaseReal(1.23456d);
      BaseReal negRealA = new BaseReal(-1.23456d);
      BaseReal negRealB = new BaseReal(-1.23456d);

      assertTrue(posRealA.compareTo(posRealB) == 0);
      assertTrue(negRealA.compareTo(negRealB) == 0);

   }

   @Test
   public void testEquals_EqualValue_ShouldBeTrue() {
      BaseReal posRealA = new BaseReal(1.23456d);
      BaseReal posRealB = new BaseReal(1.23456d);
      BaseReal negRealA = new BaseReal(-1.23456d);
      BaseReal negRealB = new BaseReal(-1.23456d);

      assertTrue(posRealA.equals(posRealB));
      assertTrue(negRealA.equals(negRealB));

   }

   @Test
   public void testEquals_NonEqualValue_ShouldBeFalse() {
      BaseReal posRealA = new BaseReal(1.23456d);
      BaseReal posRealB = new BaseReal(1.23455d);
      BaseReal negReal = new BaseReal(-1.23456d);

      assertFalse(posRealA.equals(negReal));
      assertFalse(posRealA.equals(posRealB));

   }

   @Test
   public void testBefore_LowerVsHigherReal_ShouldBeTrue() {
      BaseReal real1 = new BaseReal(1.00000000d);
      BaseReal real2 = new BaseReal(1.00000001d);

      assertTrue(real1.before(real2));

   }

   @Test
   public void testBefore_EqualReal_ShouldBeFalse() {
      BaseReal real1A = new BaseReal(1.00000000d);
      BaseReal real1B = new BaseReal(1.00000000d);

      assertFalse(real1A.before(real1B));

   }

   @Test
   public void testAfter_HigherVsLowerReal_ShouldBeTrue() {
      BaseReal real1 = new BaseReal(1.00000000d);
      BaseReal real2 = new BaseReal(1.00000001d);

      assertTrue(real2.after(real1));

   }

   @Test
   public void testAfter_EqualInteger_ShouldBeFalse() {
      BaseReal real1A = new BaseReal(1.00000000d);
      BaseReal real1B = new BaseReal(1.00000000d);

      assertFalse(real1A.after(real1B));

   }

   @Test
   public void testAdjacent_DifferentReals_ShouldAlwaysFalse() {
      BaseReal real1A = new BaseReal(1.00000000d);
      BaseReal real1B = new BaseReal(1.00000000d);
      BaseReal real2 = new BaseReal(1.00000001d);

      assertFalse(real1A.adjacent(real1B));
      assertFalse(real1A.adjacent(real2));
   }

}
