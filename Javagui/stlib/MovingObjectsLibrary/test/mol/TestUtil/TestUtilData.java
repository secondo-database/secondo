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

package mol.TestUtil;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.time.TimeInstant;
import mol.datatypes.unit.spatial.util.MovableCycle;
import mol.datatypes.unit.spatial.util.MovableFace;
import mol.datatypes.unit.spatial.util.MovableFaceIF;
import mol.datatypes.unit.spatial.util.MovableSegment;
import mol.datatypes.unit.spatial.util.MovableSegmentIF;
import mol.interfaces.time.TimeInstantIF;

/**
 * This class provides helper methods for usage in the unit tests
 * 
 * @author Markus Fuessel
 *
 */
public class TestUtilData {

   /**
    * Create an instant object, given day will be parsed as a date from 01.01.2018
    * until 31.01.2018 with time 00:00 in local system time zone
    * 
    * @param day
    *           - day to use for the instant, valid values "01" - "31"
    * @return Instant object
    */
   public static TimeInstant getInstant(String day) {

      String myTimePattern = "yyyy-MM-dd HH:mm";
      DateTimeFormatter myTimeFormat = DateTimeFormatter.ofPattern(myTimePattern);
      LocalDateTime actualDateTime = LocalDateTime.parse("2018-01-" + day + " 00:00", myTimeFormat);
      ZoneId myTimeZone = ZoneId.systemDefault();
      Instant instant = actualDateTime.toInstant(ZoneOffset.UTC); // actualDateTime.atZone(myTimeZone).toInstant();
      TimeInstant tInstant = new TimeInstant(instant);

      return tInstant;
   }

   /**
    * Create a period object with given dayBegin and dayEnd as lower and upper
    * bound of the period. Given days will be parsed as a date from 01.01.2018
    * until 31.01.2018 with time 00:00 in local system time zone
    * <p>
    * dayBegin should be lower than dayEnd, valid values: "01"-"31"
    * 
    * @param dayBegin
    *           - day on which the period begins
    * @param dayEnd
    *           - day on which the period ends
    * @param leftClosed
    *           - dayBegin belongs to period if true
    * @param rightClosed
    *           - dayEnd belongs to period if true
    * @return a Period object
    */
   public static Period getPeriod(String dayBegin, String dayEnd, boolean leftClosed, boolean rightClosed) {

      TimeInstantIF instantBegin = getInstant(dayBegin);
      TimeInstantIF instantEnd = getInstant(dayEnd);

      Period period = getPeriod(instantBegin, instantEnd, leftClosed, rightClosed);

      return period;
   }

   /**
    * Create a period object with given instantBegin and instantEnd as lower and
    * upper bound of the period.
    * 
    * @param instantBegin
    *           - instant on which the period begins
    * @param instantEnd
    *           - instant on which the period begins
    * @param leftClosed
    *           - instantBegin belongs to period if true
    * @param rightClosed
    *           - instantEnd belongs to period if true
    * @return a Period Object
    */
   public static Period getPeriod(TimeInstantIF instantBegin, TimeInstantIF instantEnd, boolean leftClosed,
                                  boolean rightClosed) {

      Period period = new Period(instantBegin, instantEnd, leftClosed, rightClosed);

      return period;
   }

   /**
    * Create a 'MovableFace' object without holes.<br>
    * The basic 'MovableFace', which would be returned by passing zeros, is a
    * square at the beginning and transforms to a triangle at the end.<br>
    * Use the offset parameter to create 'MovableFace' objects with different
    * positions. <br>
    * <br>
    * vertices of square: (0,0), (0, 10), (10, 10), (10, 0) <br>
    * vertices of triangle: (0,0), (10, 10), (10, 0) <br>
    * 
    * @param offsetX
    *           - move entire MovableFace on the x-axis
    * @param offsetY
    *           - move entire MovableFace on the y-axis
    * 
    * @return a 'MovableFace' object without holes
    */
   public static MovableFaceIF getMovableFace(double offsetX, double offsetY) {
      // a square
      Point iP0 = new Point(0.0d + offsetX, 0.0d + offsetY);
      Point iP1 = new Point(0.0d + offsetX, 10.0d + offsetY);
      Point iP2 = new Point(10.0d + offsetX, 10.0d + offsetY);
      Point iP3 = new Point(10.0d + offsetX, 0.0d + offsetY);

      // a triangle
      Point fP0 = new Point(0.0d + offsetX, 0.0d + offsetY);
      Point fP1 = new Point(10.0d + offsetX, 10.0d + offsetY);
      Point fP2 = new Point(10.0d + offsetX, 0.0d + offsetY);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycle mcycle = new MovableCycle(msegments);

      return new MovableFace(mcycle);
   }

   /**
    * Create a 'MovableFace' object with two holes.<br>
    * The basic 'MovableFace', which would be returned by passing zeros, is a
    * square at the beginning and transforms to a triangle at the end.<br>
    * One hole is a constant triangle, the other hole is a square at the beginning
    * and degenerate into one point at the end. <br>
    * Use the offset parameter to create 'MovableFace' objects with different
    * positions. <br>
    * <br>
    * vertices of square: (0, 0), (0, 10), (10, 10), (10, 0) <br>
    * vertices of triangle: (0, 0), (10, 10), (10, 0) <br>
    * vertices of hole triangle: (2, 1), (4, 4), (4, 1) <br>
    * vertices of hole square: (6, 4), (6, 6), (8, 6), (8, 4) --> (7, 5) <br>
    * 
    * @param offsetX
    *           - move entire MovableFace on the x-axis
    * @param offsetY
    *           - move entire MovableFace on the y-axis
    * 
    * @return a 'MovableFace' object without holes
    */
   public static MovableFaceIF getMovableFaceWithHoles(double offsetX, double offsetY) {
      MovableFaceIF mface = getMovableFace(offsetX, offsetY);

      // constant hole0, a triangle
      Point iH0P0 = new Point(2.0d + offsetX, 1.0d + offsetY);
      Point iH0P1 = new Point(4.0d + offsetX, 4.0d + offsetY);
      Point iH0P2 = new Point(4.0d + offsetX, 1.0d + offsetY);

      Point fH0P0 = new Point(2.0d + offsetX, 1.0d + offsetY);
      Point fH0P1 = new Point(4.0d + offsetX, 4.0d + offsetY);
      Point fH0P2 = new Point(4.0d + offsetX, 1.0d + offsetY);

      // hole1, a square, degenerate at the end
      Point iH1P0 = new Point(6.0d + offsetX, 4.0d + offsetY);
      Point iH1P1 = new Point(6.0d + offsetX, 6.0d + offsetY);
      Point iH1P2 = new Point(8.0d + offsetX, 6.0d + offsetY);
      Point iH1P3 = new Point(8.0d + offsetX, 4.0d + offsetY);

      Point fH1P0 = new Point(7.0d + offsetX, 5.0d + offsetY);

      List<MovableSegmentIF> mHoleSegments0 = new ArrayList<>();
      List<MovableSegmentIF> mHoleSegments1 = new ArrayList<>();

      mHoleSegments0.add(new MovableSegment(iH0P0, iH0P1, fH0P0, fH0P1));
      mHoleSegments0.add(new MovableSegment(iH0P1, iH0P2, fH0P1, fH0P2));
      mHoleSegments0.add(new MovableSegment(iH0P2, iH0P0, fH0P2, fH0P0));

      mHoleSegments1.add(new MovableSegment(iH1P0, iH1P1, fH1P0, fH1P0));
      mHoleSegments1.add(new MovableSegment(iH1P1, iH1P2, fH1P0, fH1P0));
      mHoleSegments1.add(new MovableSegment(iH1P2, iH1P3, fH1P0, fH1P0));
      mHoleSegments1.add(new MovableSegment(iH1P3, iH1P0, fH1P0, fH1P0));

      mface.add(new MovableCycle(mHoleSegments0));
      mface.add(new MovableCycle(mHoleSegments1));

      return mface;
   }

   /**
    * Create a 'MovableFace' object without holes which degenerates at the
    * beginning or at the end.<br>
    * The basic 'MovableFace', which would be returned by passing zeros, is a
    * square and degenerates at the beginning or at the end into one poit.<br>
    * Use the offset parameter to create 'MovableFace' objects with different
    * positions. <br>
    * <br>
    * vertices of square: (0,0), (0, 10), (10, 10), (10, 0) <br>
    * 
    * @param offsetX
    *           - move entire MovableFace on the x-axis
    * @param offsetY
    *           - move entire MovableFace on the y-axis
    * @param degeneratesAtEnd
    *           - true - degenerates at the end, false - degenerates at the
    *           beginning
    * @return a 'MovableFace' object without holes
    */
   public static MovableFaceIF getMovableFaceDegenerate(double offsetX, double offsetY, boolean degeneratesAtEnd) {
      // a square
      Point iP0 = new Point(0.0d + offsetX, 0.0d + offsetY);
      Point iP1 = new Point(0.0d + offsetX, 10.0d + offsetY);
      Point iP2 = new Point(10.0d + offsetX, 10.0d + offsetY);
      Point iP3 = new Point(10.0d + offsetX, 0.0d + offsetY);

      // the degenerating point
      Point dP0 = new Point(5.0d + offsetX, 5.0d + offsetY);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      if (degeneratesAtEnd) {
         msegments.add(new MovableSegment(iP0, iP1, dP0, dP0));
         msegments.add(new MovableSegment(iP1, iP2, dP0, dP0));
         msegments.add(new MovableSegment(iP2, iP3, dP0, dP0));
         msegments.add(new MovableSegment(iP3, iP0, dP0, dP0));
      } else {
         msegments.add(new MovableSegment(dP0, dP0, iP0, iP1));
         msegments.add(new MovableSegment(dP0, dP0, iP1, iP2));
         msegments.add(new MovableSegment(dP0, dP0, iP2, iP3));
         msegments.add(new MovableSegment(dP0, dP0, iP3, iP0));
      }

      MovableCycle mcycle = new MovableCycle(msegments);

      return new MovableFace(mcycle);
   }

}
