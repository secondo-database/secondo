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

package unittests.mmdb.util;

import java.text.ParseException;
import java.util.Date;

import mmdb.data.attributes.date.AttributeInstant;
import mmdb.data.attributes.spatial.AttributeLine;
import mmdb.data.attributes.spatial.AttributePoint;
import mmdb.data.attributes.spatial.AttributePoints;
import mmdb.data.attributes.spatial.AttributeRegion;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.attributes.util.MovableUnitObjects.MovableFace;
import mmdb.data.attributes.util.MovableUnitObjects.MovableReal;
import mmdb.data.attributes.util.MovableUnitObjects.MovableRegion;
import mmdb.data.attributes.util.SpatialObjects.Face;
import mmdb.data.attributes.util.SpatialObjects.Segment;
import mmdb.data.attributes.util.TemporalObjects.Period;
import sj.lang.ListExpr;

/**
 * This class provides commonly used testing objects concerning attributes.
 *
 * @author Alexander Castor
 */
public class TestUtilAttributes {

	public static AttributeBool getBool(boolean value) {
		AttributeBool attribute = new AttributeBool();
		attribute.setValue(value);
		return attribute;
	}

	public static ListExpr getBoolList(boolean value) {
		return ListExpr.boolAtom(value);
	}

	public static AttributeInt getInt(int value) {
		AttributeInt attribute = new AttributeInt();
		attribute.setValue(value);
		return attribute;
	}

	public static ListExpr getIntList(int value) {
		return ListExpr.intAtom(value);
	}

	public static AttributeReal getReal(float value) {
		AttributeReal attribute = new AttributeReal();
		attribute.setValue(value);
		return attribute;
	}

	public static ListExpr getRealList(float value) {
		return ListExpr.realAtom(value);
	}

	public static AttributeString getString(String value) {
		AttributeString attribute = new AttributeString();
		attribute.setValue(value);
		return attribute;
	}

	public static ListExpr getStringList(String value) {
		return ListExpr.stringAtom(value);
	}

	public static AttributePoint getPoint(float x, float y) {
		AttributePoint point = new AttributePoint();
		point.setXValue(x);
		point.setYValue(y);
		return point;
	}

	public static ListExpr getPointList(float x, float y) {
		return ListExpr.twoElemList(getRealList(x), getRealList(y));
	}

	public static AttributePoints getPoints() {
		AttributePoints points = new AttributePoints();
		points.addPoint(getPoint(1.0f, 2.0f));
		points.addPoint(getPoint(3.0f, 4.0f));
		return points;
	}

	public static ListExpr getPointsList() {
		ListExpr firstPointList = getPointList(1.0f, 2.0f);
		ListExpr secondPointList = getPointList(3.0f, 4.0f);
		return ListExpr.twoElemList(firstPointList, secondPointList);
	}

	public static AttributeInstant getInstant(String day) throws ParseException {
		AttributeInstant instant = new AttributeInstant();
		Date date = AttributeInstant.FORMAT_4.parse("2014-10-" + day);
		instant.setDate(date);
		instant.setFormat(AttributeInstant.FORMAT_4);
		return instant;
	}

	public static ListExpr getInstantList(String day) {
		return getStringList("2014-10-" + day);
	}

	public static Period getPeriod(String dayStart, String dayEnd) throws ParseException {
		Period period = new Period();
		period.setStartTime(getInstant(dayStart));
		period.setEndTime(getInstant(dayEnd));
		period.setLeftClosed(true);
		period.setRightClosed(true);
		return period;
	}

	public static ListExpr getPeriodList(String dayStart, String dayEnd) {
		ListExpr startDateList = getStringList("2014-10-" + dayStart);
		ListExpr endDateList = getStringList("2014-10-" + dayEnd);
		return ListExpr.fourElemList(startDateList, endDateList, getBoolList(true),
				getBoolList(true));
	}

	public static Segment getSegment() {
		Segment segment = new Segment();
		segment.setxValue1(1.0f);
		segment.setyValue1(2.0f);
		segment.setxValue2(3.0f);
		segment.setyValue2(4.0f);
		return segment;
	}

	public static ListExpr getSegmentList() {
		return ListExpr.fourElemList(getRealList(1.0f), getRealList(2.0f), getRealList(3.0f),
				getRealList(4.0f));
	}

	public static AttributeLine getLine() {
		AttributeLine line = new AttributeLine();
		line.addSegment(TestUtilAttributes.getSegment());
		line.addSegment(TestUtilAttributes.getSegment());
		return line;
	}

	public static ListExpr getLineList() {
		return ListExpr.twoElemList(TestUtilAttributes.getSegmentList(),
				TestUtilAttributes.getSegmentList());
	}

	public static Face getFace() {
		Face face = new Face();
		face.setOuterCycle(getPoints());
		face.addCycleToHoleCycles(getPoints());
		face.addCycleToHoleCycles(getPoints());
		return face;
	}

	public static ListExpr getFaceList() {
		return ListExpr.threeElemList(getPointsList(), getPointsList(), getPointsList());
	}

	public static AttributeRegion getRegion() {
		AttributeRegion region = new AttributeRegion();
		region.addFace(TestUtilAttributes.getFace());
		region.addFace(TestUtilAttributes.getFace());
		return region;
	}

	public static ListExpr getRegionList() {
		return ListExpr.twoElemList(getFaceList(), getFaceList());
	}

	public static MovableReal getMovableReal() {
		MovableReal movableReal = new MovableReal();
		movableReal.setFirstReal(1.0f);
		movableReal.setSecondReal(2.0f);
		movableReal.setThirdReal(3.0f);
		movableReal.setBooleanPart(true);
		return movableReal;
	}

	public static ListExpr getMovableRealList() {
		return ListExpr.fourElemList(getRealList(1.0f), getRealList(2.0f), getRealList(3.0f),
				getBoolList(true));
	}

	public static MovableFace getMovableFace() {
		MovableFace movableFace = new MovableFace();
		movableFace.setOuterCycle(getLine());
		movableFace.addCycleToHoleCycles(getLine());
		movableFace.addCycleToHoleCycles(getLine());
		return movableFace;
	}

	public static ListExpr getMovableFaceList() {
		return ListExpr.threeElemList(getLineList(), getLineList(), getLineList());
	}

	public static MovableRegion getMovableRegion() {
		MovableRegion movableRegion = new MovableRegion();
		movableRegion.addFace(TestUtilAttributes.getMovableFace());
		movableRegion.addFace(TestUtilAttributes.getMovableFace());
		return movableRegion;
	}

	public static ListExpr getMovableRegionList() {
		return ListExpr.twoElemList(getMovableFaceList(), getMovableFaceList());
	}

	public static ListExpr getUboolList() {
		return ListExpr.twoElemList(getPeriodList("01", "02"), getBoolList(true));
	}

	public static ListExpr getUintList() {
		return ListExpr.twoElemList(getPeriodList("01", "02"), getIntList(1));
	}

	public static ListExpr getUpointList() {
		return ListExpr.twoElemList(getPeriodList("01", "02"), getSegmentList());
	}

	public static ListExpr getUrealList() {
		return ListExpr.twoElemList(getPeriodList("01", "02"), getMovableRealList());
	}

	public static ListExpr getUregionList() {
		return ListExpr.twoElemList(getPeriodList("01", "02"), getMovableRegionList());
	}

	public static ListExpr getUstringList() {
		return ListExpr.twoElemList(getPeriodList("01", "02"), getStringList("A"));
	}

}
