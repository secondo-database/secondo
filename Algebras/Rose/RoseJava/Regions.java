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
	trilist = new TriList();
	perimeter = 0;
	area = 0;
    }

    public Regions(TriList tl) {
	if(!isRegularTriList(tl)) {
	    System.out.println("Error in Regions: tried to construct bad Region.");
	    System.exit(0);
	}//if
	else {
	    trilist = TriList.convert(tl.copy());
	    perimeter = computePerimeter();
	    area = computeArea();
	}//else
    }

    public Regions(SegList sl) {
	trilist = computeTriList(sl);
	perimeter = computePerimeter();
	area = computeArea();
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

	Class c = (new Triangle()).getClass();
	Class[] paramList = new Class[1];
	boolean retVal = false;
	try {
	    paramList[0] = Class.forName("Element");
	    Method m = c.getMethod("intersects",paramList);
	    if ((SetOps.group(tl,m)).size() == tl.size()) {
		return true;
	    }//if
	    else { return false; }
	}//try
	catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
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
