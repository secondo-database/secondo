/*
 * ElemComparator.java 2004-11-05
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
 * This class is derived from the original {@link java.util.Comparator} class. It is still an ordinary
 * comparator, but works only for pairs of type {@link Element}. Therefore, the original
 * <code>compare</code> method is overwritten. To be precise, this comparator doesn't work on 
 * pairs of type <code>Element</code> directly, but on such types wrapped in objects of type
 * {@link MultiSetEntry}. Usually, they can be found in a {@link ElemMultiSet} structure.
 */
public class ElemComparator implements Comparator {
    /*
     * methods
     */

    /**
     * Compares two objects which must be of type <code>MultiSetEntry</code>.
     * The objects wrapped inside of <code>ino1</code> and <code>ino2</code> must be of the same
     * <i>subtype</i> of {@link Element} in this case, e.g. both of type {@link Point}. The return value
     * is 0 if both elements are equal, -1 if <code>ino1</code> is smaller than <code>ino2</code>
     * and 1 otherwise.
     *
     * @param  ino1 the first object
     * @param  ino2 the second object
     * @return {0,-1,1} depending on the objects
     */
    public int compare(Object ino1, Object ino2) {
	if ((((MultiSetEntry)ino1).value instanceof Element) &&
	    (((MultiSetEntry)ino2).value instanceof Element))
	    return ((Element)((MultiSetEntry)ino1).value).compare((Element)((MultiSetEntry)ino2).value);
	else
	    throw new WrongTypeException("in ElemComparator: Expected Element - found: "+(((MultiSetEntry)ino1).value.getClass())+"/"+(((MultiSetEntry)ino2).value.getClass()));
	
    }//end method compare
    
}//end class ElemComparator