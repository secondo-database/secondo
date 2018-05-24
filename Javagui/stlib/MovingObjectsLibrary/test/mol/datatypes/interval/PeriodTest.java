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

import mol.datatypes.time.TimeInstant;
import mol.util.TestUtilData;

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

   private static Period      closedPeriod_From_D05_To_D10;
   private static Period      openPeriod_From_D05_To_D10;

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

   @Test(expected = IllegalArgumentException.class)
   public void testPeriod_LowerBoundGreaterUpperBound_ThrowException() {
      @SuppressWarnings("unused")
      Period period = new Period(instant_D05, instant_D04, true, true);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testPeriod_OpenIntervalLowerBoundEqualUpperBound_ThrowException() {
      @SuppressWarnings("unused")
      Period period = new Period(instant_D10, instant_D04, false, false);
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
}
