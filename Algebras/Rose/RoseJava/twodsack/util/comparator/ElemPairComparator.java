/*
 * ElemPairComparator.java 2004-11-05
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.util.comparator;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;

import java.util.Comparator;

/**
 * An <code>ElemPairComparator</code> is a comparator that is used for the comparison of two objects of
 * type {@link twodsack.setelement.ElemPair}. This comparator is used as parameter for the construction of a 
 * {@link twodsack.util.collection.MultiSet} or for one of its subclasses.
 */
public class ElemPairComparator implements Comparator {
    /*
     * constructors
     */
    /**
     * Constructs an 'empty' instance.
     */
    public ElemPairComparator(){}


    /*
     * methods
     */

    /**
     * Compares two objects which must be of type <code>MultiSetEntry</code>.
     * The objects wrapped inside of <code>ino1</code> and <code>ino2</code> must be of type
     * {@link ElemPair}. The elements inside of those <code>ElemPair</code>s themselves must
     * be of the same type implementing the {@link Element} class.
     * <p>The return value is 0 if both pairs are equal, -1 if <code>ino1</code> is smaller than
     * <code>ino2</code> and 1 otherwise.
     * 
     * @param ino1 the first object
     * @param ino2 the second object
     * @return {0,-1,1} depending on the objects
     */
    public int compare(Object ino1, Object ino2) {
	
	if ((((MultiSetEntry)ino1).value instanceof ElemPair) &&
	    (((MultiSetEntry)ino2).value instanceof ElemPair)) 
	    
	    return ((ElemPair)(((MultiSetEntry)ino1).value)).compare((ElemPair)(((MultiSetEntry)ino2).value));
	else
	    throw new WrongTypeException("in ElemPairComparator: Expected ElemPair - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
	
    }//end method compare
    
}//end class ElemPairComparator
