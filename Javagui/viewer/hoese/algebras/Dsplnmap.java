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

package  viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;

/**
 * The displayclass of the NauticalMap algebras nmap datatype.
 */
public class Dsplnmap extends DisplayGraph {
/** The internal datatype representation */


  /**
   * Init. the Dsplnmap instance.
   * @param type The symbol nmap
   * @param value maximal three relations with spatial elements(points, lines, regions).
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplnmapsrc.html#init">Source</a>
   */
   public ListExpr CreateObjectsTypeInfo()
   {
     return
       ListExpr.twoElemList( ListExpr.symbolAtom( "rel" ),
         ListExpr.twoElemList( ListExpr.symbolAtom( "tuple" ),
           ListExpr.twoElemList( ListExpr.twoElemList( ListExpr.symbolAtom( "name" ),
                                                       ListExpr.symbolAtom( "string" ) ),
                                 ListExpr.twoElemList( ListExpr.symbolAtom( "object" ),
                                                       ListExpr.symbolAtom( "point" ) ) ) ) );
   }

   public ListExpr CreateLinesTypeInfo()
   {
     return
       ListExpr.twoElemList( ListExpr.symbolAtom( "rel" ),
         ListExpr.twoElemList( ListExpr.symbolAtom( "tuple" ),
           ListExpr.twoElemList( ListExpr.twoElemList( ListExpr.symbolAtom( "name" ),
                                                       ListExpr.symbolAtom( "string" ) ),
                                 ListExpr.twoElemList( ListExpr.symbolAtom( "sline" ),
                                                       ListExpr.symbolAtom( "line" ) ) ) ) );
   }

   public ListExpr CreateRegionsTypeInfo()
   {
     return
       ListExpr.twoElemList( ListExpr.symbolAtom( "rel" ),
         ListExpr.twoElemList( ListExpr.symbolAtom( "tuple" ),
           ListExpr.twoElemList( ListExpr.twoElemList( ListExpr.symbolAtom( "name" ),
                                                       ListExpr.symbolAtom( "string" ) ),
                                 ListExpr.twoElemList( ListExpr.symbolAtom( "area" ),
                                                       ListExpr.symbolAtom( "region" ) ) ) ) );
   }


  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    value.writeListExpr();
    AttrName = type.symbolValue();
    ListExpr nmap;

       if (!value.fifth().isEmpty())
       {
          ListExpr rel3 = CreateRegionsTypeInfo();
          rel3.writeListExpr();
          LEUtils.analyse(rel3, value.fifth(), qr);
          qr.addEntry("---------");
       }
       if (!value.fourth().isEmpty())
       {
          ListExpr rel2 = CreateLinesTypeInfo();
          rel2.writeListExpr();
          LEUtils.analyse(rel2, value.fourth(), qr);
          qr.addEntry("---------");
       }
       if (!value.third().isEmpty())
       {
          ListExpr rel1 = CreateObjectsTypeInfo();
          rel1.writeListExpr();
          LEUtils.analyse(rel1, value.third(), qr);
          qr.addEntry("---------");
       }
/*
    if (nmap.listLength() == 2)
    {
       rel1= nmap.third();
       rel1.writeListExpr();
       LEUtils.analyse(rel1, value.third(), qr);
       qr.addEntry("---------");
    }
    else if (nmap.listLength() == 4)
    {
       rel2= nmap.fourth();
//     rel2.writeListExpr();
       LEUtils.analyse(rel2, value.fourth(), qr);
       qr.addEntry("---------");
       rel1= nmap.third();
//     rel1.writeListExpr();
       LEUtils.analyse(rel1, value.third(), qr);
       qr.addEntry("---------");
    }
    else if (nmap.listLength() == 5)
    {
       rel3= nmap.fifth();
//     rel3.writeListExpr();
       LEUtils.analyse(rel3, value.fifth(), qr);
       qr.addEntry("---------");
       rel2= nmap.fourth();
//     rel2.writeListExpr();
       LEUtils.analyse(rel2, value.fourth(), qr);
       qr.addEntry("---------");
       rel1= nmap.third();
//     rel1.writeListExpr();
       LEUtils.analyse(rel1, value.third(), qr);
       qr.addEntry("---------");
    }
*/
    else
    {
       Reporter.writeError("Error in ListExpr :parsing aborted");
       qr.addEntry(new String("(" + AttrName + ": GA(nmap))"));
       return;
    }

  }

}



