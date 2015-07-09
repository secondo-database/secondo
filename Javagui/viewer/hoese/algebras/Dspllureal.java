//This file is part of SECONDO.
//Copyright (C) 2009, University in Hagen, Department of Computer Science, 
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
package viewer.hoese.algebras;

import java.text.DecimalFormat;
import java.util.Vector;

import sj.lang.ListExpr;
import tools.Reporter;

import viewer.hoese.Interval;
import viewer.hoese.QueryResult;
import viewer.hoese.DsplGeneric;
import viewer.hoese.LEUtils;
import viewer.hoese.ExternDisplay;

/**
 * A displayclass for the lureal
 */
public class Dspllureal extends DsplGeneric
        implements ExternDisplay, Function {

    DecimalFormat df = new DecimalFormat(",###.###");

    double m, n;
    Interval interval;
    private static final LFunctionFrame functionframe = new LFunctionFrame();
    private boolean defined;
    private Interval boundingInterval;
    private Vector Intervals = new Vector(10, 5);
    private Vector LRealMaps = new Vector(10, 5);
    private boolean err;
    private String returnstring;

    /**
     * This method reads the parameter for the length function into a
     * LRealMap-instance
     *
     * @param le A 2 element list
     * @return A LRealMap-instance with the formula parameter
     */
    private LRealMap readLRealMap(ListExpr le) {
        Double value[] = {
            null, null
        };
        if (le.listLength() != 2) {
            return null;
        }
        for (int i = 0; i < 2; i++) {
            value[i] = LEUtils.readNumeric(le.first());
            if (value[i] == null) {
                return null;
            }
            le = le.rest();
        }
        return new LRealMap(value[0], value[1]);
    }

    /**
     * Scans the representation of a lureal datatype
     *
     * @param v A list of length-intervals
     * @see sj.lang.ListExpr
     * @see <a href="Dspllurealsrc.html#ScanValue">Source</a>
     */
    public void ScanValue(ListExpr v) {
        if (isUndefined(v)) {
            err = false;
            defined = false;
            return;
        }
        defined = true;

        ListExpr le = v;
        Interval in = null;
        ListExpr map = null;
        int len = le.listLength();
        if (len != 2) {
            return;
        }
        if (len == 2) {
            in = LEUtils.readInterval(le.first());
            map = le.second();
        }
        LRealMap lm = readLRealMap(map);
        if ((in == null) || (lm == null)) {
            return;
        }
        Intervals.add(in);
        LRealMaps.add(lm);
        err = false;
    }

    @Override
    public void init(String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
        String T = name;
        interval = null;
        String V = null;
        if (isUndefined(value)) {
            return;
        }
        ScanValue(value);
        if (value.listLength() == 2) {
            interval = LEUtils.readInterval(value.first());
            if (interval != null) {
                ListExpr functionvalues = value.second();
                if (functionvalues.first().atomType() != ListExpr.REAL_ATOM
                        || functionvalues.second().atomType() != ListExpr.REAL_ATOM) {
                    interval = null;
                } else {
                    this.m = functionvalues.first().realValue();
                    this.n = functionvalues.second().realValue();
                }
            }
        }

        if (interval == null && V == null) {
            V = "<error>";
        } else {
            qr.addEntry(this);
            V = "" + (interval.isLeftclosed() ? "[" : "(")
                    + df.format(interval.getStart()) + " , "
                    + df.format(interval.getEnd())
                    + (interval.isRightclosed() ? "]" : ")")
                    + " -> f(x) = " + df.format(this.m) + " x + " + df.format(this.n);
            T = extendString(T, nameWidth, indent);
            // compute the bounding box of all intervals
            boundingInterval = null;
            for (int i = 0; i < Intervals.size(); i++) {
                Interval in = (Interval) Intervals.elementAt(i);
                if (!in.isInfinite()) {
                    if (boundingInterval == null) {
                        boundingInterval = in.copy();
                    } else {
                        boundingInterval.unionInternal(in);
                    }
                }
            }
        }
        returnstring = T + " : " + V;
    }

    /**
     * The text representation of this object
     *
     * @return string representation of this object
     * @see <a href="Dspllengthrealsrc.html#toString">Source</a>
     */
    @Override
    public String toString() {
        return (returnstring);
    }

    @Override
    public boolean isExternDisplayed() {
        return (functionframe.isVisible() && this.equals(functionframe.getSource()));
    }

    @Override
    public void displayExtern() {
        if (!defined) {
            Reporter.showInfo("not defined");
            return;
        }
        if (boundingInterval != null) {
            functionframe.setSource(this);
            functionframe.setVisible(true);
            functionframe.toFront();
        } else {
            Reporter.showInfo("The length unit real is empty");
        }
    }

    /**
     * Computes the value of this real for a given instant. The instant is just
     * given as a double. If the lengt unit real is not defined at the given
     * instant null is returned.
     *
     * @param length is the position for which the value is searched
     * @return real value at the length position
     */
    @Override
    public Double getValueAt(double length) {
        int index = IntervalSearch.getTimeIndex(length, Intervals);
        return getValueAt(length, index);
    }

    /**
     * Computes the value for a given index There is no check wether the given
     * length is contained in the interval determined by index.
     *
     */
    private Double getValueAt(double length, int index) {
        Reporter.reportInfo("x " + length, true);
        // check for correct index
        if (index < 0 || index >= Intervals.size()) {
            return null;
        }
        Interval CurrentInterval = (Interval) Intervals.get(index);
        LRealMap CurrentMap = (LRealMap) LRealMaps.get(index);
        double start = CurrentInterval.getStart();
        length -= start;
        Reporter.reportInfo("length " + length, true);
        Reporter.reportInfo("start " + start, true);
        double polyvalue = start + CurrentMap.m * length + CurrentMap.n;
        Reporter.reportInfo("polyvalue " + polyvalue, true);
        return polyvalue;
    }

    /**
     * @return interval of this lengt unit*
     */
    @Override
    public Interval getInterval() {
        return boundingInterval;
    }
}

/**
 * The class which holds the formula parameter for an interval
 */
class LRealMap {

    double m, n;

    /**
     * Constructor
     *
     * @param double m Gradient
     * @param double n Intersection with y-Axis
     */
    public LRealMap(double x1, double x2) {
        m = x1;
        n = x2;
    }
}
