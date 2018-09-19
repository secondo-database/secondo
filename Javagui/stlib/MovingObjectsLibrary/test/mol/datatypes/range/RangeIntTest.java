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

package mol.datatypes.range;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Before;
import org.junit.Test;

import mol.datatypes.base.BaseInt;
import mol.datatypes.interval.IntervalInt;
import mol.interfaces.base.BaseIntIF;
import mol.interfaces.interval.IntervalIntIF;

/**
 * Tests for class 'RangeInt'
 * 
 * @author Markus Fuessel
 */
public class RangeIntTest {

   /**
    * [1, 5] [7, 9] [11, 15]
    */
   static RangeInt staticRInt;

   static IntervalInt lClosed1rClosed5 = new IntervalInt(1, 5, true, true); // [1, 5]
   static IntervalInt lClosed7rClosed9 = new IntervalInt(7, 9, true, true); // [7, 9]
   static IntervalInt lClosed11rClosed15 = new IntervalInt(11, 15, true, true); // [11, 15]

   @Before
   public void setUp() throws Exception {
      staticRInt = new RangeInt(3);

      staticRInt.add(lClosed1rClosed5);
      staticRInt.add(lClosed7rClosed9);
      staticRInt.add(lClosed11rClosed15);
   }

   @Test
   public void testAdd_AddIntervalInt_NoComponentsShouldIncrease() {

      int noCompBeforeAdd = staticRInt.getNoComponents();
      IntervalInt ivInt = new IntervalInt(17, 20, true, true);

      staticRInt.add(ivInt);

      assertEquals(noCompBeforeAdd + 1, staticRInt.getNoComponents());
   }

   @Test
   public void testAdd_AddUndefinedIntervalInt_NoComponentsShouldNotIncrease() {

      int noCompBeforeAdd = staticRInt.getNoComponents();
      IntervalInt ivInt = new IntervalInt(20, 19, true, true);

      boolean successful = staticRInt.add(ivInt);

      assertFalse(successful);
      assertEquals(noCompBeforeAdd, staticRInt.getNoComponents());
   }

   @Test
   public void testMergeAdd_AddIntersectingIntervalInt_NoComponentsShouldNotIncrease() {
      int noCompBeforeMergeAdd = staticRInt.getNoComponents();

      IntervalIntIF lastIvInt = staticRInt.last();
      IntervalInt ivInt = new IntervalInt(lastIvInt.getLowerBound().getValue() + 1,
            lastIvInt.getUpperBound().getValue() + 10, true, true);

      staticRInt.mergeAdd(ivInt);

      assertEquals(noCompBeforeMergeAdd, staticRInt.getNoComponents());
   }

   @Test
   public void testMergeAdd_AddDisjointIntervalInt_NoComponentsShouldIncrease() {
      int noCompBeforeMergeAdd = staticRInt.getNoComponents();

      IntervalIntIF lastIvInt = staticRInt.last();
      IntervalInt ivInt = new IntervalInt(lastIvInt.getUpperBound().getValue() + 2,
            lastIvInt.getUpperBound().getValue() + 10, true, true);

      staticRInt.mergeAdd(ivInt);

      assertEquals(noCompBeforeMergeAdd + 1, staticRInt.getNoComponents());
   }

   @Test
   public void testMergeAdd_AddDisjointIntervalIntBeforeStaticRInt_MergeAddShouldFail() {
      int noCompBeforeMergeAdd = staticRInt.getNoComponents();

      IntervalIntIF firstIvInt = staticRInt.first();
      IntervalInt ivInt = new IntervalInt(firstIvInt.getLowerBound().getValue() - 10,
            firstIvInt.getLowerBound().getValue() - 2, true, true);

      boolean successful = staticRInt.mergeAdd(ivInt);

      assertEquals(noCompBeforeMergeAdd, staticRInt.getNoComponents());
      assertFalse(successful);
   }

   @Test
   public void testIntersects_IntervalIntOverlapsOneIntervalIntOfRange_ShouldBeTrue() {
      IntervalInt ivInt1 = new IntervalInt(12, 20, true, true);
      IntervalInt ivInt2 = new IntervalInt(11, 15, true, true);

      boolean intersects1 = staticRInt.intersects(ivInt1);
      boolean intersects2 = staticRInt.intersects(ivInt2);

      assertTrue(intersects1);
      assertTrue(intersects2);
   }

   @Test
   public void testIntersects_IntervalIntAdjacentToOneIntervalIntOfRange_ShouldBeFalse() {
      IntervalInt ivInt1 = new IntervalInt(-5, 1, true, false);
      IntervalInt ivInt2 = new IntervalInt(15, 20, false, true);

      boolean intersects1 = staticRInt.intersects(ivInt1);
      boolean intersects2 = staticRInt.intersects(ivInt2);

      assertFalse(intersects1);
      assertFalse(intersects2);
   }

   @Test
   public void testIntersects_IntervalIntLiesWithinRangeWithoutOverlapping_ShouldBeFalse() {

      IntervalInt ivIntAdd = new IntervalInt(30, 40, true, true);
      IntervalInt ivIntWithin = new IntervalInt(16, 20, true, true);

      staticRInt.add(ivIntAdd);

      boolean intersects = staticRInt.intersects(ivIntWithin);

      assertFalse(intersects);
   }

   @Test
   public void testIntersects_UndefinedIntervalInt_ShouldBeFalse() {
      IntervalInt undefinedIvInt = new IntervalInt();

      boolean intersects = staticRInt.intersects(undefinedIvInt);

      assertFalse(intersects);
   }

   @Test
   public void testIntersects_OverlapsWithOtherRangeInt_ShouldBeTrue() {
      RangeInt otherRInt = new RangeInt(0);

      otherRInt.mergeAdd(new IntervalInt(15, 20, true, true));
      otherRInt.mergeAdd(new IntervalInt(22, 30, true, true));

      assertTrue(staticRInt.intersects(otherRInt));
      assertTrue(otherRInt.intersects(staticRInt));
   }

   @Test
   public void testIntersects_IntervalOfOtherRangeIntCoversCompleteRange_ShouldBeTrue() {
      RangeInt otherRInt = new RangeInt(0);

      otherRInt.mergeAdd(new IntervalInt(0, 16, true, true));
      otherRInt.mergeAdd(new IntervalInt(22, 30, true, true));

      assertTrue(staticRInt.intersects(otherRInt));
      assertTrue(otherRInt.intersects(staticRInt));
   }

   @Test
   public void testIntersects_OverlapsNotWithOtherRangeInt_ShouldBeFalse() {
      RangeInt otherRInt = new RangeInt(0);

      otherRInt.mergeAdd(new IntervalInt(15, 20, false, true));
      otherRInt.mergeAdd(new IntervalInt(22, 30, true, true));

      assertFalse(staticRInt.intersects(otherRInt));
      assertFalse(otherRInt.intersects(staticRInt));
   }

   @Test
   public void testIntersects_OtherRangeIntCoversGaps_ShouldBeFalse() {
      RangeInt otherRInt = new RangeInt(0);

      otherRInt.mergeAdd(new IntervalInt(5, 7, false, false));
      otherRInt.mergeAdd(new IntervalInt(9, 11, false, false));

      assertFalse(staticRInt.intersects(otherRInt));
      assertFalse(otherRInt.intersects(staticRInt));
   }

   @Test
   public void testIntersects_OtherRangeEmpty_ShouldBeFalse() {
      RangeInt otherRInt = new RangeInt(0);

      assertFalse(staticRInt.intersects(otherRInt));
      assertFalse(otherRInt.intersects(staticRInt));
   }

   @Test
   public void testIntersects_OtherRangeUndefined_ShouldBeFalse() {
      RangeInt otherRInt = new RangeInt(0);

      otherRInt.mergeAdd(new IntervalInt(15, 20, true, true));
      otherRInt.mergeAdd(new IntervalInt(22, 30, true, true));

      otherRInt.setDefined(false);

      assertFalse(staticRInt.intersects(otherRInt));
      assertFalse(otherRInt.intersects(staticRInt));
   }

   @Test
   public void testAdjacent_AdjacentIntervalInt_ShouldBeTrue() {

      IntervalInt adjLeftIvInt = new IntervalInt(-5, 0, true, true);
      IntervalInt adjRightIvInt = new IntervalInt(16, 20, true, true);

      assertTrue(staticRInt.adjacent(adjLeftIvInt));
      assertTrue(staticRInt.adjacent(adjRightIvInt));

   }

   @Test
   public void testAdjacent_NonAdjacentIntervalInt_ShouldBeFalse() {

      IntervalInt adjLeftIvInt = new IntervalInt(-5, 1, true, true);
      IntervalInt adjRightIvInt = new IntervalInt(15, 20, true, true);

      assertFalse(staticRInt.adjacent(adjLeftIvInt));
      assertFalse(staticRInt.adjacent(adjRightIvInt));

   }

   @Test
   public void testRightAdjacent_AdjacentIntervalInt_ShouldBeTrue() {

      IntervalInt ivInt = new IntervalInt(15, 20, false, true);

      assertTrue(staticRInt.rightAdjacent(ivInt));

   }

   @Test
   public void testRightAdjacent_NonAdjacentIntervalInt_ShouldBeFalse() {

      IntervalInt ivInt1 = new IntervalInt(16, 20, false, true);
      IntervalInt ivInt2 = new IntervalInt(17, 20, true, true);

      assertFalse(staticRInt.rightAdjacent(ivInt1));
      assertFalse(staticRInt.rightAdjacent(ivInt2));

   }

   @Test
   public void testLeftAdjacent_AdjacentIntervalInt_ShouldBeTrue() {

      IntervalInt ivInt = new IntervalInt(-5, 1, true, false);

      assertTrue(staticRInt.leftAdjacent(ivInt));

   }

   @Test
   public void testLeftAdjacent_NonAdjacentIntervalInt_ShouldBeFalse() {

      IntervalInt ivInt1 = new IntervalInt(-5, 0, true, false);
      IntervalInt ivInt2 = new IntervalInt(-5, -1, true, true);

      assertFalse(staticRInt.leftAdjacent(ivInt1));
      assertFalse(staticRInt.leftAdjacent(ivInt2));

   }

   @Test
   public void testBefore_GreaterIntervalInt_ShouldBeTrue() {

      IntervalInt ivInt = new IntervalInt(20, 30, true, true);

      assertTrue(staticRInt.before(ivInt));

   }

   @Test
   public void testBefore_AdjacentIntervalInt_ShouldBeTrue() {

      IntervalInt ivInt = new IntervalInt(15, 20, false, true);

      assertTrue(staticRInt.before(ivInt));

   }

   @Test
   public void testBefore_IntersectingIntervalInt_ShouldBeFalse() {

      IntervalInt ivInt1 = new IntervalInt(15, 30, true, true);
      IntervalInt ivInt2 = new IntervalInt(10, 20, true, true);

      assertFalse(staticRInt.before(ivInt1));
      assertFalse(staticRInt.before(ivInt2));

   }

   @Test
   public void testAfter_LowerIntervalInt_ShouldBeTrue() {

      IntervalInt ivInt = new IntervalInt(-10, -5, true, true);

      assertTrue(staticRInt.after(ivInt));

   }

   @Test
   public void testAfter_AdjacentIntervalInt_ShouldBeTrue() {

      IntervalInt ivInt = new IntervalInt(-5, 1, true, false);

      assertTrue(staticRInt.after(ivInt));

   }

   @Test
   public void testAfter_IntersectingIntervalInt_ShouldBeFalse() {

      IntervalInt ivInt1 = new IntervalInt(-5, 1, true, true);
      IntervalInt ivInt2 = new IntervalInt(-5, 5, true, true);

      assertFalse(staticRInt.after(ivInt1));
      assertFalse(staticRInt.after(ivInt2));

   }

   @Test
   public void testContains_BaseIntLiesInRange_ShouldBeTrue() {

      BaseIntIF bInt1 = new BaseInt(3);
      BaseIntIF bInt2 = new BaseInt(12);

      assertTrue(staticRInt.contains(bInt1));
      assertTrue(staticRInt.contains(bInt2));

   }

   @Test
   public void testContains_BaseIntLiesNotInRange_ShouldBeFalse() {

      BaseIntIF bInt = new BaseInt(100);

      assertFalse(staticRInt.contains(bInt));

   }

   @Test
   public void testGetMinValue() {
      BaseIntIF expectedBInt = staticRInt.first().getLowerBound();

      assertEquals(expectedBInt, staticRInt.getMinValue());
   }

   @Test
   public void testGetMinValue_EmptyRangeInt_ResultShouldBeUndefined() {
      RangeInt emptyRangeInt = new RangeInt(0);
      BaseIntIF resultBInt = emptyRangeInt.getMinValue();

      assertFalse(resultBInt.isDefined());
   }

   @Test
   public void testGetMaxValue() {
      BaseIntIF expectedBInt = staticRInt.last().getUpperBound();

      assertEquals(expectedBInt, staticRInt.getMaxValue());
   }

   @Test
   public void testGetMaxValue_EmptyRangeInt_ResultShouldBeUndefined() {
      RangeInt emptyRangeInt = new RangeInt(0);
      BaseIntIF resultBInt = emptyRangeInt.getMaxValue();

      assertFalse(resultBInt.isDefined());
   }

   @Test
   public void testFirst() {
      RangeInt rInt = new RangeInt(3);

      IntervalInt ivInt1 = new IntervalInt(1, 5, true, true);
      IntervalInt ivInt2 = new IntervalInt(7, 10, true, true);
      IntervalInt ivInt3 = new IntervalInt(12, 15, true, true);

      rInt.add(ivInt1);
      rInt.add(ivInt2);
      rInt.add(ivInt3);

      assertEquals(ivInt1, rInt.first());
   }

   @Test
   public void testLast() {
      RangeInt rInt = new RangeInt(3);

      IntervalInt ivInt1 = new IntervalInt(1, 5, true, true);
      IntervalInt ivInt2 = new IntervalInt(7, 10, true, true);
      IntervalInt ivInt3 = new IntervalInt(12, 15, true, true);

      rInt.add(ivInt1);
      rInt.add(ivInt2);
      rInt.add(ivInt3);

      assertEquals(ivInt3, rInt.last());
   }

   @Test
   public void testGet() {
      RangeInt rInt = new RangeInt(3);

      IntervalInt ivInt1 = new IntervalInt(1, 5, true, true);
      IntervalInt ivInt2 = new IntervalInt(7, 10, true, true);
      IntervalInt ivInt3 = new IntervalInt(12, 15, true, true);

      rInt.add(ivInt1);
      rInt.add(ivInt2);
      rInt.add(ivInt3);

      assertEquals(ivInt2, rInt.get(1));
   }

   @Test
   public void testGetNoComponents_AddingIntervalInt_NoComponentsShouldIncrease() {
      RangeInt rInt = new RangeInt(5);

      int noComponentsBeforeAdd = rInt.getNoComponents();

      rInt.add(new IntervalInt(1, 5, true, true));

      int noComponentsAfterAdd = rInt.getNoComponents();

      assertEquals(0, noComponentsBeforeAdd);
      assertEquals(1, noComponentsAfterAdd);

   }

   @Test
   public void testIsEmpty_NewRangeIntObject_ShouldBeEmpty() {
      RangeInt rInt = new RangeInt(1);

      assertTrue(rInt.isEmpty());
   }

   @Test
   public void testIsEmpty_NewRangeIntObjectAddPeriod_ShouldNotBeEmpty() {
      RangeInt rInt = new RangeInt(1);

      rInt.add(new IntervalInt(1, 5, true, true));

      assertFalse(rInt.isEmpty());
   }
}
