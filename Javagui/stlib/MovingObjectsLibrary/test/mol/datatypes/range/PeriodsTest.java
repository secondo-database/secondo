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

import java.time.format.DateTimeFormatter;

import org.junit.Before;
import org.junit.Test;

import mol.datatypes.interval.Period;
import mol.datatypes.time.TimeInstant;

/**
 * Tests for class 'Periods'
 * 
 * @author Markus Fuessel
 */
public class PeriodsTest {

   /**
    * [2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999] <br>
    * [2018-01-10 00:00:00:000", "2018-01-19 23:59:59:999] <br>
    * [2018-01-20 00:00:00:000", "2018-01-29 23:59:59:999]
    */
   static Periods periodsOf2018 = new Periods(1);

   @Before
   public void setUp() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);

      periodsOf2018 = new Periods(3);

      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      Period period2 = new Period("2018-01-10 00:00:00:000", "2018-01-19 23:59:59:999", true, true);
      Period period3 = new Period("2018-01-20 00:00:00:000", "2018-01-29 23:59:59:999", true, true);

      periodsOf2018.add(period1);
      periodsOf2018.add(period2);
      periodsOf2018.add(period3);
   }

   @Test
   public void testAdd_AddPeriod_NoComponentsShouldIncrease() {

      int noCompBeforeAdd = periodsOf2018.getNoComponents();
      Period period = new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", true, true);

      periodsOf2018.add(period);

      assertEquals(noCompBeforeAdd + 1, periodsOf2018.getNoComponents());
   }

   @Test
   public void testAdd_AddUndefinedIntervalInt_NoComponentsShouldNotIncrease() {

      int noCompBeforeAdd = periodsOf2018.getNoComponents();
      Period period = new Period("2018-03-01 00:00:00:000", "2018-02-09 23:59:59:999", true, true);

      boolean successful = periodsOf2018.add(period);

      assertFalse(successful);
      assertEquals(noCompBeforeAdd, periodsOf2018.getNoComponents());
   }

   @Test
   public void testMergeAdd_AddIntersectingPeriod_NoComponentsShouldNotIncrease() {
      int noCompBeforeMergeAdd = periodsOf2018.getNoComponents();

      Period lastPeriod = periodsOf2018.last();
      Period period = new Period(lastPeriod.getLowerBound().plusMillis(10000),
            lastPeriod.getUpperBound().plusMillis(10000), true, true);

      boolean successful = periodsOf2018.mergeAdd(period);

      assertEquals(noCompBeforeMergeAdd, periodsOf2018.getNoComponents());
      assertTrue(successful);
   }

   @Test
   public void testMergeAdd_AddIntersectingPeriodCommonLeftBound_NoComponentsShouldNotIncrease() {
      int noCompBeforeMergeAdd = periodsOf2018.getNoComponents();

      Period lastPeriod = periodsOf2018.last();
      Period period = new Period(lastPeriod.getLowerBound(), lastPeriod.getUpperBound().plusMillis(10000),
            lastPeriod.isLeftClosed(), true);

      boolean successful = periodsOf2018.mergeAdd(period);

      assertEquals(noCompBeforeMergeAdd, periodsOf2018.getNoComponents());
      assertTrue(successful);
   }

   @Test
   public void testMergeAdd_AddPeriodDifferByOneNanosecond_NoComponentsShouldIncrease() {
      int noCompBeforeMergeAdd = periodsOf2018.getNoComponents();

      Period lastPeriod = periodsOf2018.last();
      Period period = new Period(lastPeriod.getUpperBound().plusNanos(1), lastPeriod.getUpperBound().plusMillis(10000),
            true, true);

      periodsOf2018.mergeAdd(period);

      assertEquals(noCompBeforeMergeAdd + 1, periodsOf2018.getNoComponents());
   }

   @Test
   public void testMergeAdd_AddDisjointPeriod_NoComponentsShouldIncrease() {
      int noCompBeforeMergeAdd = periodsOf2018.getNoComponents();

      Period lastPeriod = periodsOf2018.last();
      Period period = new Period(lastPeriod.getUpperBound().plusMillis(10000),
            lastPeriod.getUpperBound().plusMillis(100000), true, true);

      periodsOf2018.mergeAdd(period);

      assertEquals(noCompBeforeMergeAdd + 1, periodsOf2018.getNoComponents());
   }

   @Test
   public void testMergeAdd_DisjointPeriodBeforePeriods_MergeAddShouldFail() {
      int noCompBeforeMergeAdd = periodsOf2018.getNoComponents();
      Period period = new Period("2017-01-01 00:00:00:000", "2017-01-09 23:59:59:999", true, true);
      boolean successful;

      successful = periodsOf2018.mergeAdd(period);

      assertEquals(noCompBeforeMergeAdd, periodsOf2018.getNoComponents());
      assertFalse(successful);
   }

   @Test
   public void testIntersects_PeriodOverlapsOnePeriodOfRange_ShouldBeTrue() {
      Period period1 = new Period("2018-01-25 00:00:00:000", "2018-02-10 23:59:59:999", true, true);
      Period period2 = new Period("2018-01-20 00:00:00:000", "2018-01-29 23:59:59:999", true, true);

      boolean intersects1 = periodsOf2018.intersects(period1);
      boolean intersects2 = periodsOf2018.intersects(period2);

      assertTrue(intersects1);
      assertTrue(intersects2);
   }

   @Test
   public void testIntersects_PeriodAdjacentToOnePeriodOfRange_ShouldBeFalse() {
      Period period1 = new Period("2017-01-01 00:00:00:000", "2018-01-01 00:00:00:000", true, false);
      Period period2 = new Period("2018-01-29 23:59:59:999", "2018-02-10 23:59:59:999", false, true);

      boolean intersects1 = periodsOf2018.intersects(period1);
      boolean intersects2 = periodsOf2018.intersects(period2);

      assertFalse(intersects1);
      assertFalse(intersects2);
   }

   @Test
   public void testIntersects_PeriodLiesWithinRangeWithoutOverlapping_ShouldBeFalse() {
      Period periodAdd = new Period("2018-03-01 00:00:00:000", "2018-03-10 23:59:59:999", true, true);
      Period periodWithin = new Period("2018-02-01 00:00:00:000", "2018-02-10 23:59:59:999", true, true);

      periodsOf2018.add(periodAdd);

      boolean intersects = periodsOf2018.intersects(periodWithin);

      assertFalse(intersects);
   }

   @Test
   public void testIntersects_UndefinedPeriod_ShouldBeFalse() {
      Period undefinedPeriod = new Period();

      boolean intersects = periodsOf2018.intersects(undefinedPeriod);

      assertFalse(intersects);
   }

   @Test
   public void testIntersects_OverlapsWithOtherPeriod_ShouldBeTrue() {
      Periods otherPeriods = new Periods(0);

      otherPeriods.mergeAdd(new Period("2018-01-29 23:59:59:999", "2018-02-09 23:59:59:999", true, true));
      otherPeriods.mergeAdd(new Period("2018-02-10 00:00:00:000", "2018-02-19 23:59:59:999", true, true));

      assertTrue(periodsOf2018.intersects(otherPeriods));
      assertTrue(otherPeriods.intersects(periodsOf2018));
   }

   @Test
   public void testIntersects_PeriodOfOtherPeriodsCoversCompleteRange_ShouldBeTrue() {
      Periods otherPeriods = new Periods(0);

      otherPeriods.mergeAdd(new Period("2017-01-29 23:59:59:999", "2018-02-09 23:59:59:999", true, true));
      otherPeriods.mergeAdd(new Period("2018-02-10 00:00:00:000", "2018-02-19 23:59:59:999", true, true));

      assertTrue(periodsOf2018.intersects(otherPeriods));
      assertTrue(otherPeriods.intersects(periodsOf2018));
   }

   @Test
   public void testIntersects_OverlapsNotWithOtherPeriod_ShouldBeFalse() {
      Periods otherPeriods = new Periods(0);

      otherPeriods.mergeAdd(new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", true, true));
      otherPeriods.mergeAdd(new Period("2018-02-10 00:00:00:000", "2018-02-19 23:59:59:999", true, true));

      assertFalse(periodsOf2018.intersects(otherPeriods));
      assertFalse(otherPeriods.intersects(periodsOf2018));
   }

   @Test
   public void testIntersects_OtherPeriodsEmpty_ShouldBeFalse() {
      Periods otherPeriods = new Periods(0);

      assertFalse(periodsOf2018.intersects(otherPeriods));
      assertFalse(otherPeriods.intersects(periodsOf2018));
   }

   @Test
   public void testIntersects_OtherRangeUndefined_ShouldBeFalse() {
      Periods otherPeriods = new Periods(0);

      otherPeriods.mergeAdd(new Period("2018-02-01 00:00:00:000", "2018-02-09 23:59:59:999", true, true));
      otherPeriods.mergeAdd(new Period("2018-02-10 00:00:00:000", "2018-02-19 23:59:59:999", true, true));

      otherPeriods.setDefined(false);

      assertFalse(periodsOf2018.intersects(otherPeriods));
      assertFalse(otherPeriods.intersects(periodsOf2018));
   }

   @Test
   public void testAdjacent_AdjacentPeriod_ShouldBeTrue() {

      Period adjLeftPeriod = new Period("2017-01-01 00:00:00:000", "2018-01-01 00:00:00:000", true, false);
      Period adjRightPeriod = new Period("2018-01-29 23:59:59:999", "2018-02-10 23:59:59:999", false, true);

      assertTrue(periodsOf2018.adjacent(adjLeftPeriod));
      assertTrue(periodsOf2018.adjacent(adjRightPeriod));

   }

   @Test
   public void testAdjacent_NonAdjacentPeriod_ShouldBeFalse() {

      Period adjLeftPeriod = new Period("2017-01-01 00:00:00:000", "2018-01-01 00:00:00:000", true, true);
      Period adjRightPeriod = new Period("2018-01-29 23:59:59:999", "2018-02-10 23:59:59:999", true, true);

      assertFalse(periodsOf2018.adjacent(adjLeftPeriod));
      assertFalse(periodsOf2018.adjacent(adjRightPeriod));

   }

   @Test
   public void testRightAdjacent_AdjacentPeriod_ShouldBeTrue() {

      Period period = new Period("2018-01-29 23:59:59:999", "2018-02-10 23:59:59:999", false, true);

      assertTrue(periodsOf2018.rightAdjacent(period));

   }

   @Test
   public void testRightAdjacent_NonAdjacentPeriod_ShouldBeFalse() {

      Period period1 = new Period("2018-01-30 00:00:00:000", "2018-02-10 23:59:59:999", false, true);
      Period period2 = new Period("2018-01-29 23:59:59:999", "2018-02-10 23:59:59:999", true, true);

      assertFalse(periodsOf2018.rightAdjacent(period1));
      assertFalse(periodsOf2018.rightAdjacent(period2));

   }

   @Test
   public void testLeftAdjacent_AdjacentPeriod_ShouldBeTrue() {

      Period period = new Period("2017-01-01 00:00:00:000", "2018-01-01 00:00:00:000", true, false);

      assertTrue(periodsOf2018.leftAdjacent(period));

   }

   @Test
   public void testLeftAdjacent_NonAdjacentPeriod_ShouldBeFalse() {

      Period period1 = new Period("2017-01-01 00:00:00:000", "2018-01-01 00:00:00:000", true, true);
      Period period2 = new Period("2017-01-01 00:00:00:000", "2017-12-31 23:59:59:999", true, false);

      assertFalse(periodsOf2018.leftAdjacent(period1));
      assertFalse(periodsOf2018.leftAdjacent(period2));

   }

   @Test
   public void testBefore_GreaterPeriod_ShouldBeTrue() {

      Period period = new Period("2018-02-10 00:00:00:000", "2018-02-20 00:00:00:000", true, true);

      assertTrue(periodsOf2018.before(period));

   }

   @Test
   public void testBefore_AdjacentPeriod_ShouldBeTrue() {

      Period period = new Period("2018-01-29 23:59:59:999", "2018-02-20 00:00:00:000", false, true);

      assertTrue(periodsOf2018.before(period));

   }

   @Test
   public void testBefore_IntersectingPeriod_ShouldBeFalse() {

      Period period1 = new Period("2018-01-29 23:59:59:999", "2018-02-20 00:00:00:000", true, true);
      Period period2 = new Period("2018-01-10 00:00:00:000", "2018-02-20 00:00:00:000", true, true);

      assertFalse(periodsOf2018.before(period1));
      assertFalse(periodsOf2018.before(period2));

   }

   @Test
   public void testAfter_LowerPeriod_ShouldBeTrue() {

      Period period = new Period("2017-12-10 00:00:00:000", "2017-12-20 00:00:00:000", true, true);

      assertTrue(periodsOf2018.after(period));

   }

   @Test
   public void testAfter_AdjacentPeriod_ShouldBeTrue() {

      Period period = new Period("2017-12-10 00:00:00:000", "2018-01-01 00:00:00:000", true, false);

      assertTrue(periodsOf2018.after(period));

   }

   @Test
   public void testAfter_IntersectingPeriod_ShouldBeFalse() {

      Period period1 = new Period("2017-12-10 00:00:00:000", "2018-01-01 00:00:00:000", true, true);
      Period period2 = new Period("2017-12-10 00:00:00:000", "2018-01-20 00:00:00:000", true, true);

      assertFalse(periodsOf2018.after(period1));
      assertFalse(periodsOf2018.after(period2));

   }

   @Test
   public void testContains_InstantLiesInRange_ShouldBeTrue() {

      TimeInstant instant1 = new TimeInstant("2018-01-10 00:00:00:000");
      TimeInstant instant2 = new TimeInstant("2018-01-25 00:00:00:000");

      assertTrue(periodsOf2018.contains(instant1));
      assertTrue(periodsOf2018.contains(instant2));

   }

   @Test
   public void testContains_InstantLiesNotInRange_ShouldBeFalse() {

      TimeInstant instant = new TimeInstant("2017-01-10 00:00:00:000");

      assertFalse(periodsOf2018.contains(instant));

   }

   @Test
   public void testGetMinValue() {
      TimeInstant expectedInstant = periodsOf2018.first().getLowerBound();

      assertEquals(expectedInstant, periodsOf2018.getMinValue());
   }

   @Test
   public void testGetMinValue_EmptyPeriods_ResultShouldBeUndefined() {
      Periods emptyPeriods = new Periods(0);
      TimeInstant resultInstant = emptyPeriods.getMinValue();

      assertFalse(resultInstant.isDefined());
   }

   @Test
   public void testGetMaxValue() {
      TimeInstant expectedInstant = periodsOf2018.last().getUpperBound();

      assertEquals(expectedInstant, periodsOf2018.getMaxValue());
   }

   @Test
   public void testGetMaxValue_EmptyPeriods_ResultShouldBeUndefined() {
      Periods emptyPeriods = new Periods(0);
      TimeInstant resultInstant = emptyPeriods.getMaxValue();

      assertFalse(resultInstant.isDefined());
   }

   @Test
   public void testFirst() {
      Periods periods = new Periods(3);

      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      Period period2 = new Period("2018-01-10 00:00:00:000", "2018-01-19 23:59:59:999", true, true);
      Period period3 = new Period("2018-01-20 00:00:00:000", "2018-01-29 23:59:59:999", true, true);

      periods.add(period1);
      periods.add(period2);
      periods.add(period3);

      assertEquals(period1, periods.first());
   }

   @Test
   public void testLast() {
      Periods periods = new Periods(3);

      Period period1 = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      Period period2 = new Period("2018-01-10 00:00:00:000", "2018-01-19 23:59:59:999", true, true);
      Period period3 = new Period("2018-01-20 00:00:00:000", "2018-01-29 23:59:59:999", true, true);

      periods.add(period1);
      periods.add(period2);
      periods.add(period3);

      assertEquals(period3, periods.last());
   }

   @Test
   public void testGetNoComponents_AddingPeriod_NoComponentsShouldIncrease() {
      Periods periods = new Periods(5);

      int noComponentsBeforeAdd = periods.getNoComponents();

      periods.add(new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true));

      int noComponentsAfterAdd = periods.getNoComponents();

      assertEquals(0, noComponentsBeforeAdd);
      assertEquals(1, noComponentsAfterAdd);

   }

   @Test
   public void testIsEmpty_NewPeriodsObject_ShouldBeEmpty() {
      Periods periods = new Periods(1);

      assertTrue(periods.isEmpty());
   }

   @Test
   public void testIsEmpty_NewPeriodsObjectAddPeriod_ShouldNotBeEmpty() {
      Periods periods = new Periods(1);

      periods.add(new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true));

      assertFalse(periods.isEmpty());
   }

}
