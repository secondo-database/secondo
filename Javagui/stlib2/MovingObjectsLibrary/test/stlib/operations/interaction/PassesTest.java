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
package stlib.operations.interaction;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import stlib.datatypes.base.BaseBool;
import stlib.datatypes.interval.Period;
import stlib.datatypes.moving.MovingBool;
import stlib.datatypes.moving.MovingPoint;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.spatial.Region;
import stlib.datatypes.spatial.util.Face;
import stlib.datatypes.time.TimeInstant;
import stlib.datatypes.unit.spatial.UnitPointLinear;
import stlib.interfaces.spatial.PointIF;

/**
 * Tests for 'Passes' class methods
 * 
 * @author Markus Fuessel
 */
public class PassesTest {

   /**
    * [2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000), TRUE<br>
    * [2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000), FALSE<br>
    * [2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000), TRUE
    */
   private MovingBool mboolVarying;

   /**
    * [2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000), FALSE<br>
    * [2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000), FALSE
    */
   private MovingBool mboolAllwaysFalse;

   /**
    * [2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000), TRUE<br>
    * [2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000), TRUE
    */
   private MovingBool mboolAllwaysTrue;

   /**
    * Boundary:<br>
    * ( (0, 0), (0, 50), (50, 50), (50, 0) )<br>
    * <br>
    * Holes:<br>
    * ( (20, 20), (20, 40), (40, 40), (40, 20) )
    */
   private Region regionWithHole;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Before
   public void setUp() throws Exception {

      mboolVarying = new MovingBool(0);
      mboolAllwaysFalse = new MovingBool(0);
      mboolAllwaysTrue = new MovingBool(0);

      mboolVarying.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), true);
      mboolVarying.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), false);
      mboolVarying.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), true);

      mboolAllwaysFalse.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), false);
      mboolAllwaysFalse.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), false);

      mboolAllwaysTrue.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), true);
      mboolAllwaysTrue.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), true);

      List<PointIF> points = new ArrayList<>();
      points.add(new Point(0, 0));
      points.add(new Point(0, 50));
      points.add(new Point(50, 50));
      points.add(new Point(50, 0));

      List<PointIF> holePoints = new ArrayList<>();
      holePoints.add(new Point(20, 20));
      holePoints.add(new Point(20, 40));
      holePoints.add(new Point(40, 40));
      holePoints.add(new Point(40, 20));

      Face face = new Face(points);
      face.add(holePoints);

      regionWithHole = new Region(face);
   }

   @Test
   public void testPasses_MovingBoolBaseBool_ValuePassed_ShouldBeTrue() {

      assertTrue(Passes.execute(mboolVarying, new BaseBool(true)));
      assertTrue(Passes.execute(mboolVarying, new BaseBool(false)));
   }

   @Test
   public void testPasses_MovingBoolBaseBool_ValueNotPassed_ShouldBeFalse() {

      assertFalse(Passes.execute(mboolAllwaysFalse, new BaseBool(true)));
      assertFalse(Passes.execute(mboolAllwaysTrue, new BaseBool(false)));
   }

   @Test
   public void testPasses_MovingBoolBaseBool_UndefinedValue_ShouldBeFalse() {

      assertFalse(Passes.execute(mboolVarying, new BaseBool()));
      assertFalse(Passes.execute(new MovingBool(0), new BaseBool(false)));
   }

   @Test
   public void testPasses_MovingPointRegion_MPointPassesRegion_ShouldBeTrue() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 30.0d);
      Point pointEnd = new Point(100.0d, 30.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      assertTrue(Passes.execute(mpoint, regionWithHole));
   }

   @Test
   public void testPasses_MovingPointRegion_MPointNotPassesRegion_ShouldBeFalse() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 300.0d);
      Point pointEnd = new Point(100.0d, 300.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      assertFalse(Passes.execute(mpoint, regionWithHole));
   }

   @Test
   public void testPasses_MovingPointRegion_UndefinedValues_ShouldBeFalse() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 300.0d);
      Point pointEnd = new Point(100.0d, 300.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      assertFalse(Passes.execute(mpoint, new Region(false)));
      assertFalse(Passes.execute(new MovingPoint(new Point()), regionWithHole));

   }
}
