import java.lang.reflect.*;
import java.io.*;

public class Regions implements Serializable{
    //this class implements the Regions value of the ROSE algebra

    //members
    public TriList trilist; //the triangle set representing the Regions value
    public double perimeter; //the Regions' perimeter
    public double area; //the Regions' area 

    //constructors
    public Regions() {
	System.out.println("--> constructed an empty REGIONS object");
	trilist = new TriList();
	perimeter = 0;
	area = 0;
    }

    public Regions(TriList tl) {
	//System.out.println("R.const: entered constructor");
	System.out.print("--> constructing a REGIONS object from triangle list...");
	if (tl.isEmpty()) {
	    trilist = tl;
	    perimeter = 0;
	    area = 0;
	}//if
	if(!isRegularTriList(tl)) {
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
	System.out.println("done");
    }

    public Regions(SegList sl) {
	System.out.print("--> constructing a REGIONS object from segment list (size: "+sl.size()+")...");
	if (sl.isEmpty()) {
	    trilist = new TriList();
	    perimeter = 0;
	    area = 0;
	}//if
	else {
	    System.out.print("triangles...");
	    trilist = computeTriList(sl);
	    //System.out.print("perimeter...");
	    //perimeter = computePerimeter();
	    //System.out.print("area...");
	    //area = computeArea();
	    System.out.println("done");
	}//else
    }

    public Regions(Lines l) {
	System.out.print("--> constructing a REGIONS object from LINES object...");
	if (l.seglist.isEmpty()) {
	    trilist = new TriList();
	    perimeter = 0;
	    area = 0;
	}//if
	trilist = computeTriList(l.seglist);
	perimeter = computePerimeter();
	area = computeArea();
	System.out.println("done");
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
	
	/*
	if (tl.size() > 0) {
	    TriList tlcop = (TriList)tl.copy();
	    GFXout g = new GFXout();
	    Rational fact = RationalFactory.constRational(40);
	    for (int i = 0; i < tlcop.size(); i++) {
		((Triangle)tlcop.get(i)).zoom(fact); }
	    g.initWindow();
	    g.addList(tlcop);
	    g.showIt();
	    try { int data = System.in.read(); }
	    catch (Exception e) { System.exit(0); }
	    g.kill();
	}
	*/

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
	    System.exit(0);
	}//catch
	return false;
    }//end method isRegularTriList


    protected ElemListListList cyclesPoints() {
	//returns the cycles of this as an ElemListListList
	//which has points as elements
	Polygons pol = new Polygons(trilist);
	return pol.cyclesPoints();
    }//end method cyclesPoints


}//end class Regions
