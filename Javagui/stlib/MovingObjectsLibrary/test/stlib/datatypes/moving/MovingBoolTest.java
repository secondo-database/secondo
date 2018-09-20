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

package stlib.datatypes.moving;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import stlib.datatypes.base.BaseBool;
import stlib.datatypes.interval.Period;
import stlib.datatypes.moving.MovingBool;
import stlib.datatypes.time.TimeInstant;
import stlib.datatypes.unit.UnitBool;
import stlib.interfaces.base.BaseBoolIF;
import stlib.interfaces.intime.IntimeIF;
import stlib.interfaces.unit.UnitBoolIF;

/**
 * Tests for the 'MovingBool' class
 * 
 * @author Markus Fuessel
 */
public class MovingBoolTest {

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
   public void testAdd_RightAdjacentUnitBool_SuccessfulAppend() {
      int sizeBefore = mbool.getNoUnits();

      UnitBoolIF ubool = new UnitBool(new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false),
            false);

      mbool.add(ubool);

      int sizeAfter = mbool.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_UnitBoolWithLaterPeriod_SuccessfulAppend() {
      int sizeBefore = mbool.getNoUnits();

      UnitBoolIF ubool = new UnitBool(new Period("2018-02-15 00:00:00:000", "2018-02-30 00:00:00:000", true, false),
            new BaseBool(false));

      mbool.add(ubool);

      int sizeAfter = mbool.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_IntersectingUnitBool_FailedAppend() {
      int sizeBefore = mbool.getNoUnits();

      UnitBoolIF ubool = new UnitBool(new Period("2018-01-15 00:00:00:000", "2018-02-20 00:00:00:000", true, false),
            false);

      boolean successful = mbool.add(ubool);

      int sizeAfter = mbool.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);

   }

   @Test
   public void testAdd_UnitBoolWithUndefinedPeriod_AppendFailed() {
      int sizeBefore = mbool.getNoUnits();

      boolean successful = mbool.add(new Period(), false);

      int sizeAfter = mbool.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);

   }

   @Test
   public void testAdd_PeriodWithBaseBoolWithLaterPeriod_SuccessfulAppend() {
      int sizeBefore = mbool.getNoUnits();

      mbool.add(new Period("2018-02-15 00:00:00:000", "2018-02-30 00:00:00:000", true, false), new BaseBool(false));

      int sizeAfter = mbool.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_BaseBoolWithEqualValueWithAdjacentTimeperiod_SuccessfulWithoutIncreaseNumberOfUnits() {
      int sizeBefore = mbool.getNoUnits();

      UnitBoolIF lastUB = mbool.getUnit(sizeBefore - 1);

      boolean successful = mbool.add(new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false),
            lastUB.getValue());

      int sizeAfter = mbool.getNoUnits();

      assertTrue(successful);
      assertEquals(sizeBefore, sizeAfter);
   }

   @Test
   public void testAdd_RightAdjacentMovingBool_SuccessfulAppend() {
      MovingBool mbool2 = new MovingBool(0);

      mbool2.add(new Period("2018-01-30 00:00:00:000", "2018-02-10 00:00:00:000", true, false), true);
      mbool2.add(new Period("2018-02-10 00:00:00:000", "2018-02-20 00:00:00:000", true, false), false);
      mbool2.add(new Period("2018-02-20 00:00:00:000", "2018-02-28 00:00:00:000", true, false), true);

      int sizeMboolBefore = mbool.getNoUnits();
      int sizeMbool2Before = mbool2.getNoUnits();

      boolean successful = mbool.add(mbool2);

      int sizeMBoolAfter = mbool.getNoUnits();

      assertTrue(successful);
      assertEquals(sizeMboolBefore + sizeMbool2Before - 1, sizeMBoolAfter);
   }

   @Test
   public void testAdd_MovingBoolWithLaterPeriod_SuccessfulAppend() {
      MovingBool mbool2 = new MovingBool(0);

      mbool2.add(new Period("2018-01-31 00:00:00:000", "2018-02-10 00:00:00:000", true, false), true);
      mbool2.add(new Period("2018-02-10 00:00:00:000", "2018-02-20 00:00:00:000", true, false), false);
      mbool2.add(new Period("2018-02-20 00:00:00:000", "2018-02-28 00:00:00:000", true, false), true);

      int sizeMboolBefore = mbool.getNoUnits();
      int sizeMbool2Before = mbool2.getNoUnits();

      boolean successful = mbool.add(mbool2);

      int sizeMBoolAfter = mbool.getNoUnits();

      assertTrue(successful);
      assertEquals(sizeMboolBefore + sizeMbool2Before, sizeMBoolAfter);

   }

   @Test
   public void testAdd_IntersectingMovingBool_FailedAppend() {
      MovingBool mbool2 = new MovingBool(0);

      mbool2.add(new Period("2018-01-29 23:59:59:999", "2018-02-10 00:00:00:000", true, false), true);
      mbool2.add(new Period("2018-02-10 00:00:00:000", "2018-02-20 00:00:00:000", true, false), false);
      mbool2.add(new Period("2018-02-20 00:00:00:000", "2018-02-28 00:00:00:000", true, false), true);

      int sizeMboolBefore = mbool.getNoUnits();

      boolean successful = mbool.add(mbool2);

      int sizeMBoolAfter = mbool.getNoUnits();

      assertFalse(successful);
      assertEquals(sizeMboolBefore, sizeMBoolAfter);
   }

   @Test
   public void testAdd_UndefinedMovingBool_AppendFailed() {
      MovingBool mbool2 = new MovingBool(0);

      mbool2.add(new Period("2018-01-31 00:00:00:000", "2018-02-10 00:00:00:000", true, false), true);
      mbool2.add(new Period("2018-02-10 00:00:00:000", "2018-02-20 00:00:00:000", true, false), false);
      mbool2.add(new Period("2018-02-20 00:00:00:000", "2018-02-28 00:00:00:000", true, false), true);

      mbool2.setDefined(false);

      int sizeMboolBefore = mbool.getNoUnits();

      boolean successful = mbool.add(mbool2);

      int sizeMBoolAfter = mbool.getNoUnits();

      assertFalse(successful);
      assertEquals(sizeMboolBefore, sizeMBoolAfter);

   }

   @Test
   public void testPresent_PassDefinedTimeInstantInsideDefinedPeriod_ShouldBeTrue() {

      assertTrue(mbool.present(new TimeInstant("2018-01-20 00:00:00:000")));
      assertTrue(mbool.present(new TimeInstant("2018-01-25 00:00:00:000")));
   }

   @Test
   public void testPresent_PassDefinedTimeInstantOutsideDefinedPeriod_ShouldBeFalse() {

      assertFalse(mbool.present(new TimeInstant("2017-12-31 23:59:59:999")));
      assertFalse(mbool.present(new TimeInstant("2018-01-30 00:00:00:000")));
   }

   @Test
   public void testPresent_PassUndefinedTimeInstant_ShouldBeFalse() {

      assertFalse(mbool.present(new TimeInstant()));
   }

   @Test
   public void testAtInstant_DefinedInstantInsideDefinedPeriod_ShouldReturnDefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool = mbool.atInstant(new TimeInstant("2018-01-20 00:00:00:000"));

      assertTrue(ibool.isDefined());
      assertEquals(new BaseBool(true), ibool.getValue());
   }

   @Test
   public void testAtInstant_DefinedInstantOutsideDefinedPeriod_ShouldReturnUndefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool1 = mbool.atInstant(new TimeInstant("2017-12-31 23:59:59:999"));
      IntimeIF<BaseBoolIF> ibool2 = mbool.atInstant(new TimeInstant("2018-02-30 00:00:00:000"));

      assertFalse(ibool1.isDefined());
      assertFalse(ibool2.isDefined());
   }

   @Test
   public void testAtInstant_UndefinedInstant_ShouldReturnUndefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool = mbool.atInstant(new TimeInstant());

      assertFalse(ibool.isDefined());
   }

   @Test
   public void testGetValue_DefinedInstantInsideDefinedPeriod_ShouldReturnDefinedBaseBool() {

      BaseBoolIF bBool = mbool.getValue(new TimeInstant("2018-01-20 00:00:00:000"));

      assertTrue(bBool.isDefined());
      assertEquals(new BaseBool(true), bBool);
   }

   @Test
   public void testGetValue_DefinedInstantOutsideDefinedPeriod_ShouldReturnUndefinedBaseBool() {

      BaseBoolIF bBool1 = mbool.getValue(new TimeInstant("2017-12-31 23:59:59:999"));
      BaseBoolIF bBool2 = mbool.getValue(new TimeInstant("2018-02-30 00:00:00:000"));

      assertFalse(bBool1.isDefined());
      assertFalse(bBool2.isDefined());
   }

   @Test
   public void testGetValue_UndefinedInstant_ShouldReturnUndefinedBaseBool() {

      BaseBoolIF bBool = mbool.getValue(new TimeInstant());

      assertFalse(bBool.isDefined());
   }

   @Test
   public void testGetUnit_ValidPosition_ShouldReturnDefinedUnit() {

      UnitBoolIF uBool = mbool.getUnit(mbool.getNoUnits() - 1);

      assertTrue(uBool.isDefined());

   }

   @Test
   public void testGetUnit_InvalidPosition_ShouldReturnUndefinedUnit() {

      UnitBoolIF uBool = mbool.getUnit(mbool.getNoUnits());

      assertFalse(uBool.isDefined());

   }

   @Test
   public void testIsClosed_AddClosedUnit_ShouldBeTrue() {

      boolean success = mbool.add(new Period("2018-02-15 00:00:00:000", "2018-02-30 00:00:00:000", true, true),
            new BaseBool(false));

      assertTrue(success);
      assertTrue(mbool.isClosed());

   }

   @Test
   public void testIsClosed_OpenMovingBool_ShouldBeFalse() {

      assertFalse(mbool.isClosed());

   }

   @Test
   public void testIsClosed_EmptyMovingBool_ShouldBeFalse() {
      MovingBool emptyMBool = new MovingBool(0);

      assertFalse(emptyMBool.isClosed());

   }
}
