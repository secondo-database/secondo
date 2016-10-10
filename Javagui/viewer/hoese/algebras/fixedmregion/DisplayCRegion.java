//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

package viewer.hoese.algebras.fixedmregion;

import java.awt.Polygon;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import sj.lang.ListExpr;
import viewer.hoese.QueryResult;
import viewer.hoese.algebras.DisplayGraph;

/**
 * 
 * Base class for displaying a CRegion object.
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class DisplayCRegion extends DisplayGraph {
    /** The cregion to display */
    private CRegion creg;
    
    @Override
    public int numberOfShapes() {
        return creg.getFaces().size();
    }

    @Override
    public Shape getRenderObject(int num, AffineTransform at) {
        if (creg == null)
            return null;
        Area a = getAreaFromFace(creg.getFaces().get(num));
        return a;
    }
    
    /**
     * Returns a displayable Area object from a CFace.
     * The resulting area is only an approximation of the real shape.
     * 
     * @param cface The CFace to create an area for
     * @return The area to display
     */
    private Area getAreaFromFace(CFace cface) {
        Polygon p = new Polygon();
        /** The number of points per curved segment. Higher value means better
            approximation. */
        final int nrpoints = 10;
        
        // Create the main polygon cycle
        for (RCurve r : cface.getRCurves()) {
            for (Point pt : r.getPoints(nrpoints))
                p.addPoint((int)pt.x, (int)pt.y);
        }
        // Make an area from it
        Area a = new Area(p);
        // And subtract all holes
        for (CFace h : cface.getHoles()) {
            p = new Polygon();
            for (RCurve r : h.getRCurves()) {
                for (Point pt : r.getPoints(nrpoints))
                    p.addPoint((int)pt.x, (int)pt.y);
            }
            a.subtract(new Area(p));
        }
        
        return a;
    }
    
    @Override
    public void init (String name, int nameWidth, int indent, ListExpr type, 
                      ListExpr value, QueryResult qr) {
        super.init(name, nameWidth, indent, type, value, qr);
        NL nl = new NL(value);
        creg = CRegion.deserialize(nl);
        
        qr.addEntry(this);
    }
}
