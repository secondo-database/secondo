/*
 * EMSPointer.java 2005-11-09
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.set.*;
import twodsack.util.*;
import twodsack.util.number.*;

import java.io.*;


/**
 * The class EMSPointer implements a type which stores pairs of type <tt>ElemMultiSet</tt> and its bounding box. There is a need for
 * this type because there is no other way to use the <tt>SetOps.overlappingPairs</tt> method for sets. The <tt>overlappingPairs</tt>
 * method computes pairs of objects (which must extend the <tt>Element</tt> class) which have overlapping bounding boxes. This
 * method can be used also for sets, if they are wrapped in objects of this class.<p>
 */

public class EMSPointer extends Element implements Serializable {

    /*
     * fields
     */
    /**
     * The set that is wrapped in instances of this class.
     */
    public ElemMultiSet set;

    /**
     * The bounding box for the set.
     */
    public Rect bbox;

    /**
     * The key that is used for identification.
     *
     * Comparison and all methods of the <tt>Element</tt> class are implemented based on this integer key.
     */
    public int key;

    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     * Sets <tt>key</tt> to -1, <tt>set = null</tt> and <tt>bbox = null</tt>.
     */
    public EMSPointer() {
	key = -1;
	set = null;
	bbox = null;
    }

    /**
     * Constructs a new EMSPointer object from given parameters.
     *
     * @param set the <tt>ElemMultiSet</tt> that is wrapped
     * @param bbox the bounding box for the set
     * @param key the integer key that is used for operations 
     */
    public EMSPointer (ElemMultiSet set, Rect bbox, int key) {
	this.set = set;
	this.bbox = bbox;
	this.key = key;
    }
	

    /*
     * methods
     */
    /**
     * Returns <tt>true</tt>, if both objects are equal.
     *
     * @param o the object to compare with
     * @return true, if <i>this</i> and <tt>o</tt> are equalelse { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
     */
    public boolean equals (Object o) {
	return (((EMSPointer)o).key == this.key);
    }//end method equals
    

     /**
      * Returns the hashcode for <i>this</i>.
      * The code is computed as the sum of its coordinates.
      *
      * @return the hashcode as int
      */
    public int hashCode() {
	return this.key;
    }//end method hashCode


    /**
     * Returns the distance between two objects, which is the difference of two <tt>int</tt> values in this case.
     *
     * @param e the 'distant' object
     * @return the distance as Rational
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>EMSPointer</tt>
     */
    public Rational dist (Element e) throws WrongTypeException {
	if (e instanceof EMSPointer) {
	    EMSPointer emsp = (EMSPointer)e;
	    return RationalFactory.constRational(this.key-emsp.key).abs();
	} else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method dist


    /**
     * Return the bounding box of <tt>this</tt>.
     *
     * @return the bounding box
     */
    public Rect rect() {
	return this.bbox;
    }//end method rect


    /**
     * Computes the intersection of two <tt>EMSPointer</tt> objects.
     *
     * This method implements a typical operation for basic type elements. Since this class implements a wrapper for sets, this 
     * operation is not implemented properly (that means, it doesn't check for intersection of some set elements). It returns 
     * <tt>true</tt> if the bounding boxes intersect, instead.
     *
     * @param e the object which is checked for intersection
     * @return <tt>true</tt> if both bounding boxes intersect
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>EMSPointer</tt>
     */
    public boolean intersects (Element e) throws WrongTypeException {
	if (e instanceof EMSPointer) {
	    EMSPointer emsp = (EMSPointer)e;
	    return this.bbox.hasCommonPoints(emsp.bbox);
	} else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method intersects


    /**
     * Prints the <tt>EMSPointer</tt>'s data to the standard output.
     */
    public void print() {
	System.out.println("EMSPointer: key: "+this.key+", bbox: "+this.bbox+", set.size: "+this.set.size());
    }//end method print

    /**
     * Compares the keys of both objects and returns one of {0, 1, -1}.<p>
     * Returns 0, if the keys are equal.<p>
     * Returns 1, if <tt>this</tt> has they greater key <p>
     * Returns -1 otherwise.
     * 
     * @param e the object to compare with
     * @return {0, 1, -1} as byte
     * @throws WrongTypeException if <tt>e</tt> is not of type EMSPointer
     */
    public byte compY(Element e) throws WrongTypeException {
	if (e instanceof EMSPointer) {
	    EMSPointer emsp = (EMSPointer)e;
	    if (this.key == emsp.key) return 0;
	    if (this.key < emsp.key) return -1;
	    return 1;
	} else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compY


    /**
     * Compares the keys of both objects and returns one of {0, 1, -1}.<p>
     * Returns 0, if the keys are equal.<p>
     * Returns 1, if <tt>this</tt> has they greater key <p>
     * Returns -1 otherwise.
     * 
     * @param e the object to compare with
     * @return {0, 1, -1} as byte
     * @throws WrongTypeException if <tt>e</tt> is not of type EMSPointer
     */
    public byte compX(Element e) throws WrongTypeException {
	if (e instanceof EMSPointer) {
	    EMSPointer emsp = (EMSPointer)e;
	    if (this.key == emsp.key) return 0;
	    if (this.key < emsp.key) return -1;
	    return 1;
	} else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compX


    /**
     * Compares the keys of both objects and returns one of {0, 1, -1}.<p>
     * Returns 0, if the keys are equal.<p>
     * Returns 1, if <tt>this</tt> has they greater key <p>
     * Returns -1 otherwise.
     * 
     * @param e the object to compare with
     * @return {0, 1, -1} as int
     * @throws WrongTypeException if <tt>e</tt> is not of type EMSPointer
     */
    public int compare(ComparableMSE e) throws WrongTypeException {
	if (e instanceof EMSPointer) {
	    EMSPointer emsp = (EMSPointer)e;
	    if (this.key == emsp.key) return 0;
	    if (this.key < emsp.key) return -1;
	    return 1;
	} else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method compare
    
    
    /**
     * Returns <tt>true</tt>, if the keys of <i>this</i> and <tt>e</tt> are equal.
     * Throws a WrongTypeException if <tt>e</tt> is not of type <tt>EMSPointer.
     *
     * @param e the object to compare with
     * @return <tt>true</tt>, if both objects have the same keys
     * @throws WrongTypeException if <tt>e</tt> is not of type <tt>EMSPointer</tt>
     */
    public boolean equal(Element e) throws WrongTypeException {
	if (e instanceof EMSPointer) {
	    EMSPointer emsp = (EMSPointer)e;
	    if (this.key == emsp.key) return true;
	    return false;
	} else { throw new WrongTypeException("Expected: "+this.getClass()+" - Found: "+e.getClass()); }
    }//end method equal


    /**
     * Returns a <i>deep</i> copy of this.
     * This means, that any changes in the copy doesn't affect the original object. Note, that the bounding box is not a copy of 
     * the original box, but is a pointer on the same box instead.
     *
     * @return the copy
     */
    public Element copy(){
	EMSPointer copy = new EMSPointer();
	copy.key = this.key;
	copy.bbox = this.bbox;
	copy.set = this.set.copy();
	
	return copy;
    }//end method copy

}//end class EMSPointer
