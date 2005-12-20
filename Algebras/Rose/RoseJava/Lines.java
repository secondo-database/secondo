import twodsack.set.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.comparator.*;
import twodsack.util.number.*;
import java.io.*;
import java.util.*;


/**
 * This class implements the Lines type. Lines instances are one of the three geometric types, which are used as parameter types in the ROSE
 * algebra. The other two types are Points and Reginons.
 */
public class Lines implements Serializable{
    /*
     * fields
     */
    /**
     * The set of line segments.
     */
    public SegMultiSet segset;
    
    /**
     * The bounding box of the Lines object.
     */
    private Rect bbox;

    /**
     * If true, a bbox was already computed and is valid.
     */
    private boolean bboxDefined = false;

    /**
     * The length of the Lines object.
     */
    private double length;

    /**
     * If true, the length was already computed and is valid.
     */
    private boolean lengthDefined = false;
    
    /*
     * constructors
     */
    /**
     * The empty constructor.
     */
    public Lines() {
	segset = new SegMultiSet(new SegmentComparator());
	bbox = null;
	length = 0.0;
	lengthDefined = false;
    }


    /**
     * Constructs a new Lines instance from a set of segments. 
     * Note, that during the construction process the segments are not checked for intersection etc. No copy is made of the segment set.
     *
     * @param sl the set of segments
     */
    public Lines(SegMultiSet sl) {
	segset = sl;
	length = ROSEAlgebra.l_length(this);
	bbox = sl.rect();
	lengthDefined = true;
    }


    /**
     * Constructs a new Lines instance from an already existing Line instance.
     * Note, that during the construction process, the segments are not copied.
     * 
     * @param l the line object
     */
    public Lines(Lines l) {
	this.segset = l.segset;
	this.length = l.length;
	this.bbox = l.bbox;
	this.lengthDefined = true;
    }


    /*
     * methods
     */
    /**
     * Returns the bounding box for the Lines object.
     */
    public Rect rect() {
	if (bboxDefined)
	    return bbox;
	else {
	    bbox = segset.rect();
	    bboxDefined = true;
	    return bbox;
	}//else
    }//end method rect


    /**
     * Constructs a new line segment from the given double coordinates and adds it to the segment set.
     *
     * @param x1 x coordinate of the startpoint
     * @param y1 y coordinate of the startpoint
     * @param x2 x coordinate of the endpoint
     * @param y2 y coordinate of the endpoint
     */
    public void add(double x1, double y1, double x2, double y2) {
	this.segset.add(new Segment(x1,y1,x2,y2));
	this.bboxDefined = false;
	this.lengthDefined = false;
    }//end method add


    /**
     * Cosntructs a new line segment from the given Rational coordinates and adds it to the segment set.
     *
     * @param x1 x coordinate of the startpoint
     * @param y1 y coordinate of the startpoint
     * @param x2 x coordinate of the endpoint
     * @param y2 y coordinate of the endpoint
     */
    public void add(Rational x1, Rational y1, Rational x2, Rational y2) {
	this.segset.add(new Segment(x1,y1,x2,y2));
	this.bboxDefined = false;
	this.lengthDefined = false;
    }//end method add


    /**
     * Constructs a new line segment form the given int coordinates and adds it to the segment set.
     *
     * @param x1 x coordinate of the startpoint
     * @param y1 y coordinate of the startpoint
     * @param x2 x coordinate of the endpoint
     * @param y2 y coordinate of the endpoint
     */
    public void add(int x1, int y1, int x2, int y2) {
	this.segset.add(new Segment(x1,y1,x2,y2));
	this.bboxDefined = false;
	this.lengthDefined = false;
    }//end method add


    /**
     * Adds the given segment to the segset.
     *
     * @param s the segment
     */
    public void add(Segment s) {
	this.segset.add(s);
	this.bboxDefined = false;
	this.lengthDefined = false;
    }//end method add
	

    /**
     * Constructs a Lines value from a byte array.
     * Given a byte array (probably from a disk access), a Lines value is constructed from it.
     * If the Lines value cannot be restored properly, <tt>null</tt> is returned.
     *
     * @param buffer the byte array
     * @param the restored Lines value
     */
    public static Lines readFrom(byte[] buffer){
	try{
	    ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(buffer));
	    Lines res = (Lines) ois.readObject();
	    ois.close();
	    return res;
	} catch(Exception e){
	    return null;
	}
    }//end method readFrom


    /**
     * Constructs a serialized Lines value.
     * From the Lines value, a byte array is constructed. Then, this array can be written to disk.
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
	    return null;
	}
    }//end method writeToByteArray
    

    /**
     * Returns a <i>deep</i> copy of <tt>this</tt>.
     *
     * @return the copy
     */    
    public Lines copy () {
	return new Lines(SegMultiSet.convert(this.segset.copy()));
    }//end method copy
    

    /**
     * Prints the set of segments to the standard output.
     */
    public void print() {
	this.segset.print();
    }//end method print
	
}//end class Lines
    
