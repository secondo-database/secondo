/*
 * PointLink.java 2004-11-14
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util;

import twodsack.setelement.datatype.basicdatatype.*;

/**
 * This class implements a pointer to a <code>Point</code> and the ability to give that point a number.
 * It is needed as a support structure for {@link twodsack.util.meshgenerator.MeshGenerator}. There, it is used in hash tables. For 
 * this reason, it implements the methods <code>hashCode</code> and <code>equals</code>.
 */
public class PointLink {
    /*
     * fields
     */
    /**
     * The referenced point.
     */
    public Point linkedPoint;

    /**
     * The number of that point.
     */
    public int number;

    /*
     * constructors
     */
    /**
     * The default constructor. Fills in default values.
     */
    public  PointLink() {
	this.linkedPoint = null;
	this.number = -1;
    }

    /**
     * Constructs a <code>PointLink</code> with passed values.
     *
     * @param point the point to be linked to
     */
    public PointLink(Point point) {
	this.linkedPoint = point;
	this.number = -1;
    }

    /*
     * methods
     */
    /**
     * Sets the number of <code>this</code> to <code>num</code>.
     *
     * @param num the number
     */
    public void setNumber(int num) {
	this.number = num;
    }//end method setNumber

}//end class PointLink
