import twodsack.set.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.collection.*;
import twodsack.util.comparator.*;

import java.lang.reflect.*;
import java.io.*;
import java.util.*;

public class Regions implements Serializable{
    //this class implements the Regions value of the ROSE algebra

    //members
    public TriMultiSet triset = null; //the triangle set representing the Regions value
    public double perimeter = 0; //the Regions' perimeter
    public double area = 0; //the Regions' area 
    private boolean cyclesDefined = false; //true, if the cycle list was already computed
    private CycleListListPoints cycles = null; //the list of cycles

    //static counter variables
    private static int noOfTris = 0;
    private static int noOfRegions = 0;
    private static int readFromCalls = 0;

    //constructors
    public Regions() {
	//System.out.println("--> constructed an empty REGIONS object");
	triset = new TriMultiSet(new TriangleComparator());
	perimeter = 0;
	area = 0;
	cyclesDefined = false;
	cycles = null;
    }

    public Regions(TriMultiSet tl) {
	//System.out.println("R.const: entered constructor");
	//System.out.print("--> constructing a REGIONS object from triangle list("+tl.size()+")...");
	if (tl == null || tl.isEmpty()) {
	    triset = tl;
	    perimeter = 0;
	    area = 0;
	    cyclesDefined = false;
	    cycles = null;
	}//if
	else if(!isRegularTriSet(tl)) {
	    System.out.println("Error in Regions: tried to construct bad Region.");
	    System.exit(0);
	}//if
	else {
	    //System.out.println("R.const: entered else...");
	    triset = TriMultiSet.convert(tl.copy());
	    //System.out.println("R.const: converted trilset");
	    //perimeter = computePerimeter();
	    //System.out.println("R.const: computed perimeter");
	    //area = computeArea();
	    //System.out.println("R.const: computed area");
	    cyclesDefined = false;
	    cycles = null;
	}//else
	//System.out.println("done"); 
    }

    public Regions(SegMultiSet sl) {
	//System.out.print("--> constructing a REGIONS object from segment list (size: "+sl.size()+")...");
	//System.out.println("\nsegList: "); sl.print();
	if (sl == null || sl.isEmpty()) {
	    triset = new TriMultiSet(new TriangleComparator());
	    perimeter = 0;
	    area = 0;
	    cyclesDefined = false;
	    cycles = null;
	}//if
	else {
	    noOfRegions++;
	    System.out.println("\n########################### computation of triangle set for Region");
	    System.out.println("\n---> REGION_NO."+noOfRegions+": compute Triangles... ");
	    triset = computeTriSet(sl);
	    noOfTris += triset.size();
	    System.out.println(triset.size()+" triangle(s). Sum of triangles: "+noOfTris);
	    System.out.println("\n########################### computation of cycles for Region");
	    System.out.println("--->compute cycles... ");
	    cycles = cyclesPoints();
	    cyclesDefined = true;
	    System.out.println(cycles.size()+" cycle(s) found.");
	}//else
    }

    public Regions(Lines l) {
	//System.out.print("--> constructing a REGIONS object from LINES object...");
	if (l.segset.isEmpty()) {
	    triset = new TriMultiSet(new TriangleComparator());
	    perimeter = 0;
	    area = 0;
	}//if
	else {
	    triset = computeTriSet(l.segset);
	    //perimeter = computePerimeter();
	    //area = computeArea();
	    //System.out.println("done");
	}//else
    }

    //methods
    private TriMultiSet computeTriSet(SegMultiSet sms) {
	//sl must be a regular border of a Regions value
	//returns the triangulation of the Region
	return Polygons.computeMesh(sms,true);  
    }//end method computeTriSet
	
    /*
    private double computePerimeter() {
	//computes the Regions' perimeter
	return ROSEAlgebra.r_perimeter(this);
    }//end method computePerimeter
    */
    
    /*
    private double computeArea() {
	//computes the Regions' area
	return ROSEAlgebra.r_area(this);
    }//end method computeArea
    */
    
    private boolean isRegularTriSet(TriMultiSet tl) {
	//returns true if tl doesn't have intersecting triangles
	//untested!!!

	//System.out.println("entering R.isRegularTriSet...");

	return true;


	/*
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
	*/
    }//end method isRegularTriSet

    
    public CycleListListPoints cyclesPoints() {
	//returns the cycles of this as an CycleListListPoints
	//which has points as elements
	
	if (this.cyclesDefined) {
	    return this.cycles;
	}//if
	else {
	    Polygons pol = new Polygons(triset);
	    this.cycles = pol.cyclesPoints();
	    this.cyclesDefined = true;
	    return this.cycles;
	}//else
    }//end method cyclesPoints

    
    public static Regions readFrom(byte[] buffer){
	readFromCalls++;
	System.out.print("readFromCalls: "+readFromCalls+": ");

	try{
	    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
	    Regions res = (Regions) ois.readObject();
	    ois.close();
	    if (res.triset != null && res.cycles != null) 
		System.out.println(res.triset.size()+" triangles, "+res.cycles.size()+" cycle(s).");

	    return res;
	} catch(Exception e){
	    System.out.println("Error in Regions.readFrom().");
	    e.printStackTrace();
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
	} catch(Exception e) {
	    System.out.println("Error in Regions.writeToByteArray: "+e);
	    e.printStackTrace();
	    return null; }
	
    }//end method writeToByteArray

    /*
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

	//first sort both trisets
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
    */

    
    public Regions copy () {
	//return new Regions(this.trilist);
	Regions nr = new Regions();
	nr.triset = TriMultiSet.convert(this.triset.copy());
	nr.cyclesDefined = true;
	nr.cycles = this.cycles.copy();
	return nr; 
    }//end method copy

    public void print() {
	this.triset.print();
    }

}//end class Regions
