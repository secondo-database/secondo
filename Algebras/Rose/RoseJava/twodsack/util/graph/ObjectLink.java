/*
 * ObjectLink.java 2004-11-14
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.graph;

/**
 * This class implements a pointer to an <code>Object</code> and the ability to give that object a number.
 * It is needed as a support structure for {@link MeshGenerator} and {@link Graph}. There, it is used in 
 * hash tables. For this reason, it implements the methods <code>hashCode</code> and <code>equals</code>.
 */
class ObjectLink {

    /*
     * fields
     */
    protected Object linkedObject;
    protected int number;

    /*
     * constructors
     */
    /**
     * The default constructor. Fills in default values.
     */
    protected ObjectLink() {
	this.linkedObject = null;
	this.number = -1;
    }

    /**
     * Constructs a <code>ObjectLink</code> with passed values.
     *
     * @param object the object to be linked to
     */
    protected ObjectLink(Object object) {
	this.linkedObject = object;
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
    protected void setNumber(int num) {
	this.number = num;
    }//end method setNumber

}//end class ObjectLink
