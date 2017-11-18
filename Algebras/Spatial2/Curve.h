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

5 Curve

5.1 Overview

This file defines the abstract class ~Curve~, its child classes ~MoveCurve~
 and ~LineCurve~ and their methods.

Defines and includes.

*/

#ifndef SECONDO_CURVE_H
#define SECONDO_CURVE_H

#include "RectangleBB.h"

#include <vector>

namespace salr {

/*
Forward declarations and typedef.

*/
  class Crossings;

/*
5.2 Class ~Curve~

*/
  class Curve {

  public:

/*
The segment type constant for a point that specifies the starting location
 for a new subpath.

*/
    static const int SEG_MOVETO = 0;

/*
The segment type constant for a point that specifies the end point of a line
 to be drawn from the most recently specified point.

*/
    static const int SEG_LINETO = 1;

/*
The segment type constant for the pair of points that specify a quadratic
 parametric curve to be drawn from the most recently specified point.

*/
    static const int SEG_QUADTO = 2;

/*
The segment type constant that specifies that the preceding subpath should be
 closed by appending a line segment back to the point corresponding to the
 most recent SEG\_MOVETO.

*/
    static const int SEG_CLOSE = 4;


/*
Constants used inside this class.

*/
    static const int INCREASING = 1;
    static const int DECREASING = -1;
    static const double TMIN = 1E-3;
    static const int RECT_INTERSECTS = 0x80000000;

/*
Declaration of static methods.

*/
    static void insertMove(std::vector<Curve*> *curves, double x, double y);
    static void insertLine(std::vector<Curve*> *curves, double x0, double y0,
                           double x1, double y1);

    static void copyCurves(const std::vector<Curve*>* source,
                           std::vector<Curve*>* target);

    static int rectCrossingsForLine(int crossings,
                                    double rxmin, double rymin,
                                    double rxmax, double rymax,
                                    double x0, double y0,
                                    double x1, double y1);

    static int rectCrossingsForQuad(int crossings,
                                    double rxmin, double rymin,
                                    double rxmax, double rymax,
                                    double x0, double y0,
                                    double xc, double yc,
                                    double x1, double y1,
                                    int level);


/*
Declaration of class methods.

*/
    Curve(int direction) : direction(direction) {}
    int getDirection();
    Curve *getWithDirection(int direction);
    static int orderof(double x1, double x2);
    double refineTforY(double t0, double yt0, double y0);
    bool fairlyClose(double v1, double v2);
    virtual int compareTo(Curve *that, double *yrange);
    virtual void enlarge(RectangleBB *rec) = 0;
    bool findIntersect(Curve *that, double *yrange, double ymin,
                       int slevel, int tlevel,
                       double s0, double xs0, double ys0,
                       double s1, double xs1, double ys1,
                       double t0, double xt0, double yt0,
                       double t1, double xt1, double yt1);

/*
Declaration of virtual methods.

*/
    virtual ~Curve();
    virtual int getOrder() = 0;
    virtual double getXTop() = 0;
    virtual double getYTop() = 0;
    virtual double getXBot() = 0;
    virtual double getYBot() = 0;
    virtual double getXMin() = 0;
    virtual double getXMax() = 0;
    virtual double getX0() = 0;
    virtual double getY0() = 0;
    virtual double getX1() = 0;
    virtual double getY1() = 0;
    virtual double XforY(double y) = 0;
    virtual double TforY(double y) = 0;
    virtual double XforT(double t) = 0;
    virtual double YforT(double t) = 0;
    virtual double dXforT(double t, int deriv) = 0;
    virtual double dYforT(double t, int deriv) = 0;
    virtual double nextVertical(double t0, double t1) = 0;
    virtual Curve *getReversedCurve() = 0;
    virtual Curve *getSubCurve(double ystart, double yend, int dir) = 0;
    virtual bool accumulateCrossings(Crossings *c) = 0;
    virtual int getSegment(double *coords) = 0;

  protected:
/*
Field indication the direction of this instance.

*/
    int direction;
  };

/*
5.3 Class ~MoveCurve~

This class is used to model a move segment in a ~Region2~.

*/
  class MoveCurve : public Curve {
  public:

/*
Declaration of constructor.

*/
    MoveCurve(double x, double y);
/*
Declaration of destructor.

*/
    ~MoveCurve();

/*
Declaration of class methods.

*/
    int getOrder();
    double getXTop();
    double getYTop();
    double getXBot();
    double getYBot();
    double getXMin();
    double getXMax();
    double getX0();
    double getY0();
    double getX1();
    double getY1();
    double XforY(double y);
    double TforY(double y);
    double XforT(double t);
    double YforT(double t);
    double dXforT(double t, int deriv);
    double dYforT(double t, int deriv);
    double nextVertical(double t0, double t1);
    Curve *getSubCurve(double ystart, double yend, int dir);
    Curve *getReversedCurve();
    void enlarge(RectangleBB *rec);
    bool accumulateCrossings(Crossings *c);
    int getSegment(double *coords);

  private:
/*
Fields used to store the position of the start point of the segment following
 this move segment.

*/
    double x;
    double y;
  };

/*
5.4 Class ~LineCurve~

This class is used to model a line segment of a ~Region2~.

*/
  class LineCurve : public Curve {
  public:

/*
Declaration of constructor.

*/
    LineCurve(double x0, double y0, double x1, double y1, int direction);
/*
Declaration of destructor.

*/
    ~LineCurve();

/*
Declaration of class methods.

*/
    int getOrder();
    double getXTop();
    double getYTop();
    double getXBot();
    double getYBot();
    double getXMin();
    double getXMax();
    double getX0();
    double getY0();
    double getX1();
    double getY1();
    double XforY(double y);
    double TforY(double y);
    double XforT(double t);
    double YforT(double t);
    double dXforT(double t, int deriv);
    double dYforT(double t, int deriv);
    double nextVertical(double t0, double t1);
    Curve *getSubCurve(double ystart, double yend, int dir);
    Curve *getReversedCurve();
    int compareTo(Curve *other, double *yrange);
    void enlarge(RectangleBB *rec);
    bool accumulateCrossings(Crossings *c);
    int getSegment(double *coords);

  private:
/*
Fields used to store the position of the start point and the end point of
 this segment as well as the most left and most right position.

*/
    double x0;
    double y0;
    double x1;
    double y1;
    double xmin;
    double xmax;
  };

}

#endif //SECONDO_CURVE_H
