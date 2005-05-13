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
 * MultiSetEntry instances are the elements that are stored in MultiSet(s). They provide two fields: A value to store an object, and a number to
 * specify how many times the object occurs.
 */
public class MultiSetEntry implements Comparable,Serializable {
    /*
     * fields
     */
    public Object value; //object to store
    public int number; //number of same objects

    
    /*
     * constructors
     */
    /**
     * The 'empty' constructor.
     * Sets value = null and number = -1.
     */
    public MultiSetEntry () {
	this.value = null;
	this.number = -1;
    }


    /**
     * Constructs a MultiSetEntry from o.
     * Sets this.number to number.
     * 
     * @param o the new object
     * @param number number of o
     */
    public MultiSetEntry (Object o, int number) {
	this.value = o;
	this.number = number;
    }

    
    /**
     * Compares the objects of to instances of MultiSetEntry and returns one of {0, 1, -1}.
     * Returns 0, if both objects are equal.<p>
     * Returns -1, if the this.o is smaller than inMSE.o.<p>
     * Returns 1 otherwise.<p>
     * The compare() method of type o is used for the comparison.
     *
     * @param inMSE this.o is compared to inMSE.o
     * @return one of  {0, 1, -1} as int
     * @throws WrongTypeException if this.o and inMSE.o are not of the same type
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
