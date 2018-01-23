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

/*
9 Dsplregion2

9.1 Overview

This file contains the implementation to add the new type ~Region2~ of the
~SpatialLRAlgebra~ to the ~HoeseViewer~.

*/

package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.math.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;
import tools.Reporter;


/*
9.2 Class ~Dsplregion2

*/
public class Dsplregion2 extends DisplayGraph {
/*
The internal datastructure of the region2 datatype

*/
  Area areas;

/*
boolean flag indicating the defined state

*/
  boolean defined;

/*
The textual representation of this region2

*/
  String entry;

  public int numberOfShapes(){
     return 1;
  }

  public Shape getRenderObject(int num,AffineTransform at){
    if(num<1){
       return areas;
    } else{
       return null;
    }
  }

/*
Creates an instance of ~Area~ form the ~ListExpr~.

*/
  public boolean ScanValue (ListExpr expr) {
        if(isUndefined(expr)){
            defined=false;
            areas=null;
            return false;
        }
        if (expr.listLength() != 2) {
            Reporter.writeError("Invalid line2 expression: 4 elements needed");
            err = true;
            return false;
        }

        defined=true;
        GeneralPath gp = new GeneralPath();
        ListExpr types = expr.first();
        ListExpr coords = expr.second();
        int offsetCoords = 0;
        double x1, x2, y1, y2 = 0;

        while(!types.isEmpty()) {
            if(LEUtils.readNumeric(types.first()) == 0) {
                x1 = LEUtils.readNumeric(coords.first());
                coords = coords.rest();
                y1 = LEUtils.readNumeric(coords.first());
                coords = coords.rest();
                gp.moveTo(x1,y1);
            } else if(LEUtils.readNumeric(types.first()) == 1) {
                x1 = LEUtils.readNumeric(coords.first());
                coords = coords.rest();
                y1 = LEUtils.readNumeric(coords.first());
                coords = coords.rest();
                gp.lineTo(x1,y1);
            } else if(LEUtils.readNumeric(types.first()) == 4) {
                gp.closePath();
            }
            types = types.rest();
        }
        gp.closePath();
        areas = new Area(gp);
        return true;
  }

/*
Gets called by ~HoeseViewer~ to change a list expr to a java object.

*/
  public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
        AttrName = extendString(name,nameWidth, indent);

        ScanValue(value);
        if (err) {
            Reporter.writeError("Error in ListExpr :parsing aborted");
            entry = AttrName + " : <error>";
            qr.addEntry(entry);
            areas=null;
            return;
        }
        else if(!defined){
            entry = AttrName+" : undefined";
            qr.addEntry(entry);
            return;
        }
        // normal case-> defined line
        entry = AttrName + " : region";
        defined=areas!=null;
        err=false;
        qr.addEntry(this);
  }
}



