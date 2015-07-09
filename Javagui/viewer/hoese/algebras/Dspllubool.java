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

import sj.lang.ListExpr;

import viewer.hoese.Interval;
import viewer.hoese.QueryResult;
import viewer.hoese.DsplGeneric;
import viewer.hoese.LEUtils;

/**
 * A displayclass for the lubool
 */
public class Dspllubool extends DsplGeneric {

    DecimalFormat df = new DecimalFormat(",###.###");

    boolean value;
    Interval interval;

    @Override
    public void init(String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
        String T = name;
        interval = null;
        String V = null;
        if (isUndefined(value)) {
            return;
        }

        if (value.listLength() == 2) {
            interval = LEUtils.readInterval(value.first());
            if (interval != null) {
                ListExpr boolval = value.second();
                if (boolval.atomType() != ListExpr.BOOL_ATOM) {
                    interval = null;
                } else {
                    this.value = boolval.boolValue();
                }
            }
        }

        if (interval == null && V == null) {
            V = "<error>";
        } else {
            V = "" + (interval.isLeftclosed() ? "[" : "(")
                    + df.format(interval.getStart()) + " , "
                    + df.format(interval.getEnd())
                    + (interval.isRightclosed() ? "]" : ")")
                    + " -> " + this.value;
        }

        T = extendString(T, nameWidth, indent);
        qr.addEntry(T + " : " + V);
    }

}
