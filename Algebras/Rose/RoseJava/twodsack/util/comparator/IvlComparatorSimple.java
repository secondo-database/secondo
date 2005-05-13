/*
 * IvlComparatorSimple.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

/**
 * This class provides one single method: The compare method for two objects of type Interval. It can be used for all collections which
 * have objects of type Interval as elements.
 */
public class IvlComparatorSimple implements Comparator {
    /*
     * methods
     */
    /**
     * Compares both intervals and returns one of {0, 1, -1}.
     * This comparator compares the left and then right border of the interval and returns:<p>
     * 0, if both interval borders are equal
     * -1, if ino1 is smaller
     * 1 otherwise
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if ino1 or ino2 is not of type Interval
     */
    public int compare (Object ino1, Object ino2) throws WrongTypeException {
	if ((((MultiSetEntry)ino1).value instanceof Interval) &&
	    (((MultiSetEntry)ino2).value instanceof Interval)) {
	    Interval i1 = (Interval)(((MultiSetEntry)ino1).value);
	    Interval i2 = (Interval)(((MultiSetEntry)ino2).value);

	    return i1.comp(i2);
	}//if
	else throw new WrongTypeException("Exception in IvlComparatorSimple: Expected type Interval - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
    }//end method compare

}//end class IvlComparatorSimple
