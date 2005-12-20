import twodsack.set.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.collection.*;
import twodsack.util.comparator.*;

import java.lang.reflect.*;
import java.io.*;
import java.util.*;


/**
 * This class implements the Regions type. A Regions instance consists of a set of polygons which may have holes. A Region can be constructed
 * from a set of triangles (which usually is a triangulation of a set of polygons) or a set of segments (which form the polygons' borders).
 * Regions instances are one of the three geometic types, which are used as parameter types in the ROSE algebra. The other two are Points and Lines.
 */
public class Regions implements Serializable{
    /*
     * fields
     */
    /**
     * Stores the set of triangles of the region.
     */
    public TriMultiSet triset = null;

    /**
     * The bounding box of the Regions object.
     */
    private Rect bbox = null;

    /**
     * If true, a bbox was already computed and is valid.
     */
    private boolean bboxDefined = false;

    /**
     * The area of the Regions object.
     */
    public double area = 0.0;

    /**
     * A flag which indicates, whether the <tt>cycles</tt> of the polygon are defined or not.
     */
    private boolean cyclesDefined = false;


    /**
     * Stores the cycle list, which is a list represenation of the Regions' border.
     */
    private CycleListListPoints cycles = null; //the list of cycles


    /*
     * constructors
     */
    /**
     * Constructs an 'empty' Regions value.
     */
    public Regions() {
	triset = new TriMultiSet(new TriangleComparator());
	cyclesDefined = false;
	cycles = null;
	bbox = null;
	area = 0.0;
	bboxDefined = false;
    }


    /**
     * Constructs a new Regions value from a set of triangles.
     * <tt>cyclesDefined</tt> is set to <tt>false</tt>.
     *
     * @param tl the set of triangles
     */
    public Regions(TriMultiSet tl) {
	if (tl == null || tl.isEmpty()) {
	    triset = tl;
	    cyclesDefined = false;
	    cycles = null;
	    bbox = null;
	    area = 0.0;
	    bboxDefined = false;
	}//if
	else if(!isRegularTriSet(tl)) {
	    System.out.println("Error in Regions: tried to construct bad Region.");
	    System.exit(0);
	}//if
	else {
	    triset = TriMultiSet.convert(tl);
	    cyclesDefined = false;
	    cycles = null;
	    bbox = triset.rect();
	    area = ROSEAlgebra.r_area(this);
	    bboxDefined = true;
	}//else
    }


    /**
     * Constructs a new Regions value from a set of segments. 
     * <tt>cyclesDefined</tt> is set to <tt>true</tt>.
     *
     * @param sl the set of segments
     */
    public Regions(SegMultiSet sl) {
	if (sl == null || sl.isEmpty()) {
	    triset = new TriMultiSet(new TriangleComparator());
	    cyclesDefined = false;
	    cycles = null;
	    bbox = null;
	    area = 0.0;
	    bboxDefined = false;
	}//if
	else {
	    triset = computeTriSet(sl);
	    cycles = cyclesPoints();
	    cyclesDefined = true;
	    bbox = triset.rect();
	    area = ROSEAlgebra.r_area(this);
	    bboxDefined = true;
	}//else
    }


    /**
     * Constructs a new Regions value from a Lines value.
     * <tt>cyclesDefined</tt> is set to <tt>false</tt>.
     *
     * @param l the Lines value
     */
    public Regions(Lines l) {
	if (l.segset.isEmpty()) {
	    triset = new TriMultiSet(new TriangleComparator());
	    cyclesDefined = false;
	    cycles = null;
	    bbox = null;
	    area = 0.0;
	    bboxDefined = false;
	}//if
	else {
	    triset = computeTriSet(l.segset);
	    cyclesDefined = false;
	    cycles = null;
	    bbox = triset.rect();
	    area = ROSEAlgebra.r_area(this);
	    bboxDefined = true;
	}//else
    }


    /**
     * Constructs a new Regions value from an existing Regions value.
     * The triangle set is not copied.
     *
     * @param r the Regions value     
     */
    public Regions (Regions r) {
	this.triset = r.triset;
	this.cyclesDefined = r.cyclesDefined;
	this.cycles = r.cycles;
	this.area = r.area;
	this.bbox = r.bbox;
	this.bboxDefined = true;
    }


    /*
     * methods
     */
    /**
     * Returns the bounding box of the Regions object.
     */
    public Rect rect() {
	if (bboxDefined)
	    return bbox;
	else {
	    bbox = triset.rect();
	    bboxDefined = true;
	    return bbox;
	}//else
    }//end method rect


    /**
     * Computes the triangle set for a given set of segments.
     * The passed set of segment must form a proper border for a Regions value.
     *
     * @param sms the set of segments
     */
    private TriMultiSet computeTriSet(SegMultiSet sms) {
	//sl must be a regular border of a Regions value
	//returns the triangulation of the Region
	return Polygons.computeMesh(sms,false);  
    }//end method computeTriSet
	
    /**
     * Returns <tt>true</tt> if the passed set of triangles has no overlapping triangles.
     */    
    private boolean isRegularTriSet(TriMultiSet tl) {
	//returns true if tl doesn't have intersecting triangles
	//untested!!!

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

    
    /**
     * Returns a structure of nested lists that stores points.
     * The resulting <tt>CycleListListPoints</tt> stores the vertices of the Regions' cycles. It has a list of points (vertices)
     * for every face, the Regions value has. Then, every face consists of an outer cycle (again, it is represented by its border
     * points) and its holes. 'Islands' inside of holes are not supported.
     *
     * @return the nested list that describes the Regions value
     */
    public CycleListListPoints cyclesPoints() {
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


    /**
     * Constructs a Regions value from a byte array.
     * Given a byte array (probably from a disk access), a Regions value is constructed from it.
     * If the value cannot be restored properly, <tt>null</tt> is returned.
     *
     * @param buffer the byte array
     * @return the restored Regions value
     */    
    public static Regions readFrom(byte[] buffer){
	try{
	    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
	    Regions res = (Regions) ois.readObject();
	    ois.close();
	    if (res.triset != null && res.cycles != null) 
	    return res;
	} catch(Exception e) {
	    System.out.println("Error in Regions.readFrom(). Cannot restore Regions value properly.");
	    e.printStackTrace();
	}//catch
	return null;
    }//end method readFrom


    /**
     * Constructs a serialized Regions value.
     * From the Regions value, a byte array is constructed. Then, this array can be written to disk.
     *
     * @return the byte array
     */
    public byte[] writeToByteArray(){
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


    /**
     * Returs a copy of <tt>this</tt>.
     *
     * @return the copy
     */
    public Regions copy () {
	Regions nr = new Regions();
	nr.triset = TriMultiSet.convert(this.triset.copy());
	nr.cyclesDefined = true;
	nr.cycles = this.cycles.copy();
	return nr; 
    }//end method copy


    /**
     * Prints the triangle set to standard output.
     */
    public void print() {
	this.triset.print();
    }//end method print
}//end class Regions
