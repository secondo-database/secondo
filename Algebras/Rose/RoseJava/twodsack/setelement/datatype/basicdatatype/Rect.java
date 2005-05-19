/*
 * Rect.java 2005-05-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.basicdatatype;

import twodsack.setelement.*;
import twodsack.util.number.*;
import java.io.*;


/**
 * The Rect class implements a minimal bounding box for geometrical objects. The coordinates for the box's vertices
 * are implemented as public fields. However, they should not be changed directly. Generally, a bounding box is constructed once
 * and not changed during its lifetime. If the box's object changes, the box is dismissed and computed again. Though this seems
 * to be a waste of time and space, this is not problem in practical use. The reason for this is, that usually bounding boxes
 * are computed for 'original' data which is only rarely changed.
 */
public class Rect implements Serializable {
    /*
     * fields
     */
    public Rational ulx;
    public Rational uly;
    public Rational urx;
    public Rational ury;
    public Rational llx;
    public Rational lly;
    public Rational lrx;
    public Rational lry;
   
    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     */
    public Rect() {}
    

    /**
     * Constructs a new bounding box from the coordinate of the upper left and lower right point.
     *
     * @param x1 x coordinate of the upper left point
     * @param y1 y coordinate of the upper left point
     * @param x2 x coordinate of the lower right point
     * @param y2 y coordinate of the lower right point
     */
    public Rect(Rational x1, Rational y1, Rational x2, Rational y2) {
	this.ulx = x1;
	this.uly = y1;
	this.urx = x2;
	this.ury = y1;
	this.llx = x1;
	this.lly = y2;
	this.lrx = x2;
	this.lry = y2;
    }
	

    /**
     * Prints the data of the box to standard output.
     */
    public void print() {
	System.out.print("Rect: ");
	System.out.print(" ul: ("+ulx.toString()+", "+uly.toString()+")");
	System.out.print(" ur: ("+urx.toString()+", "+ury.toString()+")");
	System.out.print(" ll: ("+llx.toString()+", "+lly.toString()+")");
	System.out.println(" lr: ("+lrx.toString()+", "+lry.toString()+")");
    }//end method print


    /**
     * Returns true, if the passed point is covered by the box.
     *
     * @param inPoint the point
     * @return true, if the point is covered
     */
    public boolean covers (Point inPoint) {
	if (inPoint.x.greaterOrEqual(ulx) && 
	    inPoint.x.lessOrEqual(urx) &&
	    inPoint.y.greaterOrEqual(lly) &&
	    inPoint.y.lessOrEqual(uly)) {
	    return true;
	}//if
	else return false;
    }//end method covers


    /**
     * Converts the box data to a string.
     * Useful for pretty-printing.
     *
     * @return the data converted to a string
     */
    public String toString() {
	return new String("Rect [ul("+ulx+","+uly+") ll("+llx+","+lly+") ur("+urx+","+ury+") lr("+lrx+","+lry+")]");
    }//end method toString


    /**
     * Return true, if both boxes have common points.
     *
     * @param inRect the second box
     * @return true, if both boxes have at least one common point
     */
    public boolean hasCommonPoints(Rect inRect) {
	boolean xCommon = false;
	boolean yCommon = false;
	if (this.ulx.equal(inRect.ulx) || this.urx.equal(inRect.urx) ||
	    this.ulx.equal(inRect.urx) || this.urx.equal(inRect.ulx) ||
	    (this.ulx.less(inRect.ulx) && this.urx.greater(inRect.ulx)) ||
	    (this.ulx.less(inRect.urx) && this.urx.greater(inRect.urx)) ||
	    (inRect.ulx.less(this.ulx) && inRect.urx.greater(this.ulx)) ||
	    (inRect.ulx.less(this.urx) && inRect.urx.greater(this.urx))) xCommon = true;
	
	if (this.uly.equal(inRect.uly) || this.lly.equal(inRect.lly) ||
	    this.uly.equal(inRect.lly) || this.lly.equal(inRect.uly) ||
	    (this.uly.greater(inRect.uly) && this.lly.less(inRect.uly)) ||
	    (this.uly.greater(inRect.lly) && this.lly.less(inRect.lly)) ||
	    (inRect.uly.greater(this.uly) && inRect.lly.less(this.uly)) ||
	    (inRect.uly.greater(this.lly) && inRect.lly.less(this.lly))) yCommon = true;

	if (xCommon && yCommon) return true;
	else return false;
    }//end method hasCommonPoints
	    

    /**
     * Returns the intersection of two boxes.
     * 
     * @param inRect the second box
     * @return the intersection box
     */
    public Rect intersection (Rect inRect) {
	Rational ulxN = null;
	Rational ulyN = null;
	Rational lrxN = null;
	Rational lryN = null;
	if (this.ulx.lessOrEqual(inRect.ulx)) ulxN = inRect.ulx;
	else ulxN = this.ulx;
	if (this.uly.lessOrEqual(inRect.uly)) ulyN = this.uly;
	else ulyN = inRect.uly;
	if (this.lrx.lessOrEqual(inRect.lrx)) lrxN = this.lrx;
	else lrxN = inRect.lrx;
	if (this.lry.lessOrEqual(inRect.lry)) lryN = inRect.lry;
	else lryN = this.lry;
	
	return new Rect(ulxN,ulyN,lrxN,lryN);
    }//end method intersection

}//end class Rect
