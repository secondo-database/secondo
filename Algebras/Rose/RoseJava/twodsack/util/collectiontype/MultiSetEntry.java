/*
 * MultiSetEntry.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collectiontype;

import twodsack.setelement.datatype.*;
import twodsack.util.*;
import java.io.*;


/**
 * MultiSetEntry instances are the elements that are stored in {@link twodsack.util.collection.MultiSet}s.
 * They provide two fields: A <tt>value</tt> to store an object, and a <tt>number</tt> to
 * specify how many times the object occurs.
 */
public class MultiSetEntry implements Comparable,Serializable {
    /*
     * fields
     */
    /**
     * The object that is wrapped in a MultiSetEntry.
     */
    public Object value; //object to store

    /**
     * The number of same object values.
     */
    public int number; //number of same objects

    
    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     * Sets <tt>value = null</tt> and <tt>number = -1</tt>.
     */
    public MultiSetEntry () {
	this.value = null;
	this.number = -1;
    }


    /**
     * Constructs a MultiSetEntry from <tt>o</tt>.
     * Sets <tt>this.number</tt> to <tt>number</tt>.
     * 
     * @param o the new object
     * @param number number of <tt>o</tt>
     */
    public MultiSetEntry (Object o, int number) {
	this.value = o;
	this.number = number;
    }

    
    /**
     * Compares the objects of to instances of MultiSetEntry and returns one of {0, 1, -1}.
     * Returns 0, if both objects are equal.<p>
     * Returns -1, if the <tt>this.o</tt> is smaller than <tt>inMSE.o</tt>.<p>
     * Returns 1 otherwise.<p>
     * The <tt>compare()</tt> method of type <tt>o</tt> is used for the comparison.
     *
     * @param inMSE <tt>this.o</tt> is compared to <tt>inMSE.o</tt>
     * @return one of  {0, 1, -1} as int
     * @throws WrongTypeException if <tt>this.o</tt> and <tt>inMSE.o</tt> are not of the same type
     */
    public int compareTo(Object inMSE) {
	if (!(inMSE instanceof MultiSetEntry)) throw new WrongTypeException("Expected "+this.getClass()+", found "+inMSE.getClass());
	MultiSetEntry mse = (MultiSetEntry)inMSE;
	if (mse.value instanceof ComparableMSE) {
	    return ((ComparableMSE)this.value).compare((ComparableMSE)(mse.value));
	}
	else throw new WrongTypeException("Expected class ComparableMSE, found "+mse.value.getClass());
    }//end method compareTo

}//end class MultiSetEntry
