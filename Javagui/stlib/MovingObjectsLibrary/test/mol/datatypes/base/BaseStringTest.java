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
 * Tests for class 'BaseString'
 * 
 * @author Markus Fuessel
 */
public class BaseStringTest {

   @Test
   public void testBaseStringConstructor_UndefinedObject() {
      BaseString undefinedString = new BaseString();

      assertFalse(undefinedString.isDefined());
   }

   @Test
   public void testBaseStringCopyConstructor() {
      BaseString baseString = new BaseString("aBc");

      BaseString copybaseString = new BaseString(baseString);

      assertEquals(baseString, copybaseString);
   }

   @Test
   public void testHashCode_DifferentObjectSameValue_HashCodeShouldBeEqual() {
      BaseString stringA1 = new BaseString("aBc");
      BaseString stringA2 = new BaseString("aBc");

      assertTrue(stringA1.hashCode() == stringA2.hashCode());
   }

   @Test
   public void testHashCode_DifferentValue_HashCodeShouldBeNonEqual() {
      BaseString stringA = new BaseString("aBc");
      BaseString stringB = new BaseString("abc");
      BaseString stringC = new BaseString("abc ");

      assertFalse(stringA.hashCode() == stringB.hashCode());
      assertFalse(stringB.hashCode() == stringC.hashCode());
   }

   @Test
   public void testCompareTo_LowVsHighValue_ShouldBeLowerZero() {
      BaseString stringA = new BaseString("aa");
      BaseString stringB = new BaseString("ab");
      BaseString stringC = new BaseString("ac");

      assertTrue(stringA.compareTo(stringB) < 0);
      assertTrue(stringB.compareTo(stringC) < 0);
      assertTrue(stringA.compareTo(stringC) < 0);

   }

   @Test
   public void testCompareTo_HighVsLowValue_ShouldBeGreaterZero() {
      BaseString stringA = new BaseString("aa");
      BaseString stringB = new BaseString("ab");
      BaseString stringC = new BaseString("ac");

      assertTrue(stringC.compareTo(stringB) > 0);
      assertTrue(stringC.compareTo(stringA) > 0);
      assertTrue(stringB.compareTo(stringA) > 0);

   }

   @Test
   public void testCompareTo_SameValue_ShouldBeZero() {
      BaseString stringA1 = new BaseString("aa");
      BaseString stringA2 = new BaseString("aa");
      BaseString stringB1 = new BaseString("a b");
      BaseString stringB2 = new BaseString("a b");

      assertTrue(stringA1.compareTo(stringA2) == 0);
      assertTrue(stringB1.compareTo(stringB2) == 0);

   }

   @Test
   public void testEquals_EqualValue_ShouldBeTrue() {
      BaseString stringA1 = new BaseString("aa");
      BaseString stringA2 = new BaseString("aa");
      BaseString stringB1 = new BaseString("a b");
      BaseString stringB2 = new BaseString("a b");

      assertTrue(stringA1.equals(stringA2));
      assertTrue(stringB1.equals(stringB2));

   }

   @Test
   public void testEquals_NonEqualValue_ShouldBeFalse() {
      BaseString stringA = new BaseString("aa");
      BaseString stringB = new BaseString("aa ");
      BaseString stringC = new BaseString("AA");

      assertFalse(stringA.equals(stringB));
      assertFalse(stringA.equals(stringC));

   }

   @Test
   public void testEquals_NullObject_ShouldBeFalse() {
      BaseString baseString = new BaseString("aBc");
      Object object = null;

      assertFalse(baseString.equals(object));

   }

   @Test
   public void testEquals_SameObject_ShouldBeTrue() {
      BaseString baseString = new BaseString("aBc");

      Object object = baseString;

      assertTrue(baseString.equals(object));

   }

   @Test
   public void testBefore_StringInAlphabeticalOrder_ShouldBeTrue() {
      BaseString string1 = new BaseString("aaa");
      BaseString string2 = new BaseString("aab");
      BaseString string3 = new BaseString("aa");

      assertTrue(string1.before(string2));
      assertTrue(string3.before(string1));
   }

   @Test
   public void testBefore_EqualString_ShouldBeFalse() {
      BaseString string1 = new BaseString("aaa");
      BaseString string2 = new BaseString("aaa");

      assertFalse(string1.before(string2));

   }

   @Test
   public void testAfter_StringInAlphabeticalOrder_ShouldBeTrue() {
      BaseString string1 = new BaseString("aaa");
      BaseString string2 = new BaseString("aab");
      BaseString string3 = new BaseString("aa");

      assertTrue(string2.after(string1));
      assertTrue(string1.after(string3));
   }

   @Test
   public void testAfter_EqualString_ShouldBeFalse() {
      BaseString string1 = new BaseString("aaa");
      BaseString string2 = new BaseString("aaa");

      assertFalse(string1.before(string2));

   }

   @Test
   public void testAdjacent_AdjacentStrings_ShouldBeTrue() {
      BaseString string1 = new BaseString("aaa");
      BaseString string2 = new BaseString("aab");

      assertTrue(string1.adjacent(string2));
      assertTrue(string2.adjacent(string1));

   }

   @Test
   public void testAdjacent_NonAdjacentStrings_ShouldBeFalse() {
      BaseString string1 = new BaseString("aaa");
      BaseString string2 = new BaseString("aaa");
      BaseString string3 = new BaseString("aaaa");
      BaseString string4 = new BaseString("aaba");

      assertFalse(string1.adjacent(string2));
      assertFalse(string1.adjacent(string3));
      assertFalse(string3.adjacent(string4));

   }

}
