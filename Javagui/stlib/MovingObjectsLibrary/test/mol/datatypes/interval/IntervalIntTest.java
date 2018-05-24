//This file is part of SECONDO.

//Copyright (C) 214, University in Hagen, Department of Computer Science,
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

package mol.datatypes.interval;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import org.junit.BeforeClass;
import org.junit.Test;

import mol.datatypes.interval.IntervalInt;

/**
 * Tests for class 'IntervalInt'
 * <p>
 * Here only a few tests have been defined here for methods of the class
 * 'IntervalInt' which must be tested separately for integers
 * 
 * @author Markus Fuessel
 *
 */
public class IntervalIntTest {

   static IntervalInt lOpen1rOpen5 = new IntervalInt(1, 5, false, false); // (1, 5)
   static IntervalInt lOpen3rOpen12 = new IntervalInt(3, 12, false, false); // (3, 12)
   static IntervalInt lOpen4rOpen11 = new IntervalInt(4, 11, false, false); // (4, 11)
   static IntervalInt lOpen5rOpen10 = new IntervalInt(5, 10, false, false); // (5, 10)
   static IntervalInt lOpen10rOpen15 = new IntervalInt(10, 15, false, false); // (10, 15)

   static IntervalInt lOpen1rClosed5 = new IntervalInt(1, 5, false, true); // (1, 5]
   static IntervalInt lOpen5rClosed10 = new IntervalInt(5, 10, false, true); // (5, 10]
   static IntervalInt lOpen10rClosed15 = new IntervalInt(10, 15, false, true); // (10, 15]

   static IntervalInt lClosed1rOpen5 = new IntervalInt(1, 5, true, false); // [1, 5)
   static IntervalInt lClosed5rOpen10 = new IntervalInt(5, 10, true, false); // [5, 10)
   static IntervalInt lClosed10rOpen15 = new IntervalInt(10, 15, true, false); // [10, 15)

   static IntervalInt lClosed1rClosed5 = new IntervalInt(1, 5, true, true); // [1, 5]
   static IntervalInt lClosed1rClosed7 = new IntervalInt(1, 7, true, true); // [1, 7]
   static IntervalInt lClosed1rClosed10 = new IntervalInt(1, 10, true, true); // [1, 10]
   static IntervalInt lClosed1rClosed15 = new IntervalInt(1, 15, true, true); // [1, 15]
   static IntervalInt lClosed5rClosed10 = new IntervalInt(5, 10, true, true); // [5, 10]
   static IntervalInt lClosed5rClosed15 = new IntervalInt(5, 15, true, true); // [5, 15]
   static IntervalInt lClosed6rClosed10 = new IntervalInt(6, 10, true, true); // [6, 10]
   static IntervalInt lClosed10rClosed15 = new IntervalInt(10, 15, true, true); // [10, 15]
   static IntervalInt lClosed11rClosed15 = new IntervalInt(11, 15, true, true); // [11, 15]

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {

   }

   @Test
   public void testLeftAdjacent_TwoDisjointAdjacentIntervalInt_ShouldReturnTrue() {

      // [1, 5)[5, 10)
      assertTrue(lClosed5rOpen10.leftAdjacent(lClosed1rOpen5));

      // (1, 5](5, 10]
      assertTrue(lOpen5rClosed10.leftAdjacent(lOpen1rClosed5));

   }

   /**
    * Special case for integers. [1, 5][6, 10] are adjacent, cause 6 is the
    * successor of 5
    */
   @Test
   public void testLeftAdjacent_TwoDisjointSuccessiveClosedIntervalInt_ShouldReturnTrue() {

      // [1, 5][6, 10]
      assertTrue(lClosed6rClosed10.leftAdjacent(lClosed1rClosed5));

   }

   @Test
   public void testLeftAdjacent_TwoDisjointSuccessiveOpenIntervalInt_ShouldReturnFalse() {

      // (1, 5)(5, 10)
      assertFalse(lOpen5rOpen10.leftAdjacent(lOpen1rOpen5));

   }

   @Test
   public void testLeftAdjacent_TwoNonDisjointIntervalInt_ShouldReturnFalse() {

      // [1, 5][5, 10]
      assertFalse(lClosed5rClosed10.leftAdjacent(lClosed1rClosed5));

      // [1, 7][5, 10]
      assertFalse(lClosed5rClosed10.leftAdjacent(lClosed1rClosed7));

   }

   @Test
   public void testRightAdjacent_TwoDisjointAdjacentIntervalInt_ShouldReturnTrue() {

      // [1, 5)[5, 10)
      assertTrue(lClosed1rOpen5.rightAdjacent(lClosed5rOpen10));

      // (1, 5](5, 10]
      assertTrue(lOpen1rClosed5.rightAdjacent(lOpen5rClosed10));

   }

   /**
    * Special case for integers. [1, 5][6, 10] are adjacent, cause 6 is the
    * successor of 5
    */
   @Test
   public void testRightAdjacent_TwoDisjointSuccessiveClosedIntervalInt_ShouldReturnTrue() {

      // [1, 5][6, 10]
      assertTrue(lClosed1rClosed5.rightAdjacent(lClosed6rClosed10));

   }

   @Test
   public void testRightAdjacent_TwoDisjointSuccessiveOpenIntervalInt_ShouldReturnFalse() {

      // (1, 5)(5, 10)
      assertFalse(lOpen1rOpen5.rightAdjacent(lOpen5rOpen10));

   }

   @Test
   public void testRightAdjacent_TwoNonDisjointIntervalInt_ShouldReturnFalse() {

      // [1, 5][5, 10]
      assertFalse(lClosed1rClosed5.rightAdjacent(lClosed5rClosed10));

      // [1, 7][5, 10]
      assertFalse(lClosed1rClosed7.rightAdjacent(lClosed5rClosed10));

   }

   @Test
   public void testAdjacent_DisjointAdjacentIntervalInt_ShouldReturnTrue() {

      // [1, 5)[5, 10)[5, 15)
      assertTrue(lClosed5rOpen10.adjacent(lClosed1rOpen5));
      assertTrue(lClosed5rOpen10.adjacent(lClosed10rOpen15));

      // (1, 5](5, 10](10, 15]
      assertTrue(lOpen5rClosed10.adjacent(lOpen1rClosed5));
      assertTrue(lOpen5rClosed10.adjacent(lOpen10rClosed15));

   }

   @Test
   public void testAdjacent_DisjointAlmostAdjacentIntervalInt_ShouldReturnFalse() {

      // (1, 5)(5, 10)(5, 15)
      assertFalse(lOpen5rOpen10.adjacent(lOpen1rOpen5));
      assertFalse(lOpen5rOpen10.adjacent(lOpen10rOpen15));

   }

   /**
    * Special case for integers.
    * <p>
    * (1, 5)(4, 11)(10, 15) are adjacent
    * <p>
    * [1, 5][6, 10][11, 15] are adjacent
    */
   @Test
   public void testAdjacent_DisjointSuccessiveIntervalInt_ShouldReturnTrue() {

      // (1, 5)(4, 11)(10, 15)
      assertTrue(lOpen4rOpen11.adjacent(lOpen1rOpen5));
      assertTrue(lOpen4rOpen11.adjacent(lOpen10rOpen15));

      // [1, 5][6, 10][11, 15]
      assertTrue(lClosed6rClosed10.adjacent(lClosed1rClosed5));
      assertTrue(lClosed6rClosed10.adjacent(lClosed11rClosed15));
   }

   @Test
   public void testAdjacent_NonDisjointIntervalInt_ShouldReturnFalse() {

      // (1, 5)(3, 12)(10, 15)
      assertFalse(lOpen3rOpen12.adjacent(lOpen1rOpen5));
      assertFalse(lOpen3rOpen12.adjacent(lOpen10rOpen15));

      // [1, 5][5, 10][10, 15]
      assertFalse(lClosed5rClosed10.adjacent(lClosed1rClosed5));
      assertFalse(lClosed5rClosed10.adjacent(lClosed10rClosed15));

   }

   @Test
   public void testBefore_IntervalIntBeforeOther_ShouldBeTrue() {

      assertTrue(lClosed1rClosed5.before(lClosed10rClosed15));
   }

   @Test
   public void testBefore_IntervalIntAdjacentToOther_ShouldBeTrue() {
      assertTrue(lClosed1rClosed5.rightAdjacent(lOpen5rClosed10));
      assertTrue(lOpen1rOpen5.rightAdjacent(lClosed5rClosed10));

      assertTrue(lClosed1rClosed5.before(lOpen5rClosed10));
      assertTrue(lOpen1rOpen5.before(lClosed5rClosed10));
   }

   @Test
   public void testBefore_IntervalIntWithCommonBoundary_ShouldBeFalse() {

      assertFalse(lClosed1rClosed5.before(lClosed5rClosed10));
   }

   @Test
   public void testAfter_IntervalIntAfterOther_ShouldBeTrue() {
      assertTrue(lClosed10rClosed15.after(lClosed1rClosed5));
   }

   @Test
   public void testAfter_IntervalIntAdjacentToOther_ShouldBeTrue() {

      assertTrue(lOpen5rClosed10.leftAdjacent(lClosed1rClosed5));
      assertTrue(lClosed5rClosed10.leftAdjacent(lOpen1rOpen5));

      assertTrue(lOpen5rClosed10.after(lClosed1rClosed5));
      assertTrue(lClosed5rClosed10.after(lOpen1rOpen5));
   }

   @Test
   public void testAfter_IntervalIntWithCommonBoundary_ShouldBeFalse() {
      assertFalse(lClosed5rClosed10.after(lClosed1rClosed5));
   }

   @Test
   public void testIntersects_IntervalIntHalfInsideOtherIntervalInt_ShouldReturnTrue() {

      assertTrue(lOpen1rOpen5.intersects(lOpen3rOpen12));
      assertTrue(lClosed1rClosed7.intersects(lClosed5rClosed10));
   }

   @Test
   public void testIntersects_IntervalIntCompleteInsideOtherIntervalInt_ShouldReturnTrue() {

      assertTrue(lClosed1rClosed15.intersects(lClosed5rClosed10));
   }

   @Test
   public void testIntersects_IntervalIntWithEqualEndpoints_ShouldReturnTrue() {

      assertTrue(lClosed1rClosed5.intersects(lClosed5rClosed10));
   }

   @Test
   public void testIntersects_IntervalIntOutsideOtherIntervalInt_ShouldReturnFalse() {
      assertFalse(lClosed1rClosed5.intersects(lClosed10rOpen15));
   }

   @Test
   public void testIntersectsLeft_IntervalIntOverlappsLeftWithOther_ShouldBeTrue() {

      assertTrue(lClosed5rClosed10.intersectsLeft(lClosed1rClosed7));
   }

   @Test
   public void testIntersectsLeft_CommonLeftBound_ShouldBeFalse() {

      assertFalse(lClosed1rClosed15.intersectsLeft(lClosed1rClosed5));
   }

   @Test
   public void testIntersectsLeft_IntervalIntLeftAdjacentWithOther_ShouldBeFalse() {

      assertFalse(lClosed5rClosed10.intersectsLeft(lClosed1rOpen5));
   }

   @Test
   public void testIntersectsLeft_IntervalIntInsideOther_ShouldBeFalse() {
      assertFalse(lClosed1rClosed15.intersectsLeft(lClosed6rClosed10));
   }

   @Test
   public void testIntersectsRight_IntervalIntOverlappsRightWithOther_ShouldBeTrue() {

      assertTrue(lClosed1rClosed7.intersectsRight(lClosed5rClosed10));
   }

   @Test
   public void testIntersectsRight_CommonRightBound_ShouldBeFalse() {

      assertFalse(lClosed1rClosed15.intersectsRight(lClosed10rClosed15));
   }

   @Test
   public void testIntersectsRight_IntervalIntRightAdjacentWithOther_ShouldBeFalse() {

      assertFalse(lClosed1rOpen5.intersectsRight(lClosed5rClosed10));
   }

   @Test
   public void testIntersectsRight_IntervalIntInsideOther_ShouldBeFalse() {
      assertFalse(lClosed1rClosed15.intersectsRight(lClosed6rClosed10));
   }

   @Test
   public void testMerge_MergeTwoIntervalInt_ShouldBeSuccessful() {

      IntervalInt mergedIntervalIntA = (IntervalInt) lClosed1rClosed5.merge(lClosed10rClosed15);
      IntervalInt mergedIntervalIntB = (IntervalInt) lClosed10rClosed15.merge(lClosed1rClosed5);
      IntervalInt mergedIntervalIntC = (IntervalInt) lClosed1rClosed15.merge(lClosed1rClosed5);

      assertTrue(mergedIntervalIntA.equals(lClosed1rClosed15));
      assertTrue(mergedIntervalIntB.equals(lClosed1rClosed15));
      assertTrue(mergedIntervalIntC.equals(lClosed1rClosed15));
   }

   @Test
   public void testLeftMerge_LeftIntersectingIntervalInt_MergedIntervalIntWithNewLowerBound() {

      IntervalInt mergedIntervalInt = (IntervalInt) lClosed5rClosed10.mergeLeft(lClosed1rClosed7);

      assertTrue(mergedIntervalInt.equals(lClosed1rClosed10));
   }

   @Test
   public void testLeftMerge_WithEnclosingIntervalInt_MergedIntervalIntWithNewLowerBound() {
      IntervalInt mergedIntervalInt = (IntervalInt) lClosed5rClosed10.mergeLeft(lClosed1rClosed15);

      assertTrue(mergedIntervalInt.equals(lClosed1rClosed10));
   }

   @Test
   public void testLeftMerge_RightIntersectingIntervalInt_MergedIntervalIntEqualToOriginal() {
      IntervalInt mergedIntervalInt = (IntervalInt) lClosed1rClosed7.mergeLeft(lClosed1rClosed15);

      assertTrue(mergedIntervalInt.equals(lClosed1rClosed7));
   }

   @Test
   public void testLeftMerge_WithIncludedIntervalInt_MergedIntervalIntEqualToOriginal() {
      IntervalInt mergedIntervalInt = (IntervalInt) lClosed1rClosed15.mergeLeft(lClosed5rClosed10);

      assertTrue(mergedIntervalInt.equals(lClosed1rClosed15));
   }

   @Test
   public void testRightMerge_RightIntersectingIntervalInt_MergedIntervalIntWithNewUpperBound() {

      IntervalInt mergedIntervalInt = (IntervalInt) lClosed1rClosed7.mergeRight(lClosed5rClosed10);

      assertTrue(mergedIntervalInt.equals(lClosed1rClosed10));
   }

   @Test
   public void testRightMerge_WithEnclosingIntervalInt_MergedIntervalIntWithNewUpperBound() {
      IntervalInt mergedIntervalInt = (IntervalInt) lClosed5rClosed10.mergeRight(lClosed1rClosed15);

      assertTrue(mergedIntervalInt.equals(lClosed5rClosed15));
   }

   @Test
   public void testRightMerge_LeftIntersectingIntervalInt_MergedIntervalIntEqualToOriginal() {
      IntervalInt mergedIntervalInt = (IntervalInt) lClosed5rClosed10.mergeRight(lClosed1rClosed7);

      assertTrue(mergedIntervalInt.equals(lClosed5rClosed10));
   }

   @Test
   public void testRightMerge_WithIncludedIntervalInt_MergedIntervalIntEqualToOriginal() {
      IntervalInt mergedIntervalInt = (IntervalInt) lClosed1rClosed15.mergeRight(lClosed5rClosed10);

      assertTrue(mergedIntervalInt.equals(lClosed1rClosed15));
   }
}
