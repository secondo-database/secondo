/*
 ----

 This file is part of SECONDO.

 Copyright (C) 2004, University in Hagen, Department of Computer Science,
 Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 SECONDO is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SECONDO; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ----
 [1] Header File of the RobustGeometry Algebra


 [TOC]
 1 Overview
 2 Defines and includes

*/
#ifndef __ROBUSTGEOMETRY_ALGEBRA_H__
#define __ROBUSTGEOMETRY_ALGEBRA_H__

#include "HalfSegment.h"
#include "AVLSegment.h"
#include "Coord.h"
#include <iosfwd>

using std::ostream;

namespace robustGeometry {

double const Epsilon = 0.0001;
double const ScaleFactor = 0.5;
enum BOPointType {
	start, end, intersect
};
enum BOOwnerType {
	first, second, both
};

class BOLine {
public:
	BOLine() {
	}
	;
	BOLine(const BOLine& line);
	BOLine(const double x1, const double y1,
		   const double x2, const double y2,
		   const BOOwnerType owner);
	~BOLine(void) {
	}
	;
	void setX1(const double x);
	void setX2(const double x);
	void setY1(const double y);
	void setY2(const double y);
	void setOwner(const BOOwnerType owner);

	double getX1() const {
		return x1;
	}
	;
	double getX2() const {
		return x2;
	}
	;
	double getY1() const {
		return y1;
	}
	;
	double getY2() const {
		return y2;
	}
	;
	BOOwnerType getOwner() const {
		return owner;
	}
	;
	friend bool operator <(const BOLine& l1, const BOLine& l2) {
		if (l1.getX1() < l2.getX1())
			return true;
		else
			return false;
	}
private:
	double x1;
	double y1;
	double x2;
	double y2;
	//BOPointType pointType;
	BOOwnerType owner;

};

class BOEvent {
public:
	BOEvent() {
	}
	;
	BOEvent(const double x, const double y, const BOPointType pType);
	BOEvent(const double x, const double y, const BOPointType pType,
			const BOLine& line);
	BOEvent(const double x, const double y, const BOPointType pType,
			const BOLine& line1, const BOLine& line2);
	~BOEvent(void) {
	}
	;

	void setX(const double x);
	void setY(const double y);
	void setPointType(const BOPointType pointType);
	void setLine(const BOLine& line);
	void setLine1(const BOLine& line1);
	void setLine2(const BOLine& line2);
	double getX() const {
		return x;
	}
	;
	double getY() const {
		return y;
	}
	;

	BOLine& getLine() {
		return line;
	}
	;
	BOLine& getLine1() {
		return line1;
	}
	;
	BOLine& getLine2() {
		return line2;
	}
	;
	BOPointType getPointType() const {
		return pointType;
	}
	;
	void Print(ostream& out) const;
	BOLine& getLineAbove() {
		return lineAbove;
	}
	;
	BOLine& getLineBelow() {
		return lineBelow;
	}
	;

private:
	double x;
	double y;
	BOPointType pointType;
	BOLine line, line1, line2, lineAbove, lineBelow;
};

class CompBOEventXY {
public:
	bool operator()(BOEvent e1, BOEvent e2) {
		if (e1.getX() < e2.getX())
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() < e2.getY()))
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() == e2.getY())
				&& (e1.getPointType() < e2.getPointType()))
			return true;
		else
			return false;
	}
	;
	friend bool operator ==(const BOEvent e1, const BOEvent e2) {
		if ((e1.getX() == e2.getX()) && (e1.getY() == e2.getY())
				&& (e1.getPointType() == e2.getPointType()))
			return true;
		else
			return false;
	}
	;
	friend bool operator !=(const BOEvent e1, const BOEvent e2) {
		if (e1.getX() != e2.getX())
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() != e2.getY()))
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() == e2.getY())
				&& (e1.getPointType() != e2.getPointType()))
			return true;
		else
			return false;
	}
	;
	friend bool operator <(const BOEvent e1, const BOEvent e2) {
		if (e1.getX() < e2.getX())
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() < e2.getY()))
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() == e2.getY())
				&& (e1.getPointType() < e2.getPointType()))
			return false;
	}
	;

	friend bool operator >(const BOEvent e1, const BOEvent e2) {
		if (e1.getX() > e2.getX())
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() > e2.getY()))
			return true;
		else
			return false;
	}
	;
	friend bool operator <=(const BOEvent e1, const BOEvent e2) {
		if (e1.getX() <= e2.getX())
			return true;
		else if ((e1.getX() == e2.getX()) && (e1.getY() <= e2.getY()))
			return true;
		else
			return false;
	}
	;
	friend bool operator >=(const BOEvent e1, const BOEvent e2) {
		if (e1.getX() >= e2.getX()) {
			return true;
		} else if ((e1.getX() == e2.getX()) && (e1.getY() >= e2.getY()))
			return true;
		else
			return false;
	}

};

class CompBOLine {
public:
	bool operator()(BOLine l1, BOLine l2) {
		if (l1.getX1() < l2.getX1())
			return true;
		else if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() < l2.getY1()))
			return true;
		else if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() == l2.getY1())
				&& (l1.getX2() < l2.getX2()))
			return true;
		else if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() == l2.getY1())
				&& (l1.getX2() == l2.getX2())
				&& (l1.getY2() < l2.getY2()))
			return true;
		else

			return false;
	}
	;
	friend bool operator ==(const BOLine l1,
			const BOLine l2) {
		if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() == l2.getY1())
				&& (l1.getX2() == l2.getX2())
				&& (l1.getY2() == l2.getY2())
				&& (l1.getOwner() == l2.getOwner()))
			return true;
		else
			return false;
	}
	;

};

class CompBOLineXYOwn {
public:
	bool operator()(BOLine l1, BOLine l2) {
		if (l1.getX1() < l2.getX1())
			return true;
		else if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() < l2.getY1()))
			return true;
		else if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() == l2.getY1())
				&& (l1.getOwner() < l2.getOwner()))
			return true;
		else if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() == l2.getY1())
				&& (l1.getOwner() == l2.getOwner())
				&& (l1.getX2() < l2.getX2()))
			return true;
		else if ((l1.getX1() == l2.getX1())
				&& (l1.getY1() == l2.getY1())
				&& (l1.getOwner() == l2.getOwner())
				&& (l1.getX2()
				== l2.getX2())
				&& (l1.getY2() < l2.getY2()))
			return true;
		else
			return false;
	}
	;


};

class ToleranceSquare {

public:
	ToleranceSquare() {
	}
	;
	ToleranceSquare(const double x11, const double y11,
			const double x12,
			const double y12, const double x21,
			const double y21,
			const double x22, const double y22,
			const double snap_x,
			const double snap_y, const BOEvent& boEv) {
		setX11(x11);
		setY11(y11);
		setX12(x12);
		setY12(y12);
		setX21(x21);
		setY21(y21);
		setX22(x22);
		setY22(y22);
		setSnapX(snap_x);
		setSnapY(snap_y);

		setBOEvent(boEv);
	}
	;
	~ToleranceSquare(void) {
	}
	;

	void setX11(const double x11);
	void setY11(const double y11);
	void setX12(const double x12);
	void setY12(const double y12);
	void setX21(const double x21);
	void setY21(const double y21);
	void setX22(const double x22);
	void setY22(const double y22);
	void setSnapX(const double snap_x);
	void setSnapY(const double snap_y);

	void setBOEvent(const BOEvent& boEv);

	double getX11() const {
		return x11;
	}
	;
	double getY11() const {
		return y11;
	}
	;
	double getX12() const {
		return x12;
	}
	;
	double getY12() const {
		return y12;
	}
	;
	double getX21() const {
		return x21;
	}
	;
	double getY21() const {
		return y21;
	}
	;
	double getX22() const {
		return x22;
	}
	;
	double getY22() const {
		return y22;
	}
	;
	double getSnapX() const {
		return snap_x;
	}
	;
	double getSnapY() const {
		return snap_y;
	}
	;

	const BOEvent& getBOEvent() {
		return boEv;
	}
	;
	void Print(ostream& out) const;

private:
	double x11;
	double y11;
	double x12;
	double y12;
	double x21;
	double y21;
	double x22;
	double y22;
	double snap_x;
	double snap_y;

	BOEvent boEv;
	double scale(double value) {
		return (double) (value * ScaleFactor); //todo runden?
	}
	;

};
class CompToleranceSquareY {
public:
	bool operator()(ToleranceSquare t1, ToleranceSquare t2) {
		if (t1.getY21() < t2.getY21())
			return true;
		else if ((t1.getY21() ==
				t2.getY21()) && (t1.getX21() < t2.getX21()))
			return true;
		else
			return false;
	}
	;


};
class Point {
public:
	Point() {
	}
	;
	Point(const double x, const double y) {
		setX(x);
		setY(y);
	}
	;

	~Point(void) {
	}
	;

	void setX(const double x);
	void setY(const double y);
	double getX() const {
		return x;
	}
	;
	double getY() const {
		return y;
	}
	;

private:
	double x;
	double y;

};



}
; //end of namespaces


#endif // __ROBUSTGEOMETRY_ALGEBRA_H__

