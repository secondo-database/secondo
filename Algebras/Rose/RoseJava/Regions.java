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

import java.lang.reflect.*;
import java.io.*;
import java.util.*;

public class Regions implements Serializable{
    //this class implements the Regions value of the ROSE algebra

    //members
    public TriList trilist; //the triangle set representing the Regions value
    public double perimeter; //the Regions' perimeter
    public double area; //the Regions' area 

    //constructors
    public Regions() {
	//System.out.println("--> constructed an empty REGIONS object");
	trilist = new TriList();
	perimeter = 0;
	area = 0;
    }

    public Regions(TriList tl) {
	//System.out.println("R.const: entered constructor");
	//System.out.print("--> constructing a REGIONS object from triangle list("+tl.size()+")...");
	if (tl.isEmpty()) {
	    trilist = tl;
	    perimeter = 0;
	    area = 0;
	}//if
	else if(!isRegularTriList(tl)) {
	    System.out.println("Error in Regions: tried to construct bad Region.");
	    System.exit(0);
	}//if
	else {
	    //System.out.println("R.const: entered else...");
	    trilist = TriList.convert(tl.copy());
	    //System.out.println("R.const: converted trilist");
	    //perimeter = computePerimeter();
	    //System.out.println("R.const: computed perimeter");
	    //area = computeArea();
	    //System.out.println("R.const: computed area");
	}//else
	//System.out.println("done"); 
    }

    public Regions(SegList sl) {
	//System.out.print("--> constructing a REGIONS object from segment list (size: "+sl.size()+")...");
	//System.out.println("\nsegList: "); sl.print();
	if (sl.isEmpty()) {
	    trilist = new TriList();
	    perimeter = 0;
	    area = 0;
	}//if
	else {
	    //System.out.print("triangles...");
	    trilist = computeTriList(sl);
	    //System.out.print("perimeter...");
	    //perimeter = computePerimeter();
	    //System.out.print("area...");
	    //area = computeArea();
	    //System.out.println("done.");
	}//else
    }

    public Regions(Lines l) {
	//System.out.print("--> constructing a REGIONS object from LINES object...");
	if (l.seglist.isEmpty()) {
	    trilist = new TriList();
	    perimeter = 0;
	    area = 0;
	}//if
	else {
	    trilist = computeTriList(l.seglist);
	    //perimeter = computePerimeter();
	    //area = computeArea();
	    //System.out.println("done");
	}//else
    }

    //methods
    private TriList computeTriList(SegList sl) {
	//sl must be a regular border of a Regions value
	//returns the triangulation of the Region
	return Polygons.computeTriangles(sl);
    }//end method computeTriList
	

    private double computePerimeter() {
	//computes the Regions' perimeter
	return ROSEAlgebra.r_perimeter(this);
    }//end method computePerimeter

    
    private double computeArea() {
	//computes the Regions' area
	return ROSEAlgebra.r_area(this);
    }//end method computeArea

    
    private boolean isRegularTriList(TriList tl) {
	//returns true if tl doesn't have intersecting triangles
	//untested!!!

	//System.out.println("entering R.isRegularTriList...");

	Class c = (new Triangle()).getClass();
	Class[] paramList = new Class[1];
	boolean retVal = false;
	try {
	    paramList[0] = c;
	    //paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("pintersects",paramList);
	    int groupedSize = (SetOps.overlapGroup(tl,m,false)).size();
	    //System.out.println("R.iRTL: groupedSize: "+groupedSize+", tl.size: "+tl.size());
	    if (groupedSize == tl.size()) return true;
	    else { return false; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception was thrown in Regions.isRegularTriList(TriList):");
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.out.println("Exception Cause: "+e.getCause());
	    System.out.println("Exception String: "+e.toString());
	    e.printStackTrace();
	    System.exit(0);
	}//catch
	return false;
    }//end method isRegularTriList


    protected ElemListListList cyclesPoints() {
	//returns the cycles of this as an ElemListListList
	//which has points as elements
	//System.out.println("entering Regions.cyclesPoints()...trilist.size="+trilist.size());
	Polygons pol = new Polygons(trilist);
	ElemListListList elll = pol.cyclesPoints();
	//System.out.println("\ncomputed ElemListListList: "); elll.print();
	//System.out.println("leaving Regions.cyclesPoints()...");
	return elll;
    }//end method cyclesPoints

    
    public static Regions readFrom(byte[] buffer){
	try{
	    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
	    Regions res = (Regions) ois.readObject();
	    ois.close();
	    return res;
	} catch(Exception e){
	    return null;
	}
    }//end method readFrom


    /** this method serializes an object */
    public  byte[] writeToByteArray(){
	
	try{
	    ByteArrayOutputStream byteout = new ByteArrayOutputStream();
	    ObjectOutputStream objectout = new ObjectOutputStream(byteout);
	    objectout.writeObject(this);
	    objectout.flush();
	    byte[] res = byteout.toByteArray();
	    objectout.close();
	    return  res;
	} catch(Exception e) { return null; }
	
    }//end method writeToByteArray


    public int compare (Regions rIn) {
	//returns 0 if this == pin
	//as long as elements in sorted lists from the beginning to the
	//end are equal, traverse through the lists.
	//When the first elements are found which are not equal, then
	//return -1 if this has the smaller element
	//return +1 if rIn has the smaller element
	//if one list has less elements than the other and the first elements
	//are equal, then
	//return -1 if this is shorter than rIn
	//return +1 if rIn is shorter than this

	//first sort both trilists
	TriList thiscop = (TriList)this.trilist.clone();
	TriList rincop = (TriList)this.trilist.clone();
	
	SetOps.quicksortX(thiscop);
	SetOps.quicksortX(rincop);

	ListIterator lit1 = thiscop.listIterator(0);
	ListIterator lit2 = rincop.listIterator(0);
	
	Triangle actT1;
	Triangle actT2;
	byte res;
	while (lit1.hasNext() && lit2.hasNext()) {
	    actT1 = (Triangle)lit1.next();
	    actT2 = (Triangle)lit2.next();
	    res = actT2.compare(actT2);
	    if (!(res == 0)) return (int)res;
	}//while
	if (!lit1.hasNext() && !lit2.hasNext()) return 0;
	if (!lit1.hasNext()) return -1;
	else return 1;
    }//end method compare


    public Regions copy () {
	//return new Regions(this.trilist);
	Regions nr = new Regions();
	nr.trilist = TriList.convert(this.trilist.copy());
	return nr;
    }//end method copy
	
}//end class Regions
