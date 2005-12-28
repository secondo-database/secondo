import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collectiontype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;
import java.lang.reflect.*;
import java.io.*;
import java.util.*;


/**
 * This class implements the Points type. A Points instance consists of a set of points. Points are one of the three geometric types, that are
 * used as parameter types in the ROSE algebra. The other two types are Regions and Lines.
 */
public class Points implements Serializable{
    /*
     * fields
     */
    /**
     * The set of points.
     */
    public PointMultiSet pointset;

    /**
     * The bounding box of the Points object.
     */
    private Rect bbox;

    /**
     * If true, a bbox was already computed and is valid
     */
    private boolean bboxDefined = false;

    /*
     * constructors
     */
    /**
     * Constructs an emtpy Points value.
     */
    public Points() {
	pointset = new PointMultiSet(new PointComparator());
	bbox = null;
	bboxDefined = false;
    }


    /**
     * Constructs a new Points value from a set of points.
     * Note that no copy is made of the set of points.
     *
     * @param pl the set of points
     */
    public Points(PointMultiSet pl) {
	pointset = pl;
	bbox = pl.rect();
	bboxDefined = true;
    }


    /**
     * Constructs a new Points value from an already existing Points value.
     * Note that no copy is made of the set of points.
     *
     * @param p the Points value
     */
    public Points(Points p) {
	this.pointset = p.pointset;
	this.bbox = p.bbox;
	this.bboxDefined = p.bboxDefined;
    }

    
    /*
     * methods
     */
    /**
     * Returns the bounding box of the Points object.
     */
    public Rect rect() {
	if (bboxDefined)
	    return bbox;
	else {
	    this.bbox = pointset.rect();
	    this.bboxDefined = true;
	    return bbox;
	}//if
    }//end method rect


    /**
     * Returns the size of the pointset.
     *
     * @return the size as int
     */
    public int size() {
	return this.pointset.size();
    }//end method size


    /**
     * Adds the given point <tt>p</tt> to the pointset.
     *
     * @param p the point
     */
    public void add(Point p) {
	this.pointset.add(p);
	this.bbox = null;
	this.bboxDefined = false;
    }//end method add


    /**
     * Constructs a new point from the given Rational coordinates and adds that point to the pointset.
     *
     * @param x the x coordinate for the new point
     * @param y the y coordinate for the new point
     */
    public void add(Rational x, Rational y) {
	this.pointset.add(new Point(x,y));
	this.bbox = null;
	this.bboxDefined = false;
    }//end method add


    /**
     * Constructs a new point from the given int coordinates and adds that point to the pointset.
     *
     * @param x the x coordinate for the new point
     * @param y the y coodrinate for the new point
     */
    public void add(int x, int y) {
	this.pointset.add(new Point(x,y));
	this.bbox = null;
	this.bboxDefined = false;
    }//end method add


    /**
     * Constructs a new point from the given double coordinates and adds that point to the pointset.
     *
     * @param x the x coordinate for the new point
     * @param y the y coordinate for the new point
     */
    public void add(double x, double y) {
	this.pointset.add(new Point(x,y));
	this.bbox = null;
	this.bboxDefined = false;
    }//end method add


    /**
     * Constructs a Points value from a byte array.
     * Given a byte array (probably from a disk access), a Points value is constructed from it.
     * If the value cannot be restored properly, <tt>null</tt> is returned.
     *
     * @param buffer the byte array
     * @return the restored Points value
     */
    public static Points readFrom(byte[] buffer){
	try{
	    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
	    Points res = (Points) ois.readObject();
	    ois.close();
	    return res;
	} catch(Exception e){
	    return null;
	}
    }//end method readFrom


    /**
     * Constructs a serialized Points value.
     * From the Points value, a byte array is constructed. Then, this array can be written to disk.
     *
     * @return the byte array
     */
    public  byte[] writeToByteArray(){
	try{
	    ByteArrayOutputStream byteout = new ByteArrayOutputStream();
	    ObjectOutputStream objectout = new ObjectOutputStream(byteout);
	    objectout.writeObject(this);
	    objectout.flush();
	    byte[] res = byteout.toByteArray();
	    objectout.close();
	    return  res;
	} catch(Exception e){
	    System.out.println("Error in Points.writeToByteArray."+e); 
	    e.printStackTrace();
	    return null;
	}
    }//end method writeToByteArray


    /**
     * Returns a <i>deep</i> copy of <tt>this</tt>.
     */
    public Points copy () {
	Points np = new Points();
	np.pointset = PointMultiSet.convert(this.pointset.copy());
	np.bbox = this.bbox;
	np.bboxDefined = this.bboxDefined;
	
	return np;
    }//end method copy

    
    /**
     * Prints the set of points to the standard output.
     */
    public void print() {
	this.pointset.print();
    }//end method print


    /**
     * Compares two Points objects.
     * Returns 0, if both objects are equal.<br>
     * Returns 1, if <tt>this</tt> is greater than <tt>p</tt>.<br>
     * Returns -1 otherwise.<p>
     * The points of the Points object are sorted and then compared in parallel. The Points object with the first point that is 
     * greater than the appropriate point of the other object is greater etc.
     *
     * @param p the Points object to compare with
     */
    public int compare (Points p) {
	Iterator it1 = this.pointset.iterator();
	Iterator it2 = p.pointset.iterator();
	Point actPoint1,actPoint2;    
	int res;
	while (it1.hasNext() && it2.hasNext()) {
	    actPoint1 = (Point)((MultiSetEntry)it1.next()).value;
	    actPoint2 = (Point)((MultiSetEntry)it2.next()).value;
	    res = actPoint1.compare(actPoint2);
	    if (res != 0) return res;
	}//while
	
	//both sets are equal
	if (!it1.hasNext() && !it2.hasNext()) return 0;
	
	//p still has elements
	if (!it1.hasNext()) return -1;

	return 1;
    }//end method compare

}//end class Points
    
