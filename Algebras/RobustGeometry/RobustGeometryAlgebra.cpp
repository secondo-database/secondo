/*

 [1] RobustGeometryAlgebra

 Februar 2012 Katja Koch
 Overview




 Defines and Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoConfig.h"
#include "AvlTree.h"
#include "AVLSegment.h"
#include "AlmostEqual.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "RegionTools.h"
#include "Symbols.h"
#include "NList.h"
#include "LogMsg.h"
#include "ConstructorTemplates.h"
#include "TypeMapUtils.h"
#include "SpatialAlgebra.h"
#include "RobustGeometryAlgebra.h"
#include "AVLSegment.h"
#include "HalfSegment.h"

#include <vector>
#include <set>
#include <queue>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <queue>
#include <iterator>
#include <sstream>
#include <limits>
#include <errno.h>
#include <cerrno>

#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

extern NestedList* nl;
extern QueryProcessor* qp;

/*
 Algebra Implementation
 Type Mapping Functions
 These functions check whether the correct argument types are supplied for an
 operator; if so, returns a list expression for the result type, otherwise the
 symbol ~typeerror~.

*/

/*
 An algebra module must provide for each type functions which take a nested
 list and create an instance of the class representing the type and vice versa.
 These functions are called In- and Out-functions. ->see SpatialAlgebra In/Out

*/

/*
 This means that for a type constructor the functions create, delete,
 close, and clone must be implemented. The remaining two functions open
 and save may be implemented. If they are not implemented,
 the default persistent storage mechanism is used. ->see SpatialAlgebra

*/


enum SpatialTypeRG {
	stpoint, stpoints, stline, stregion, stbox, sterror
};

SpatialTypeRG SpatialTypeOfSymbolRG(ListExpr symbol) {
	if (nl->AtomType(symbol) == SymbolType) {
		string s = nl->SymbolValue(symbol);
		if (s == Point::BasicType())
			return (stpoint);
		if (s == Points::BasicType())
			return (stpoints);
		if (s == Line::BasicType())
			return (stline);
		if (s == Region::BasicType())
			return (stregion);
		if (s == Rectangle<2>::BasicType())
			return (stbox);
	}
	return (sterror);
}

/*
 Type mapping

*/

static ListExpr intersectionTM(ListExpr args) {
	ListExpr arg1, arg2;
	if (nl->ListLength(args) == 2) {
		arg1 = nl->First(args);
		arg2 = nl->Second(args);
		if (SpatialTypeOfSymbolRG(arg1) == stline
				&& SpatialTypeOfSymbolRG(arg2) == stline)
			return (nl->SymbolAtom(Line::BasicType()));
	}
	return (nl->SymbolAtom(Symbol::TYPEERROR()));
}

int RobustGeometrySetOpSelect(ListExpr args) {
	string a1 = nl->SymbolValue(nl->First(args));
	string a2 = nl->SymbolValue(nl->Second(args));

	if (a1 == Line::BasicType()) {
		if (a2 == Line::BasicType())
			return 1;
		return -1;
	}

	return -1;
}

/*
 Implementation of ~BOEvent~

*/

/*
 Constructors

 ~Standard Constructor~

*/
robustGeometry::BOLine::BOLine(const BOLine& line) {
	x1 = line.getX1();
	x2 = line.getX2();
	y1 = line.getY1();
	y2 = line.getY2();
	owner = line.getOwner();
};

robustGeometry::BOLine::BOLine(const double x1, const double y1,
		const double x2, const double y2,
		const robustGeometry::BOOwnerType owner) {
	setX1(x1);
	setY1(y1);
	setX2(x2);
	setY2(y2);
	setOwner(owner);
}
;
void robustGeometry::BOLine::setX1(const double x) {
	this->x1 = x;
}
;
void robustGeometry::BOLine::setX2(const double x) {
	this->x2 = x;
}
;
void robustGeometry::BOLine::setY1(const double y) {
	this->y1 = y;
}
;
void robustGeometry::BOLine::setY2(const double y) {
	this->y2 = y;
}
;
void robustGeometry::BOLine::setOwner(const BOOwnerType owner) {
	this->owner = owner;
}
;

robustGeometry::BOEvent::BOEvent(const double x, const double y,
		const robustGeometry::BOPointType pType) {
	setX(x);
	setY(y);
	setPointType(pType);
}
;

robustGeometry::BOEvent::BOEvent(const double x, const double y,
		const robustGeometry::BOPointType pType,
		const robustGeometry::BOLine& line) {
	setX(x);
	setY(y);
	setPointType(pType);
	setLine(line);
}
;

robustGeometry::BOEvent::BOEvent(const double x, const double y,
		const robustGeometry::BOPointType pType,
		const robustGeometry::BOLine& line1,
		const robustGeometry::BOLine& line2) {
	setX(x);
	setY(y);
	setPointType(pType);
	setLine1(line1);
	setLine2(line2);
	if (line1.getY1() > line2.getY1()) {
		lineAbove = line1;
		lineBelow = line2;
	} else {
		lineAbove = line2;
		lineBelow = line1;
	};
};


void robustGeometry::BOEvent::setX(const double x) {
	this->x = x;
}
;
void robustGeometry::BOEvent::setY(const double y) {
	this->y = y;
}
;
void robustGeometry::BOEvent::setPointType(const BOPointType pointType) {
	this->pointType = pointType;
}
;
void robustGeometry::BOEvent::setLine(const BOLine & line) {
	this->line = line;
}
;

void robustGeometry::BOEvent::setLine1(const BOLine & line1) {
	this->line1 = line1;
}
;
void robustGeometry::BOEvent::setLine2(const BOLine & line2) {
	this->line2 = line2;
}
;

void robustGeometry::BOEvent::Print(ostream& out) const {
	//	out << "X : " << getX( ) <<
	//		   ",Y : " << getY( );
	//		  // ", owner : " << getOwner( )

}
;

void robustGeometry::Point::setX(const double x) {
	this->x = x;
}
;
void robustGeometry::Point::setY(const double y) {
	this->y = y;
}
;

void robustGeometry::ToleranceSquare::setX11(const double x11) {
	this->x11 = x11;
}
;
void robustGeometry::ToleranceSquare::setY11(const double y11) {
	this->y11 = y11;
}
;
void robustGeometry::ToleranceSquare::setX12(const double x12) {
	this->x12 = x12;
}
;
void robustGeometry::ToleranceSquare::setY12(const double y12) {
	this->y12 = y12;
}
;
void robustGeometry::ToleranceSquare::setX21(const double x21) {
	this->x21 = x21;
}
;
void robustGeometry::ToleranceSquare::setY21(const double y21) {
	this->y21 = y21;
}
;
void robustGeometry::ToleranceSquare::setX22(const double x22) {
	this->x22 = x22;
}
;
void robustGeometry::ToleranceSquare::setY22(const double y22) {
	this->y22 = y22;
}
;
void robustGeometry::ToleranceSquare::setSnapX(const double snap_x) {
	this->snap_x = snap_x;
}
;
void robustGeometry::ToleranceSquare::setSnapY(const double snap_y) {
	this->snap_y = snap_y;
}
;
void robustGeometry::ToleranceSquare::setBOEvent(const BOEvent & boEv) {
	this->boEv = boEv;
};


const string intersectionSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
	"( <text>{line} x"
	"   {line} -> T, "
	" where T = points if any point or point type is one of the "
	" arguments or the argument having the smaller dimension </text--->"
	"<text>intersectionBO(arg1, arg2)</text--->"
	"<text>intersectionBO of two spatial objects</text--->"
	"<text>query intersectionBO(tiergarten, thecenter) </text--->"
	") )";

/*
 class MakeBO

*/

class MakeBO {

public:
	void IntersectionBO(const Line& line1,
			const Line& line2, Line& result);

	// void Intersection( const Line& l, Line& result,
	//const Geoid* geoid=0 ) const;

	int intersect(const double l1_x1,
			const double l1_y1, const double l1_x2,
			const double l1_y2, const double l2_x1,
			const double l2_y1,
			const double l2_x2, const double l2_y2,
			double& x, double& y);
	void checkIS(robustGeometry::BOEvent& currEv,
			const robustGeometry::BOLine& currSeg);
	void checkIS(const robustGeometry::BOLine& line1,
			const robustGeometry::BOLine& line2);
	void findAndSwap(const robustGeometry::BOLine& aboveSeg,
			const robustGeometry::BOLine& belowSeg);

	const robustGeometry::BOLine*
	getAbove(const double x, const double y);

	const robustGeometry::BOLine*
	getBelow(const double aktXPos, double aktYPos);

	const robustGeometry::BOLine*
	getAbove(robustGeometry::BOEvent& event);

	const robustGeometry::BOLine*
	getBelow(robustGeometry::BOEvent& event);

	void addLineToSweepLine(const robustGeometry::BOLine& line);
	void addBOEvent(const robustGeometry::BOEvent& event);
	void doBOCaseA(robustGeometry::BOEvent & event);
	void doBOCaseB(robustGeometry::BOEvent & event);
	void doBOCaseC(robustGeometry::BOEvent & event);
	void addInitialEvent(const robustGeometry::BOEvent& event);

	void findEventAndDelete(robustGeometry::BOEvent & event);
	void doBOCases();
	void printInitialEvents();
	void printBOEvents();
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>& get_BOEvents() {
		return boEvents;
	};
	MakeBO() {
		//	  outputPoints= new Points();

	};
	~MakeBO() {
	};

private:
	set<robustGeometry::BOLine,
	robustGeometry::CompBOLine> sL; // sweepLine
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> initialEvents;
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> boEvents;
};

void MakeBO::doBOCaseA(robustGeometry::BOEvent & event) {
	addBOEvent(event);

	addLineToSweepLine(event.getLine());
	const robustGeometry::BOLine* segAbove = 0;

	segAbove = getAbove(event);
	if (segAbove != 0) {
		checkIS(event, *segAbove);
	};
	const robustGeometry::BOLine* segBelow = 0;
	segBelow = getBelow(event);
	if (segBelow != 0) {
		checkIS(event, *segBelow);
	};
}

void MakeBO::doBOCaseB(robustGeometry::BOEvent & event) {
	addBOEvent(event);
	const robustGeometry::BOLine* segAbove = 0;
	segAbove = getAbove(event);
	const robustGeometry::BOLine* segBelow = 0;
	segBelow = getBelow(event);

	findEventAndDelete(event);

	if ((segAbove != 0) && (segBelow != 0)) {
		checkIS(*segAbove, *segBelow);
	};
}

void MakeBO::doBOCaseC(robustGeometry::BOEvent & event) {
	addBOEvent(event);
	const robustGeometry::BOLine* seg1 =
			&event.getLineAbove();
	const robustGeometry::BOLine* seg2 =
			&event.getLineBelow();

	findAndSwap(*seg1, *seg2);

	const robustGeometry::BOLine *segAbove = 0;
	segAbove = getAbove(seg2->getX2(), seg2->getY2());
	const robustGeometry::BOLine *segBelow = 0;
	segBelow = getBelow(seg1->getX2(), seg1->getY2());

	if ((segAbove != 0) && (seg2 != 0)) {
		checkIS(*segAbove, *seg2);
	};
	if ((segBelow != 0) && (seg1 != 0)) {
		checkIS(*seg1, *segBelow);
	};
}

void MakeBO::doBOCases() {

	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>::iterator
			event_it;

	boEvents.clear();
	sL.clear();

	for (event_it = initialEvents.begin();
			event_it != initialEvents.end(); ++event_it) {
		robustGeometry::BOEvent event = *event_it;
		if (event.getPointType() == robustGeometry::start) {
			doBOCaseA(event);
		} else if (event.getPointType() == robustGeometry::end) {
			doBOCaseB(event);
		} else if (event.getPointType() == robustGeometry::intersect) {
			doBOCaseC(event);
		};

	}

}

void MakeBO::printInitialEvents() {

	set<robustGeometry::BOEvent>::iterator it;
	for (it = initialEvents.begin(); it != initialEvents.end(); ++it) {
		cout << it->getX() << " " << it->getY() << " Pointtype ";
		if (it->getPointType() == robustGeometry::start)
			cout << "start";
		else if (it->getPointType() == robustGeometry::end)
			cout << "end";
		else
			cout << "intersect";
		cout << endl;
	}
}
void MakeBO::printBOEvents() {

	set<robustGeometry::BOEvent>::iterator it;
	for (it = boEvents.begin(); it != boEvents.end(); ++it) {
		cout << it->getX() << " " << it->getY();
		if (it->getPointType() == robustGeometry::start)
			cout << "start";
		else if (it->getPointType() == robustGeometry::end)
			cout << "end";
		else {
			robustGeometry::BOEvent tmpEv = *it;
			cout << "intersect" << " line1 "
					<< tmpEv.getLine1().getX1()
					<< " line2 " <<
					tmpEv.getLine1().getX2();
		}
		cout << endl;
	}
}

const robustGeometry::BOLine * MakeBO::getAbove(const double x,
		const double y) {

	set<robustGeometry::BOLine>::iterator seg_it;
	for (seg_it = sL.begin(); seg_it != sL.end(); ++seg_it) {
		robustGeometry::BOLine tmpSeg = *seg_it;
		if ((x > seg_it->getX1()) && (x < seg_it->getX2()) && (y
				< seg_it->getY1())) {
			return &*seg_it;
		}
		if (tmpSeg.getX1() > x)
			break;
	}
	return 0;
}

const robustGeometry::BOLine * MakeBO::getAbove
(robustGeometry::BOEvent& event) {
	return getAbove(event.getX(), event.getY());
}
const robustGeometry::BOLine * MakeBO::getBelow
(const double x, const double y) {
	set<robustGeometry::BOLine,
	robustGeometry::CompBOLine>::iterator seg_it;
	for (seg_it = sL.begin(); seg_it != sL.end(); ++seg_it) {
		robustGeometry::BOLine tmpSeg = *seg_it;
		if ((x > tmpSeg.getX1()) && (x < tmpSeg.getX2())
				&& (y > tmpSeg.getY1())) {
			return &*seg_it;
		}
		if (tmpSeg.getX1() > x)
			break;
	}
	return 0;
}

const robustGeometry::BOLine * MakeBO::getBelow
(robustGeometry::BOEvent& event) {
	return getBelow(event.getX(), event.getY());
}

void MakeBO::findEventAndDelete(robustGeometry::BOEvent & event) {
	if (sL.size() == 0)
		return;

	robustGeometry::BOLine eventSeg = event.getLine();

	sL.erase(robustGeometry::BOLine(eventSeg.getX1(),
			eventSeg.getY1(),
			eventSeg.getX2(),
			eventSeg.getY2(), eventSeg.getOwner()));
}

void MakeBO::findAndSwap(const robustGeometry::BOLine& aboveSeg,
		const robustGeometry::BOLine& belowSeg) {
	int pos1 = -1;
	int pos2 = -1;

	set<robustGeometry::BOLine,
	robustGeometry::CompBOLine>::iterator sL_it;
	int i = 0;
	double tempx1 = 0.0;
	double tempx2 = 0.0;
	double tempy1 = 0.0;
	double tempy2 = 0.0;
	for (sL_it = sL.begin(); sL_it != sL.end(); ++sL_it) {

		robustGeometry::BOLine tmpSeg = *sL_it;
		tempx1 = tmpSeg.getX1();
		tempy1 = tmpSeg.getY1();
		tempx2 = tmpSeg.getX2();
		tempy2 = tmpSeg.getY2();

		if ((tempx1 == aboveSeg.getX1()) &&
				(tempy1 == aboveSeg.getY1())
				&& (tempx1 == aboveSeg.getX2())
				&& (tempy1 == aboveSeg.getY2())) {
			pos1 = i;
		}
		if ((tempx1 == belowSeg.getX1()) &&
				(tempy1 == belowSeg.getY1())
				&& (tempx1 == belowSeg.getX2())
				&& (tempy1 == belowSeg.getY2())) {
			pos2 = i;
		}
		i++;
		if (pos1 >= 0 && pos2 >= 0) {
			robustGeometry::BOLine tmp = aboveSeg;
		}
	}

}

int MakeBO::intersect(const double l1_x1,
		const double l1_y1,
		const double l1_x2, const double l1_y2,
		const double l2_x1,
		const double l2_y1, const double l2_x2,
		const double l2_y2, double & x,
		double & y) {
	//return =0 OK, 1 lines parallel, 2 no intersection point
	//x,y intersection point
	double a1 = 0.0;
	double a2 = 0.0;
	double b1 = 0.0;
	double b2 = 0.0;
	double c1 = 0.0;
	double c2 = 0.0; // Coefficients of line
	double m = 0.0;

	a1 = l1_y2 - l1_y1;
	b1 = l1_x1 - l1_x2;
	c1 = l1_x2 * l1_y1 - l1_x1 * l1_y2;
	//a1*x + b1*y + c1 = 0 is segment 1

	a2 = l2_y2 - l2_y1;
	b2 = l2_x1 - l2_x2;
	c2 = l2_x2 * l2_y1 - l2_x1 * l2_y2;
	//a2*x + b2*y + c2 = 0 is segment 2

	m = a1 * b2 - a2 * b1;
	if (m == 0)
		return 1;

	x = (b1 * c2 - b2 * c1) / m;
	// intersection range

	if (x < l1_x1 || x < l2_x1 || x > l1_x2 || x > l2_x2)
		return 2;

	y = (a2 * c1 - a1 * c2) / m;

	return 0;
}

void MakeBO::checkIS(const robustGeometry::BOLine& line1,
		const robustGeometry::BOLine& line2) {
	double sx1 = 0.0;
	double sy1 = 0.0;
	double ex1 = 0.0;
	double ey1 = 0.0;
	double sx2 = 0.0;
	double sy2 = 0.0;
	double ex2 = 0.0;
	double ey2 = 0.0;
	double is_x = 0.0;
	double is_y = 0.0;
	int isIntersected = 1;

	robustGeometry::BOOwnerType currOwner;
	robustGeometry::BOOwnerType nextOwner;

	sx2 = line2.getX1();
	sy2 = line2.getY1();
	ex2 = line2.getX2();
	ey2 = line2.getY2();
	nextOwner = line2.getOwner();

	sx1 = line1.getX1();
	sy1 = line1.getY1();
	ex1 = line1.getX2();
	ey1 = line1.getY2();
	currOwner = line1.getOwner();

	//check owner first, second,
	if (currOwner != nextOwner) {
		isIntersected = intersect(sx1, sy1, ex1,
				ey1, sx2, sy2, ex2, ey2, is_x,
				is_y);
		if (isIntersected == 0) {
cout << "line 1 X1 : " << line1.getX1() << "line 1 y1" << line1.getY1() << endl;
cout << "line 2 X1 : " << line2.getX1() << "line 2 y1" << line2.getY1() << endl;
			robustGeometry::BOEvent intersP =
			robustGeometry::BOEvent(is_x,
			is_y, robustGeometry::intersect, line1, line2);
			addLineToSweepLine(intersP.getLine());
			addBOEvent(intersP);
		}
	}
}
void MakeBO::checkIS(robustGeometry::BOEvent& currEv,
		const robustGeometry::BOLine& currSeg) {
	robustGeometry::BOLine* evSeg = &currEv.getLine();
	checkIS(*evSeg, currSeg);
}

void MakeBO::addLineToSweepLine(const robustGeometry::BOLine& line) {
	sL.insert(line);
}

void MakeBO::addBOEvent(const robustGeometry::BOEvent& event) {
	boEvents.insert(event);
}
void MakeBO::addInitialEvent(const robustGeometry::BOEvent& event) {
	initialEvents.insert(event);
}

/*
 class MakeHobby

*/

class MakeHobby {
public:

	MakeHobby() {
	}
	;
	~MakeHobby() {
	}
	;
	void doHO();
	void computeToleranceSq();
	void round(robustGeometry::BOEvent& boEv);
	void locateValue(robustGeometry::ToleranceSquare & ts);

	void snapSegment(robustGeometry::BOEvent & boEv,
			robustGeometry::ToleranceSquare & tolS);
	bool computeAllIntersections(robustGeometry::BOEvent & boEv,
			robustGeometry::ToleranceSquare & tolS);
	void intersectsToleranceSquare(robustGeometry::BOEvent & boEv);
	void updateMainActive(double von, double bis);
	void set_BOEvents(set<robustGeometry::BOEvent,
			robustGeometry::CompBOEventXY> & events) {
		events_org = events;
	};
	set<robustGeometry::BOLine, robustGeometry::CompBOLineXYOwn>
	& get_BOLine() {
			return result_line;
	};

	void event2line(robustGeometry::BOEvent & boEv);

	void splitSegment(robustGeometry::BOEvent & boEv);
	void printHOInitialEvents();
	void printHOEvents();
	void printHOResult_events();
	void printHOResult_lines();

private:
	set<robustGeometry::ToleranceSquare,
	robustGeometry::CompToleranceSquareY>
			tolSquare;
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> mainActive;
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> events_org;
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> events_r;
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> result_e;
	set<robustGeometry::BOLine,
	robustGeometry::CompBOLineXYOwn> result_line;

	int intersect(double sx1, double sy1, double ex2,
			double ey2, double sx3,
			double sy3, double ex4, double ey4);

	const robustGeometry::BOEvent *
	getRoundedPoint(robustGeometry::BOEvent & boEv);

	void getRoundedPoints(const robustGeometry::BOEvent ** bo_s,
			const robustGeometry::BOEvent ** bo_e,
			robustGeometry::BOLine & line);

};

/*
 Construktors and Destructor

*/

/*MakeHobby::MakeHobby()
 {
 tolSquare.clear();
 }*/
void MakeHobby::doHO() {
	//events_org befüllen!!!

	//events nach i runden
	robustGeometry::BOEvent be;
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>::iterator
			row_it = events_org.begin();

	for (; row_it != events_org.end(); ++row_it) {
		be = *row_it;
		round(be);
	}

	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>::iterator
			batch_it = events_r.begin();

	for (; batch_it != events_r.end(); ++batch_it) {

		robustGeometry::BOEvent bo_tmp;
		bo_tmp = *batch_it;
		double i = bo_tmp.getX();
		updateMainActive(i - robustGeometry::ScaleFactor
		- robustGeometry::Epsilon, i + robustGeometry::ScaleFactor);

		//build tolerance square for events e
		computeToleranceSq();

		robustGeometry::BOEvent be;
		robustGeometry::ToleranceSquare ts;
		set<robustGeometry::ToleranceSquare,
		robustGeometry::CompToleranceSquareY>::iterator ts_it =
		tolSquare.begin(); //todo

		for (; ts_it != tolSquare.end(); ts_it++) {
			ts = *ts_it;
			locateValue(ts);
		}

	}

	//events 2 lines
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>::iterator
			re_it = result_e.begin();
	robustGeometry::BOEvent boEv_re;
	for (; re_it != result_e.end(); re_it++) {
		boEv_re = *re_it;
		splitSegment(boEv_re);
	}

	// das muss noch rein
	//	re_it = result_e.begin();
	//	robustGeometry::BOEvent boEv_re1;
	//	for (; re_it != result_e.end(); re_it++)
	//	{
	//		boEv_re1 = *re_it;
	//		event2line(boEv_re1);
	//	}
}

void MakeHobby::printHOInitialEvents() {

	set<robustGeometry::BOEvent>::iterator it;
	for (it = events_org.begin(); it != events_org.end(); ++it) {
		cout << it->getX() << " " << it->getY() << " Pointtype ";
		if (it->getPointType() == robustGeometry::start)
			cout << "start";
		else if (it->getPointType() == robustGeometry::end)
			cout << "end";
		else {
			robustGeometry::BOEvent tmpEv = *it;
			cout << "intersect" << " line1 "
					<< tmpEv.getLine1().getX1()
					<< " line2 "
					<< tmpEv.getLine1().getX2();

		}
		cout << endl;
	}
}
void MakeHobby::printHOEvents() {

	set<robustGeometry::BOEvent>::iterator it;
	for (it = events_r.begin(); it != events_r.end(); ++it) {
		cout << it->getX() << " " << it->getY();
		if (it->getPointType() == robustGeometry::start)
			cout << " start";
		else if (it->getPointType() == robustGeometry::end)
			cout << " end";
		else
			cout << " intersect";
		cout << endl;
	}
}

void MakeHobby::printHOResult_events() {

	set<robustGeometry::BOEvent>::iterator it;
	for (it = result_e.begin();
			it != result_e.end(); it++) {
		cout << "result_e: x " << it->getX()
				<< "  y " << it->getY();
		cout << endl;
	}
}

void MakeHobby::printHOResult_lines() {

	set<robustGeometry::BOLine>::iterator it;
	for (it = result_line.begin(); it !=
			result_line.end(); it++) {
		cout << it->getX1() << " "
				<< it->getY1() << " ";
		cout << it->getX2() << " "
				<< it->getY2();
		cout << endl;
	}
}

int MakeHobby::intersect(double sx1, double sy1,
		double ex2, double ey2,
		double sx3, double sy3, double ex4,
		double ey4) {
	//return =0 OK, 1 lines parallel, 2 no intersection point
	//x,y intersection point

	double a1 = 0.0;
	double a2 = 0.0;
	double b1 = 0.0;
	double b2 = 0.0;
	double c1 = 0.0;
	double c2 = 0.0; // Coefficients of line
	double m = 0.0;
	double x;
	double y;

	a1 = ey2 - sy1;
	b1 = sx1 - ex2;
	c1 = ex2 * sy1 - sx1 * ey2;
	//a1*x + b1*y + c1 = 0 is segment 1

	a2 = ey4 - sy3;
	b2 = sx3 - ex4;
	c2 = ex4 * sy3 - sx3 * ey4;
	//a2*x + b2*y + c2 = 0 is segment 2

	m = a1 * b2 - a2 * b1;
	if (m == 0)
		return 1;

	x = (b1 * c2 - b2 * c1) / m;
	// intersection range

	if (x < sx1 || x < sx3 || x > ex2 || x > ex4)
		return 2;

	y = (a2 * c1 - a1 * c2) / m;

	return 0;
}
//zu prüfen: statt Bereich, Schnitt mit Segment?
void MakeHobby::updateMainActive(double von, double bis) {
	robustGeometry::BOEvent bo_tmp;
	mainActive.clear();
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>::iterator
			bo_it = events_org.begin();

	for (; bo_it != events_org.end(); bo_it++) {
		if ((bo_it->getX() >= von)
				&& (bo_it->getX() < bis)) {
			bo_tmp = *bo_it;
			mainActive.insert(bo_tmp);
		} else if (bo_it->getX() > bis)
			break;
	}
}
void MakeHobby::locateValue(robustGeometry::ToleranceSquare & ts) {
	robustGeometry::BOEvent boEv;
	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>::iterator
			ma_it = mainActive.begin();

	for (; ma_it != mainActive.end(); ++ma_it) {
		boEv = *ma_it;
		double y_org = boEv.getY();
		double x_org = boEv.getX();
		if ((y_org <= ts.getY11()) && (y_org > ts.getY21()) && (x_org
				>= ts.getX11()) && (x_org <= ts.getX12())) {
			snapSegment(boEv, ts);
		}
	}
}

void MakeHobby::snapSegment(robustGeometry::BOEvent & boEv,
		robustGeometry::ToleranceSquare & ts) {
	double x = 0.0;
	double y = 0.0;

	x = ts.getSnapX();
	y = ts.getSnapY();

	boEv.setX(x);
	boEv.setY(y);
	result_e.insert(boEv);

}

void MakeHobby::getRoundedPoints
(const robustGeometry::BOEvent ** bo_s,
		const robustGeometry::BOEvent ** bo_e,
		robustGeometry::BOLine & line) {
	set<robustGeometry::BOEvent>::iterator seg_it1;

	for (seg_it1 = events_r.begin();
			seg_it1 != events_r.end(); ++seg_it1) {
		robustGeometry::BOEvent tmpSeg = *seg_it1;
		if ((line.getX1() == tmpSeg.getLine().getX1())
				&& (line.getY1()
				== tmpSeg.getLine().getY1())
				&& (line.getX2()
				== tmpSeg.getLine().getX2())
				&& (line.getY2()
				== tmpSeg.getLine().getY2())
				&& (line.getOwner()
				== tmpSeg.getLine().getOwner())) {

		if (tmpSeg.getPointType() == robustGeometry::start) {
			cout << "start " ; seg_it1->Print( cout ) ;
			cout << endl;
			*bo_s = &*seg_it1;
		} else if (tmpSeg.getPointType() == robustGeometry::end) {
			cout << "end " ;  seg_it1->Print( cout ) ; cout << endl;
			*bo_e = &*seg_it1;
		} else
			cout << "irgendwas " << endl;
		}


	}
}
const robustGeometry::BOEvent * MakeHobby::getRoundedPoint(
		robustGeometry::BOEvent & boEv) {
	set<robustGeometry::BOEvent>::iterator seg_it;
	for (seg_it = events_r.begin();
			seg_it != events_r.end(); ++seg_it) {
		robustGeometry::BOEvent tmpSeg = *seg_it;
		if (boEv.getPointType() == robustGeometry::end) {
			if ((boEv.getX() == tmpSeg.getLine().getX2())
					&& (boEv.getLine().getOwner()
					== tmpSeg.getLine().getOwner())
					&& (boEv.getY()
					== tmpSeg.getLine().getY2())) {
				return &*seg_it;
			}
		}
		if (boEv.getPointType() == robustGeometry::start) {
			if ((boEv.getX() == tmpSeg.getLine().getX1())
					&& (boEv.getLine().getOwner()
					== tmpSeg.getLine().getOwner())
					&& (boEv.getY()
					== tmpSeg.getLine().getY1())) {
				return &*seg_it;
			}
		}
	}
	return 0;
}

void MakeHobby::splitSegment(robustGeometry::BOEvent & boEv) {
	//const double is_x = 0.0;
	//const double is_y = 0.0;

	const robustGeometry::BOEvent* boEv_sl1 = 0;
	const robustGeometry::BOEvent* boEv_el1 = 0;
	const robustGeometry::BOEvent* boEv_sl2 = 0;
	const robustGeometry::BOEvent* boEv_el2 = 0;

	//im Fall von Intersection neues Segment erzeugen
	if (boEv.getPointType() == robustGeometry::intersect) {
		//is_x = boEv.getX();
		//is_y = boEv.getY();

		//getSNum( &s, sN, 2 );
		getRoundedPoints(&boEv_sl1, &boEv_el1,
				boEv.getLine1());
		getRoundedPoints(&boEv_sl2, &boEv_el2,
				boEv.getLine2());

		cout << " sl1-X  " << boEv_sl1->getX()
			<< " sl1-Y  " << boEv_sl1->getY() << endl;
		cout << " el1-X  " << boEv_el1->getX()
			<< " el1-Y  " << boEv_el1->getY() << endl;
		robustGeometry::BOLine boLine_s1o = robustGeometry::BOLine(
				boEv_sl1->getX(),boEv_sl1->getY(),
				boEv.getX(), boEv.getY(),
				boEv.getLine1().getOwner());
		result_line.insert(boLine_s1o);
		robustGeometry::BOLine boLine_s1u =
				robustGeometry::BOLine(
				boEv.getX(), boEv.getY(),
				boEv_el1->getX(), boEv_el1->getY(),
				boEv.getLine1().getOwner());
		result_line.insert(boLine_s1u);
		robustGeometry::BOLine boLine_s2o = robustGeometry::BOLine(
				boEv_sl2->getX(), boEv_sl2->getY(),
				boEv.getX(), boEv.getY(),
				boEv.getLine2().getOwner());
		result_line.insert(boLine_s2o);
		robustGeometry::BOLine boLine_s2u =
				robustGeometry::BOLine(boEv.getX(), boEv.getY(),
				boEv_el2->getX(), boEv_el2->getY(),
				boEv.getLine2().getOwner());
		result_line.insert(boLine_s2u);

	}
}

void MakeHobby::event2line(robustGeometry::BOEvent & boEv) {
	//const SNum* s = 0;
	const robustGeometry::BOEvent* boEv_s;
	const robustGeometry::BOEvent* boEv_e;

	//im Fall von Intersection neues LineSegment erzeugen
	//
	if (boEv.getPointType() != robustGeometry::intersect) {

		if (boEv.getPointType() == robustGeometry::start) {
			boEv_e = getRoundedPoint(boEv);
			robustGeometry::BOLine boLine_s =
					robustGeometry::BOLine(
					boEv.getX(), boEv.getY(),
					boEv_e->getX(), boEv_e->getY(),
					boEv.getLine().getOwner());
			result_line.insert(boLine_s);
		} else {
			boEv_s = getRoundedPoint(boEv);
			robustGeometry::BOLine boLine_e =
					robustGeometry::BOLine(
					boEv_s->getX(), boEv_s->getY(),
					boEv.getX(), boEv.getY(),
					boEv.getLine().getOwner());
			result_line.insert(boLine_e);
		}
	}
}
/*
 tolerance square is open along right.
 It's sufficient to check intersection:
 - with the segment and any hot pixel edge
 - between the segment and both the left and bottom edges
*/
bool MakeHobby::computeAllIntersections(robustGeometry::BOEvent & boEv,
		robustGeometry::ToleranceSquare & tolS) {
	double x1 = 0.0;
	double y1 = 0.0;
	double x2 = 0.0;
	double y2 = 0.0;
	double i11 = 0.0;
	double j11 = 0.0;
	double i12 = 0.0;
	double j12 = 0.0;
	double i21 = 0.0;
	double j21 = 0.0;
	double i22 = 0.0;
	double j22 = 0.0;
	int isIntersected = 1;

	x1 = boEv.getLine().getX1();
	y1 = boEv.getLine().getY1();
	x2 = boEv.getLine().getX2();
	y2 = boEv.getLine().getY2();
	i11 = tolS.getX11();
	j11 = tolS.getY11();
	i12 = tolS.getX12();
	j12 = tolS.getY12();
	i21 = tolS.getX21();
	j21 = tolS.getY21();
	i22 = tolS.getX22();
	j22 = tolS.getY22();
	//top
	isIntersected = intersect(x1, y1, x2, y2, i11, j11, i12, j12);
	if (isIntersected == 0)
		return true;
	//bottom
	isIntersected = intersect(x1, y1, x2, y2, i21, j21, i22, j22);
	if (isIntersected == 0)
		return true;
	//left
	isIntersected = intersect(x1, y1, x2, y2, i11, j11, i21, j21);
	if (isIntersected == 0)
		return true;
	//right
	isIntersected = intersect(x1, y1, x2, y2, i12, j12, i22, j22);
	if (isIntersected == 0)
		return true;

	return false;
}
void MakeHobby::intersectsToleranceSquare(robustGeometry::BOEvent & boEv) {
	bool isIntersected = false;
	double y_org = 0.0;

	//search corresponding tolerance square
	y_org = boEv.getY();
	set<robustGeometry::ToleranceSquare,
	robustGeometry::CompToleranceSquareY>::iterator
			ts_it = tolSquare.begin();

	for (; ts_it != tolSquare.end(); ++ts_it) {
		if ((y_org >= (ts_it->getY11() - robustGeometry::ScaleFactor))
				&& (y_org <= (ts_it->getY12() +
						robustGeometry::ScaleFactor))) {
			robustGeometry::ToleranceSquare tolSq = *ts_it;
			isIntersected = computeAllIntersections(boEv, tolSq);
			if (isIntersected == true) {
				snapSegment(boEv, tolSq);
			}
		}
	}
}

void MakeHobby::round(robustGeometry::BOEvent & boEv) {

	double x = 0.0;
	double y = 0.0;
	int xi = 0;
	int yi = 0;

	const robustGeometry::BOPointType pType = boEv.getPointType();
	const robustGeometry::BOLine* line = &boEv.getLine();

	xi = (int) (boEv.getX() + robustGeometry::ScaleFactor);
	x = xi;
	yi = (int) (boEv.getY() + robustGeometry::ScaleFactor);
	y = yi;

	robustGeometry::BOEvent be = robustGeometry::BOEvent(x,
			y, pType, *line);
	events_r.insert(be);
};

void MakeHobby::computeToleranceSq() {

	double x11 = 0.0;
	double y11 = 0.0;
	double x12 = 0.0;
	double y12 = 0.0;
	double x21 = 0.0;
	double y21 = 0.0;
	double x22 = 0.0;
	double y22 = 0.0;
	double snap_x = 0.0;
	double snap_y = 0.0;
	robustGeometry::BOEvent boEv;

	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY>::iterator
			ev_it1 = events_r.begin();
	for (; ev_it1 != events_r.end(); ++ev_it1) {
		robustGeometry::BOEvent boEv = *ev_it1;
		x11 = boEv.getX() - robustGeometry::ScaleFactor
				- robustGeometry::Epsilon;
		y11 = boEv.getY() + robustGeometry::ScaleFactor;
		x12 = boEv.getX() + robustGeometry::ScaleFactor;
		y12 = boEv.getY() + robustGeometry::ScaleFactor;
		x21 = boEv.getX() - robustGeometry::ScaleFactor
				- robustGeometry::Epsilon;
		y21 = boEv.getY() - robustGeometry::ScaleFactor;
		x22 = boEv.getX() + robustGeometry::ScaleFactor;
		y22 = boEv.getY() - robustGeometry::ScaleFactor;
		snap_x = boEv.getX();
		snap_y = boEv.getY();
		robustGeometry::ToleranceSquare
		ts = robustGeometry::ToleranceSquare(
				x11, y11, x12, y12, x21, y21,
				x22, y22, snap_x, snap_y, boEv);
		tolSquare.insert(ts);
	}

}
;

void MakeBO::IntersectionBO(const Line& line1, const Line& line2,
		Line& result) {
	// Initialisation
	result.Clear();

	result.SetDefined(true);
	if (line1.Size() == 0 || line2.Size() == 0) {
		return; //empty line
	}

	//Initialize event queue: all segment endpoints
	//Sort x by increasing x and y
	priority_queue<avlseg::ExtendedHalfSegment, vector<
			avlseg::ExtendedHalfSegment> ,
			greater<avlseg::ExtendedHalfSegment> >
			q1;
	priority_queue<avlseg::ExtendedHalfSegment, vector<
			avlseg::ExtendedHalfSegment> ,
			greater<avlseg::ExtendedHalfSegment> >
			q2;

	avltree::AVLTree<avlseg::AVLSegment> sss;
	avlseg::ownertype owner;

	int pos1 = 0;
	int pos2 = 0;
	avlseg::ExtendedHalfSegment nextExtHs;
	int src = 0;

	//const avlseg::AVLSegment* member=0;
	//const avlseg::AVLSegment* leftN = 0;
	//const avlseg::AVLSegment* rightN = 0;

	avlseg::AVLSegment left1, right1, common1,
	left2, right2;
	avlseg::AVLSegment tmpL, tmpR;
	robustGeometry::BOOwnerType boOwner;

	result.StartBulkLoad();

	while ((owner
			= selectNext(line1, pos1, line2,
					pos2, q1, q2, nextExtHs, src))
			!= avlseg::none) {

		if (owner == avlseg::first)
			boOwner = robustGeometry::first;
		else
			boOwner = robustGeometry::second;

		robustGeometry::BOLine boLine = robustGeometry::BOLine(
				nextExtHs.GetLeftPoint().GetX(),
				nextExtHs.GetLeftPoint().GetY(),
				nextExtHs.GetRightPoint().GetX(),
				nextExtHs.GetRightPoint().GetY(), boOwner);
		robustGeometry::BOEvent boEvent_s = robustGeometry::BOEvent(
				nextExtHs.GetLeftPoint().GetX(),
				nextExtHs.GetLeftPoint().GetY(),
				robustGeometry::start, boLine);
		addInitialEvent(boEvent_s);
		robustGeometry::BOEvent boEvent_e = robustGeometry::BOEvent(
				nextExtHs.GetRightPoint().GetX(),
				nextExtHs.GetRightPoint().GetY(),
				robustGeometry::end, boLine);
		addInitialEvent(boEvent_e);

	};

	doBOCases();
	// makeBO->printBOEvents();
	MakeHobby* makeHO = new MakeHobby();
	makeHO->set_BOEvents(get_BOEvents());
	// makeHO->printHOInitialEvents();
	makeHO->doHO();
	//	makeHO->printHOEvents();
	//makeHO->printHOResult_events();
	makeHO->printHOResult_lines();

	set<robustGeometry::BOLine,robustGeometry::
	CompBOLineXYOwn >::iterator rs_it;
	for (rs_it = makeHO->get_BOLine().begin();
			rs_it != makeHO->get_BOLine().end(); rs_it++)
	{
		robustGeometry::BOLine tmpSeg = *rs_it;

		Point p,q;
		p.Set(rs_it->getX1(), rs_it->getY1());
		q.Set(rs_it->getX2(), rs_it->getY2());
		HalfSegment hs1(true,p,q);
		HalfSegment hs2(false,p,q);
		result += hs1;
		result += hs2;
	}
	result.EndBulkLoad(true, false);
}

/*
 ~Intersection~ operation.

*/

static int intersectionBO_ll(Word* args, Word& result, int message,
		Word& local, Supplier s) {

	result = qp->ResultStorage(s);
	Line *line1 = ((Line*) args[0].addr);
	Line *line2 = ((Line*) args[1].addr);

	cerr << "line1 = ";
	line1->Print(cerr);
	cerr << endl;
	cerr << "line2 = ";
	line2->Print(cerr);
	cerr << endl;

	if (line1->IsDefined() && line2->IsDefined()) {
		if (line1->IsEmpty() || line2->IsEmpty()) {
			cout << __PRETTY_FUNCTION__ << ": "
					"lines are empty!" << endl;
			((Line *) result.addr)->SetDefined(false);
			return (0);
		} else if (line1->BoundingBox().IsDefined()
				&& line2->BoundingBox().IsDefined()) {
			if (line1->BoundingBox().
					Intersects(line2->BoundingBox())) {
				MakeBO bo;
				cout << __PRETTY_FUNCTION__ << ": "
						"BoundingBox Intersects!"
						<< endl;
				bo.IntersectionBO(*line1, *line2,
				*static_cast<Line*> (result.addr));

				return (0);
			} else {
				cout << __PRETTY_FUNCTION__ << ": "
					"BoundingBox don't intersects!" << endl;
				((Line *) result.addr)->Clear();
				return (0);
			}
		} else {
			MakeBO bo;
			cout << __PRETTY_FUNCTION__ << ": "
					"BoundingBox isn't defined!"
					<< endl;
			bo.IntersectionBO(*line1, *line2,
					*static_cast<Line*> (result.addr));
			return (0);
		}
	} else {
		cout << __PRETTY_FUNCTION__ << ": "
				"lines aren't defined!" << endl;
		cout << __PRETTY_FUNCTION__ << ": value mapping "
				"implemented only for lines."
		         << endl;
		((Line *) result.addr)->Clear();
		((Line *) result.addr)->SetDefined(false);
		//assert(false); // TODO: Implement IntersectionBOGeneric.

		return (0);
	}
}

/*
 Operator Description

*/

struct intersectionInfo: OperatorInfo {
	intersectionInfo() {
		name = "intersectionBO";
		signature = Line::BasicType()
		+ " x " + Line::BasicType() + " -> "
				+ Line::BasicType();
		syntax = "_ intersectionBO _";
		meaning = "Intersection predicate for two Lines.";
	}
};

/*
 Selection function ~RGSetOpSelect~
 This select function is used for the ~intersectionBO~ operator.

*/
int RGSetOpSelect(ListExpr args) {
	string a1 = nl->SymbolValue(nl->First(args));
	string a2 = nl->SymbolValue(nl->Second(args));

	if (a1 == Line::BasicType()) {
		if (a2 == Line::BasicType())
			return 0;
		return -1;
	}

	return -1;
}

ValueMapping intersectionVM[] = { intersectionBO_ll };
/*
 Definition of the operators

*/
Operator test("intersectionBO", intersectionSpec, 1, intersectionVM,
		RGSetOpSelect, intersectionTM);

/*
 Creating the Algebra

*/

class RobustGeometryAlgebra: public Algebra {

public:
	RobustGeometryAlgebra() :
		Algebra() {
		AddOperator(&test);
	}
	~RobustGeometryAlgebra() {
	}
	;
};

/*
 Algebra Initialization

*/

extern "C" Algebra*
InitializeRobustGeometryAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
	nl = nlRef;
	qp = qpRef;
	return (new RobustGeometryAlgebra());
};

