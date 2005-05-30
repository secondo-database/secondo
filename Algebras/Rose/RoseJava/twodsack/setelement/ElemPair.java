/*
 * ElemPair.java 2004-11-18
 *
 * Dirk Ansorge FernUniversitaet Hagen
 *
 */

package twodsack.setelement;

import twodsack.setelement.datatype.*;
import java.io.Serializable;

/**
 * This class implements a pair of objects which must implement the interface {@link Element}. These objects
 * are stored in fields {@link #first} and {@link #second}. Provided in this class are methods {@link #equal(ElemPair)}
 * and {@link #equalOrInvertedEqual(ElemPair)}, which can be used to check for equality. Note, that both objects
 * stored in the fields must be of the same type (e.g. Segment) when using methods like {@link #equal(ElemPair)} or
 * {@link #compX(ElemPair)} etc.
 */
public class ElemPair implements Serializable {
    /*
     * fields
     */
    /**
     * The first element of the pair.
     */
    public Element first;


    /**
     * The second element of the pair.
     */
    public Element second;
    

    /*
     * constructors
     */

    /**
     * Standard constructor which fills in default values.
     */
    public ElemPair(){
	first = null;
	second = null;
    }
    

    /**
     * Sets {@link #first} and {@link #second} to the passed values. No elements are copied, just pointers are set.
     *
     * @param e1 the first element
     * @param e2 the second element
     */
    public ElemPair(Element e1, Element e2) {
	first = e1;
	second = e2;
    }


    /*
     * methods
     */
    /**
     * Makes a 'deep' copy of <code>ElemPair</code>, i.e. copies all fields.
     *
     * @return the copy of <code>this</code>
     */
    public ElemPair copy(){
	ElemPair copy = new ElemPair();
	copy.first = (Element)first.copy();
	copy.second = (Element)second.copy();
	return copy;
    }//end method copy
    

    /**
     * Checks for equality of <code>inEl</code> and <code>this</code>.
     * Returns <code>true</code> for (a,b) and (a,b) but <b>not</b> for (a,b) and (b,a).
     *
     * @param inEl the object to compare with
     * @return <code>true</code> if equal, <code>false</code> otherwise
     * @see #equalOrInvertedEqual(ElemPair)
     */
    public boolean equal(ElemPair inEl) {
	if (this.first.getClass() != inEl.first.getClass() ||
	    this.second.getClass() != inEl.second.getClass()) {
	    return false;
	}//if
	else {
	    if (this.first.equal(inEl.first) &&
		this.second.equal(inEl.second)) {
		return true;
	    }//if
	}//else
	return false;
    }//end method equal
    

    /**
     * Checks for equality of <code>inEl</code> and <code>this</code>.
     * Returns <code>true</code> for (a,b) and (a,b) <b>and</b> (a,b) and (b,a).
     *
     * @param inEl the object to compare with
     * @return <code>true</code> if equal, <code>false</code> otherwise
     * @see #equal(ElemPair)
     */
    public boolean equalOrInvertedEqual (ElemPair inEl) {
	if (this.equal(inEl)) {
	    return true; }
	else {
	    if (this.first.getClass() != inEl.second.getClass() ||
		this.second.getClass() != inEl.first.getClass()) {
		return false; 
	    }//if
	    else {
		if (this.first.equal(inEl.second) &&
		    this.second.equal(inEl.first)) {
		    return true;
		}//if
	    }//else
	}//else
	return false;
    }//end method equalOrInvertedEqual

    
    /**
     * Prints {@link #first} and {@link #second} to standard output.
     *
     */
    public void print () {
	System.out.println("first:");
	first.print();
	System.out.println("second:");
	second.print();
    }//end method print    
    
    
    /**
     * Returns a value {-1,0,1} depending on the mutual position of two <code>ElemPair</code> instances.
     * Note, that the stored objectsin <code>ElemPair</code> must be of the same type, otherwise
     * a {@link WrongTypeException} is thrown. Both objects are compared by their x-coordinates in this 
     * method.
     *
     * @param inPair the object to compare with
     * @return one of {-1,0,1} depending on the mutual position of both objects
     * @see #compY(ElemPair)
     * @see #compare(ElemPair)
     */
    public byte compX (ElemPair inPair) throws WrongTypeException {
	byte res = this.first.compX(inPair.first);
	if (res == 0) {
	    res = this.second.compX(inPair.second); }
	return res;
    }//end method compX
    

    /**
     * Returns a value {-1,0,1} depending on the mutual position of two <code>ElemPair</code> instances.
     * Note, that the stored objects in <code>ElemPair</code> must be of the same type, otherwise a
     * {@link WrongTypeException} is thrown. Both objects are compared by their y-coordinates in this 
     * method.
     *
     * @param inPair the object to compare with
     * @return one of {-1,0,1} depending on the mutual position of both objects
     * @see #compX(ElemPair)
     * @see #compare(ElemPair)
     */
    public byte compY (ElemPair inPair) throws WrongTypeException {
	byte res = this.first.compY(inPair.first);
	if (res == 0) {
	    res = this.second.compY(inPair.second); }
	return res;
    }//end method compY
    

    /**
     * Returns a value {-1,0,1} depending on the mutual position of two <code>ElemPair</code> instances.
     * Note, that the stored objectsin <code>ElemPair</code> must be of the same type, otherwise
     * {@link WrongTypeException} is thrown. Both objects are compared by their x- and y-coordinates in this 
     * method.
     *
     * @param inPair the object to compare with
     * @return one of {-1,0,1} depending on the mutual position of both objects
     * @see #compX(ElemPair)
     * @see #compY(ElemPair)
     */
    public int compare (ElemPair inPair) throws WrongTypeException {
	int res = this.first.compare(inPair.first);
	if (res == 0) res = this.second.compare(inPair.second);
	return res;
    }//end method compare


    /**
     * Swaps the elements in both fields.
     * If (a,b) is twisted, (b,a) is the result.
     */
    public void twist () {
	Element swap = this.first;
	this.first = this.second;
	this.second = swap;
    }//end method twist

}//end class ElemPair
