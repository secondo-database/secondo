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

package mol.datatypes.interval;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import org.junit.BeforeClass;
import org.junit.Test;

import mol.TestUtil.TestUtilData;
import mol.datatypes.time.TimeInstant;

/**
 * Tests for class 'Period'
 * <p>
 * Some of this tests are generally valid for the methods which are defined in
 * the abstract class {@code 'Interval<T>'}
 * 
 * @author Markus Fuessel
 *
 */
public class PeriodTest {

   private static TimeInstant instant_D04;
   private static TimeInstant instant_D05;
   private static TimeInstant instant_D06;
   private static TimeInstant instant_D10;
   private static TimeInstant instant_D11;

   private static Period closedPeriod_From_D05_To_D10;
   private static Period openPeriod_From_D05_To_D10;

   static Period lOpen1rOpen5 = TestUtilData.getPeriod("01", "05", false, false); // (1, 5)
   static Period lOpen3rOpen12 = TestUtilData.getPeriod("03", "12", false, false); // (3, 12)
   static Period lOpen4rOpen11 = TestUtilData.getPeriod("04", "11", false, false); // (4, 11)
   static Period lOpen5rOpen10 = TestUtilData.getPeriod("05", "10", false, false); // (5, 10)
   static Period lOpen10rOpen15 = TestUtilData.getPeriod("10", "15", false, false); // (10, 15)

   static Period lOpen1rClosed5 = TestUtilData.getPeriod("01", "05", false, true); // (1, 5]
   static Period lOpen5rClosed10 = TestUtilData.getPeriod("05", "10", false, true); // (5, 10]
   static Period lOpen10rClosed15 = TestUtilData.getPeriod("10", "15", false, true); // (10, 15]

   static Period lClosed1rOpen5 = TestUtilData.getPeriod("01", "05", true, false); // [1, 5)
   static Period lClosed5rOpen10 = TestUtilData.getPeriod("05", "10", true, false); // [5, 10)
   static Period lClosed10rOpen15 = TestUtilData.getPeriod("10", "15", true, false); // [10, 15)

   static Period lClosed1rClosed5 = TestUtilData.getPeriod("01", "05", true, true); // [1, 5]
   static Period lClosed1rClosed7 = TestUtilData.getPeriod("01", "07", true, true); // [1, 7]
   static Period lClosed1rClosed10 = TestUtilData.getPeriod("01", "10", true, true); // [1, 10]
   static Period lClosed1rClosed15 = TestUtilData.getPeriod("01", "15", true, true); // [1, 15]
   static Period lClosed5rClosed10 = TestUtilData.getPeriod("05", "10", true, true); // [5, 10]
   static Period lClosed5rClosed15 = TestUtilData.getPeriod("05", "15", true, true); // [5, 15]
   static Period lClosed6rClosed10 = TestUtilData.getPeriod("06", "10", true, true); // [6, 10]
   static Period lClosed10rClosed15 = TestUtilData.getPeriod("10", "15", true, true); // [10, 15]
   static Period lClosed11rClosed15 = TestUtilData.getPeriod("11", "15", true, true); // [11, 15]

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {

      instant_D04 = TestUtilData.getInstant("04");
      instant_D05 = TestUtilData.getInstant("05");
      instant_D06 = TestUtilData.getInstant("06");
      instant_D10 = TestUtilData.getInstant("10");
      instant_D11 = TestUtilData.getInstant("11");

      closedPeriod_From_D05_To_D10 = TestUtilData.getPeriod(instant_D05, instant_D10, true, true);
      openPeriod_From_D05_To_D10 = TestUtilData.getPeriod(instant_D05, instant_D10, false, false);

   }

   @Test
   public void testPeriodConstructor_UndefinedObject() {
      Period undefinedPeriod = new Period();

      assertFalse(undefinedPeriod.isDefined());
   }

   @Test
   public void testPeriodCopyConstructor() {

      Period copyPeriod = closedPeriod_From_D05_To_D10.copy();

      assertEquals(closedPeriod_From_D05_To_D10, copyPeriod);
   }

   @Test
   public void testPeriod_LowerBoundGreaterUpperBound_ShouldBeUndefined() {
      Period period = new Period(instant_D05, instant_D04, true, true);
      assertFalse(period.isDefined());
   }

   @Test
   public void testPeriod_OpenIntervalLowerBoundEqualUpperBound_ShouldBeUndefined() {
      Period period = new Period(instant_D10, instant_D10, false, false);

      assertFalse(period.isDefined());
   }

   @Test
   public void testContains_InstantInsidePeriod_ShouldBeTrue() {

      assertTrue(closedPeriod_From_D05_To_D10.contains(instant_D06));
      assertTrue(closedPeriod_From_D05_To_D10.contains(instant_D05));
      assertTrue(closedPeriod_From_D05_To_D10.contains(instant_D10));

   }

   @Test
   public void testContains_InstantOutsidePeriod_ShouldBeFalse() {

      assertFalse(closedPeriod_From_D05_To_D10.contains(instant_D04));
      assertFalse(closedPeriod_From_D05_To_D10.contains(instant_D11));
      assertFalse(openPeriod_From_D05_To_D10.contains(instant_D05));
      assertFalse(openPeriod_From_D05_To_D10.contains(instant_D10));

   }

   @Test
   public void testContains_InstantOnBoundOfOpenPeriod_IgnoreClosedFlags_ShouldBeTrue() {

      assertTrue(openPeriod_From_D05_To_D10.contains(instant_D05, true));
      assertTrue(openPeriod_From_D05_To_D10.contains(instant_D10, true));

   }

   @Test
   public void testContains_UndefinedInstant_ShouldBeFalse() {

      assertFalse(closedPeriod_From_D05_To_D10.contains(new TimeInstant()));

   }

   @Test
   public void testCompareTo_TwoEqualPeriods_ShouldReturnZero() {

      int resultClosedClosed = closedPeriod_From_D05_To_D10.compareTo(closedPeriod_From_D05_To_D10);
      int resultOpenOpen = openPeriod_From_D05_To_D10.compareTo(openPeriod_From_D05_To_D10);

      assertTrue(resultClosedClosed == 0);
      assertTrue(resultOpenOpen == 0);

   }

   @Test
   public void testCompareTo_EarlierVsLaterPeriod_ShouldReturnValueLowerZero() {

      Period laterPeriod = TestUtilData.getPeriod("06", "10", true, true);

      int resultClosedOpen = closedPeriod_From_D05_To_D10.compareTo(openPeriod_From_D05_To_D10);
      int resultEarlierVsLater = closedPeriod_From_D05_To_D10.compareTo(laterPeriod);

      assertTrue(resultClosedOpen < 0);
      assertTrue(resultEarlierVsLater < 0);

   }

   @Test
   public void testCompareTo_LaterVsEarlierPeriod_ShouldReturnValueGreaterZero() {

      Period laterPeriod = TestUtilData.getPeriod("06", "10", true, true);

      int resultOpenClosed = openPeriod_From_D05_To_D10.compareTo(closedPeriod_From_D05_To_D10);
      int resultLaterVsEarlier = laterPeriod.compareTo(closedPeriod_From_D05_To_D10);

      assertTrue(resultOpenClosed > 0);
      assertTrue(resultLaterVsEarlier > 0);

   }

   @Test
   public void testEquals_TwoEqualPeriods_ShouldBeTrue() {

      Period periodClosed1 = TestUtilData.getPeriod("06", "10", true, true);
      Period periodClosed2 = TestUtilData.getPeriod("06", "10", true, true);

      Period periodOpen1 = TestUtilData.getPeriod("06", "10", false, false);
      Period periodOpen2 = TestUtilData.getPeriod("06", "10", false, false);

      assertTrue(periodClosed1.equals(periodClosed2));
      assertTrue(periodOpen1.equals(periodOpen2));

   }

   @Test
   public void testEquals_TwoNonEqualPeriods_ShouldBeFalse() {

      Period periodClosed1 = TestUtilData.getPeriod("06", "10", true, true);
      Period periodClosed2 = TestUtilData.getPeriod("06", "11", true, true);

      Period periodOpen1 = TestUtilData.getPeriod("06", "10", false, false);
      Period periodOpenClosed2 = TestUtilData.getPeriod("06", "10", false, true);

      assertFalse(periodClosed1.equals(periodClosed2));
      assertFalse(periodOpen1.equals(periodOpenClosed2));

   }

   @Test
   public void testEquals_NullObject_ShouldBeFalse() {
      Object object = null;

      assertFalse(closedPeriod_From_D05_To_D10.equals(object));

   }

   @Test
   public void testEquals_DifferentIntervalObject_ShouldBeFalse() {
      Period periodClosed = TestUtilData.getPeriod("01", "05", true, true);

      Object object = new IntervalInt(1, 5, true, true);

      assertFalse(periodClosed.equals(object));

   }

   @Test
   public void testHashCode_EqualPeriods_HashCodesShouldBeIndentical() {

      Period periodClosed1 = TestUtilData.getPeriod("06", "10", true, true);
      Period periodClosed2 = TestUtilData.getPeriod("06", "10", true, true);

      Period periodOpen1 = TestUtilData.getPeriod("06", "10", false, false);
      Period periodOpen2 = TestUtilData.getPeriod("06", "10", false, false);

      assertEquals(periodClosed1.hashCode(), periodClosed2.hashCode());
      assertEquals(periodOpen1.hashCode(), periodOpen2.hashCode());

   }

   @Test
   public void testHashCode_NonEqualPeriods_HashCodesShouldBeDifferent() {

      Period periodClosed1 = TestUtilData.getPeriod("06", "10", true, true);
      Period periodClosed2 = TestUtilData.getPeriod("06", "11", true, true);

      Period periodOpen1 = TestUtilData.getPeriod("06", "10", false, false);
      Period periodOpenClosed2 = TestUtilData.getPeriod("06", "10", false, true);

      assertNotEquals(periodClosed1.hashCode(), periodClosed2.hashCode());
      assertNotEquals(periodOpen1.hashCode(), periodOpenClosed2.hashCode());

   }

   @Test
   public void testDisjoint_TwoDisjointPeriods_ShouldReturnTrue() {

      Period period1 = TestUtilData.getPeriod("01", "05", true, true);
      Period period2 = TestUtilData.getPeriod("10", "15", true, true);

      assertTrue(period1.disjoint(period2));
      assertTrue(period2.disjoint(period1));

   }

   @Test
   public void testDisjoint_TwoAdjacentDisjointPeriods_ShouldReturnTrue() {

      Period closedPeriod1 = TestUtilData.getPeriod("01", "05", true, true);
      Period leftOpenPeriod2 = TestUtilData.getPeriod("05", "15", false, true);

      assertTrue(closedPeriod1.disjoint(leftOpenPeriod2));
      assertTrue(leftOpenPeriod2.disjoint(closedPeriod1));

   }

   @Test
   public void testDisjoint_TwoNonDisjointPeriods_ShouldReturnFalse() {

      Period period1 = TestUtilData.getPeriod("01", "05", true, true);
      Period period2 = TestUtilData.getPeriod("04", "15", true, true);

      assertFalse(period1.disjoint(period2));
      assertFalse(period2.disjoint(period1));

   }

   @Test
   public void testLeftAdjacent_TwoDisjointAdjacentPeriods_ShouldReturnTrue() {

      Period periodBeforeRightOpen = TestUtilData.getPeriod("01", "05", true, false); // [01, 05)
      Period periodAfterLeftClosed = TestUtilData.getPeriod("05", "10", true, false); // [05, 10)

      Period periodBeforeRightClosed = TestUtilData.getPeriod("01", "05", false, true); // (01, 05]
      Period periodAfterLeftOpen = TestUtilData.getPeriod("05", "10", false, true); // (05, 10]

      // [01, 05)[05, 10)
      assertTrue(periodAfterLeftClosed.leftAdjacent(periodBeforeRightOpen));

      // (01, 05](05, 10]
      assertTrue(periodAfterLeftOpen.leftAdjacent(periodBeforeRightClosed));

   }

   @Test
   public void testLeftAdjacent_TwoDisjointAlmostAdjacentPeriods_ShouldReturnFalse() {

      Period periodBefore = TestUtilData.getPeriod("01", "05", true, true); // [01, 05]
      Period periodAfter = TestUtilData.getPeriod("06", "10", true, true); // [06, 10]

      Period periodBeforeOpen = TestUtilData.getPeriod("01", "05", false, false); // (01, 05)
      Period periodAfterOpen = TestUtilData.getPeriod("05", "10", false, false); // (05, 10)

      // [01, 05][06, 10]
      assertFalse(periodAfter.leftAdjacent(periodBefore));

      // (01, 05)(05, 10)
      assertFalse(periodAfterOpen.leftAdjacent(periodBeforeOpen));

   }

   @Test
   public void testLeftAdjacent_TwoNonDisjointPeriods_ShouldReturnFalse() {

      Period periodBeforeClosed = TestUtilData.getPeriod("01", "05", true, true); // [01, 05]
      Period periodAfterClosed = TestUtilData.getPeriod("05", "10", true, true); // [05, 10]

      Period longPeriodBeforeClosed = TestUtilData.getPeriod("01", "07", true, true); // [01, 07]

      // [01, 05][05, 10]
      assertFalse(periodAfterClosed.leftAdjacent(periodBeforeClosed));

      // [01, 07][05, 10]
      assertFalse(periodAfterClosed.leftAdjacent(longPeriodBeforeClosed));

   }

   @Test
   public void testRightAdjacent_TwoDisjointAdjacentPeriods_ShouldReturnTrue() {

      Period periodBeforeRightOpen = TestUtilData.getPeriod("01", "05", true, false); // [01, 05)
      Period periodAfterLeftClosed = TestUtilData.getPeriod("05", "10", true, false); // [05, 10)

      Period periodBeforeRightClosed = TestUtilData.getPeriod("01", "05", false, true); // (01, 05]
      Period periodAfterLeftOpen = TestUtilData.getPeriod("05", "10", false, true); // (05, 10]

      // [01, 05)[05, 10)
      assertTrue(periodBeforeRightOpen.rightAdjacent(periodAfterLeftClosed));

      // (01, 05](05, 10]
      assertTrue(periodBeforeRightClosed.rightAdjacent(periodAfterLeftOpen));

   }

   @Test
   public void testRightAdjacent_TwoDisjointAlmostAdjacentPeriods_ShouldReturnFalse() {

      Period periodBefore = TestUtilData.getPeriod("01", "05", true, true); // [01, 05]
      Period periodAfter = TestUtilData.getPeriod("06", "10", true, true); // [06, 10]

      Period periodBeforeOpen = TestUtilData.getPeriod("01", "05", false, false); // (01, 05)
      Period periodAfterOpen = TestUtilData.getPeriod("05", "10", false, false); // (05, 10)

      // [01, 05][06, 10]
      assertFalse(periodBefore.rightAdjacent(periodAfter));

      // (01, 05)(05, 10)
      assertFalse(periodBeforeOpen.rightAdjacent(periodAfterOpen));

   }

   @Test
   public void testRightAdjacent_TwoNonDisjointPeriods_ShouldReturnFalse() {

      Period periodBeforeClosed = TestUtilData.getPeriod("01", "05", true, true); // [01, 05]
      Period periodAfterClosed = TestUtilData.getPeriod("05", "10", true, true); // [05, 10]

      Period longPeriodBeforeClosed = TestUtilData.getPeriod("01", "07", true, true); // [01, 07]

      // [01, 05][05, 10]
      assertFalse(periodBeforeClosed.rightAdjacent(periodAfterClosed));

      // [01, 07][05, 10]
      assertFalse(longPeriodBeforeClosed.rightAdjacent(periodAfterClosed));

   }

   @Test
   public void testAdjacent_DisjointAdjacentPeriods_ShouldReturnTrue() {

      Period periodBeforeRightOpen = TestUtilData.getPeriod("01", "05", true, false); // [01, 05)
      Period periodBetweenRightOpen = TestUtilData.getPeriod("05", "10", true, false); // [05, 10)
      Period periodAfterRightOpen = TestUtilData.getPeriod("10", "15", true, false); // [10, 15)

      Period periodBeforeRightClosed = TestUtilData.getPeriod("01", "05", false, true); // (01, 05]
      Period periodBetweenRightClosed = TestUtilData.getPeriod("05", "10", false, true); // (05, 10]
      Period periodAfterRightClosed = TestUtilData.getPeriod("10", "15", false, true); // (10, 15]

      // [01, 05)[05, 10)[05, 15)
      assertTrue(periodBetweenRightOpen.adjacent(periodBeforeRightOpen));
      assertTrue(periodBetweenRightOpen.adjacent(periodAfterRightOpen));

      // (01, 05](05, 10](10, 15]
      assertTrue(periodBetweenRightClosed.adjacent(periodBeforeRightClosed));
      assertTrue(periodBetweenRightClosed.adjacent(periodAfterRightClosed));

   }

   @Test
   public void testAdjacent_DisjointAlmostAdjacentPeriods_ShouldReturnFalse() {

      Period periodBeforeOpen = TestUtilData.getPeriod("01", "05", false, false); // (01, 05)
      Period periodBetweenOpen = TestUtilData.getPeriod("05", "10", false, false); // (05, 10)
      Period periodAfterOpen = TestUtilData.getPeriod("10", "15", false, false); // (10, 15)

      Period periodBeforeClosed = TestUtilData.getPeriod("01", "05", true, true); // [01, 05]
      Period periodBetweenClosed = TestUtilData.getPeriod("06", "10", true, true); // [06, 10]
      Period periodAfterClosed = TestUtilData.getPeriod("11", "15", true, true); // [11, 15]

      // (01, 05)(05, 10)(05, 15)
      assertFalse(periodBetweenOpen.adjacent(periodBeforeOpen));
      assertFalse(periodBetweenOpen.adjacent(periodAfterOpen));

      // [01, 05][06, 10][11, 15]
      assertFalse(periodBetweenClosed.adjacent(periodBeforeClosed));
      assertFalse(periodBetweenClosed.adjacent(periodAfterClosed));

   }

   @Test
   public void testAdjacent_NonDisjointPeriods_ShouldReturnFalse() {

      Period periodBeforeOpen = TestUtilData.getPeriod("01", "05", false, false); // (01, 05)
      Period periodBetweenOpen = TestUtilData.getPeriod("04", "11", false, false); // (04, 11)
      Period periodAfterOpen = TestUtilData.getPeriod("10", "15", false, false); // (10, 15)

      Period periodBeforeClosed = TestUtilData.getPeriod("01", "05", true, true); // [01, 05]
      Period periodBetweenClosed = TestUtilData.getPeriod("05", "10", true, true); // [05, 10]
      Period periodAfterClosed = TestUtilData.getPeriod("10", "15", true, true); // [10, 15]

      // (01, 05)(04, 11)(10, 15)
      assertFalse(periodBetweenOpen.adjacent(periodBeforeOpen));
      assertFalse(periodBetweenOpen.adjacent(periodAfterOpen));

      // [01, 05][05, 10][10, 15]
      assertFalse(periodBetweenClosed.adjacent(periodBeforeClosed));
      assertFalse(periodBetweenClosed.adjacent(periodAfterClosed));

   }

   @Test
   public void testBeforeInterval_WithDisjointPeriodBefore_ShouldBeTrue() {

      Period periodD01D05Open = TestUtilData.getPeriod("01", "05", false, false);
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);

      Period periodD01D05Closed = TestUtilData.getPeriod("01", "05", true, true);
      Period periodD06D10Closed = TestUtilData.getPeriod("06", "10", true, true);

      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);

      assertTrue(periodD01D05Open.before(periodD05D10Open));
      assertTrue(periodD01D05Closed.before(periodD06D10Closed));
      assertTrue(periodD01D05Open.before(periodD05D10Closed));

   }

   @Test
   public void testBeforeInterval_WithIntersectingPeriodBefore_ShouldBeFalse() {

      Period periodD01D06Open = TestUtilData.getPeriod("01", "06", false, false);
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);

      Period periodD01D05Closed = TestUtilData.getPeriod("01", "05", true, true);
      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);

      assertFalse(periodD01D06Open.before(periodD05D10Open));
      assertFalse(periodD01D05Closed.before(periodD05D10Closed));

   }

   @Test
   public void testBeforeValue_WithGreaterValue_ShouldBeTrue() {
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);
      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);
      TimeInstant instantUpperBound = periodD05D10Open.getUpperBound();
      TimeInstant instantUpperBoundPlusOne = periodD05D10Closed.getUpperBound().plusMillis(1);

      assertTrue(periodD05D10Open.before(instantUpperBound));
      assertTrue(periodD05D10Closed.before(instantUpperBoundPlusOne));
   }

   @Test
   public void testBeforeValue_WithNonGreaterValue_ShouldBeFalse() {
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);
      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);
      TimeInstant instantUpperBound = periodD05D10Open.getUpperBound();
      TimeInstant instantUpperBoundMinusOne = periodD05D10Closed.getUpperBound().minusMillis(1);

      assertFalse(periodD05D10Open.before(instantUpperBoundMinusOne));
      assertFalse(periodD05D10Closed.before(instantUpperBound));
   }

   @Test
   public void testAfterInterval_WithDisjointPeriodAfter_ShouldBeTrue() {
      Period periodD01D05Open = TestUtilData.getPeriod("01", "05", false, false);
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);

      Period periodD01D05Closed = TestUtilData.getPeriod("01", "05", true, true);
      Period periodD06D10Closed = TestUtilData.getPeriod("06", "10", true, true);

      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);

      assertTrue(periodD05D10Open.after(periodD01D05Open));
      assertTrue(periodD06D10Closed.after(periodD01D05Closed));
      assertTrue(periodD05D10Closed.after(periodD01D05Open));

   }

   @Test
   public void testAfterInterval_WithIntersectingPeriodAfter_ShouldBeFalse() {

      Period periodD01D06Open = TestUtilData.getPeriod("01", "06", false, false);
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);

      Period periodD01D05Closed = TestUtilData.getPeriod("01", "05", true, true);
      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);

      assertFalse(periodD05D10Open.after(periodD01D06Open));
      assertFalse(periodD05D10Closed.after(periodD01D05Closed));

   }

   @Test
   public void testAfterValue_WithLowerValue_ShouldBeTrue() {
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);
      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);
      TimeInstant instantLowerBound = periodD05D10Open.getLowerBound();
      TimeInstant instantLowerBoundMinusOne = periodD05D10Closed.getLowerBound().minusMillis(1);

      assertTrue(periodD05D10Open.after(instantLowerBound));
      assertTrue(periodD05D10Closed.after(instantLowerBoundMinusOne));
   }

   @Test
   public void testAfterValue_WithNonLowerValue_ShouldBeFalse() {
      Period periodD05D10Open = TestUtilData.getPeriod("05", "10", false, false);
      Period periodD05D10Closed = TestUtilData.getPeriod("05", "10", true, true);
      TimeInstant instantLowerBound = periodD05D10Open.getLowerBound();
      TimeInstant instantLowerBoundPlusOne = periodD05D10Closed.getLowerBound().plusMillis(1);

      assertFalse(periodD05D10Open.after(instantLowerBoundPlusOne));
      assertFalse(periodD05D10Closed.after(instantLowerBound));
   }

   @Test
   public void testGetDurationInMilliseconds() {
      Period periodOneDay = new Period(instant_D04, instant_D05, true, true);
      long dayInMilliSec = 24 * 60 * 60 * 1000;

      long durationInMilliSec = periodOneDay.getDurationInMilliseconds();

      assertEquals(dayInMilliSec, durationInMilliSec);
   }

   @Test
   public void testIntersection_IntersectingPeriod_NewDefinedInterval() {
      Period intersection1 = lClosed1rClosed10.intersection(lClosed5rClosed15);
      Period intersection2 = lClosed5rClosed15.intersection(lClosed1rClosed10);

      assertTrue(intersection1.equals(lClosed5rClosed10));
      assertTrue(intersection2.equals(lClosed5rClosed10));
   }

   @Test
   public void testIntersection_IntervalInside_NewDefinedInterval() {
      Period intersection1 = lClosed1rClosed15.intersection(lClosed5rClosed10);
      Period intersection2 = lClosed5rClosed10.intersection(lClosed1rClosed15);

      assertTrue(intersection1.equals(lClosed5rClosed10));
      assertTrue(intersection2.equals(lClosed5rClosed10));
   }

   @Test
   public void testIntersection_IntersectingOnlyAtBorder_NewDefinedInterval() {
      Period intersection1 = lClosed1rClosed5.intersection(lClosed5rClosed10);
      Period intersection2 = lClosed5rClosed10.intersection(lClosed1rClosed5);

      assertTrue(intersection1.equals(TestUtilData.getPeriod("05", "05", true, true)));
      assertTrue(intersection2.equals(TestUtilData.getPeriod("05", "05", true, true)));
   }

   @Test
   public void testIntersection_IntersectingEqualPeriod_NewDefinedInterval() {
      Period intersection = lClosed1rClosed10.intersection(lClosed1rClosed10);

      assertTrue(intersection.equals(lClosed1rClosed10));
   }

   @Test
   public void testIntersection_IntersectingNotAtBorder_NewUndefinedInterval() {
      Period intersection1 = lOpen1rOpen5.intersection(lOpen5rOpen10);
      Period intersection2 = lOpen5rOpen10.intersection(lOpen1rOpen5);

      assertFalse(intersection1.isDefined());
      assertFalse(intersection2.isDefined());
   }

   @Test
   public void testIntersection_NoIntersection_NewUndefinedInterval() {
      Period intersection1 = lClosed1rClosed5.intersection(lClosed10rClosed15);
      Period intersection2 = lClosed10rClosed15.intersection(lClosed1rClosed5);

      assertFalse(intersection1.isDefined());
      assertFalse(intersection2.isDefined());
   }
}
