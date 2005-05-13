/*
 * PSPointComparator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;


/**
 * A PSPointComparator is used wherever objects of type PSPoint shall be inserted in a sorted set working with objects of type MultiSetEntry.
 * Then, in the constructor an instance of this constructor is needed. It sorts the PSPoint objects by calling the PSPoint.compare() method.
 */
public class PSPointComparator implements Comparator {
    /**
     * Compares both objects and returns one of {0, 1, -1}.
     * Calls the compare() method for two PSPoint values stored in MultiSetentry types.
     * Returns 0, if both points are equal.<p>
     * Returns -1, if ino1.o is smaller than ino2.o.<p>
     * Returns 1 otherwise.
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if one of ino1.o, ono2.o is not of type PSPoint
     */
    public int compare(Object ino1, Object ino2) throws WrongTypeException {
	if ((((MultiSetEntry)ino1).value instanceof PSPoint) &&
	    (((MultiSetEntry)ino2).value instanceof PSPoint))
	    return ((PSPoint)((MultiSetEntry)ino1).value).compare((PSPoint)((MultiSetEntry)ino2).value);
	else 
	    throw new WrongTypeException("in PSPointComparator: Expected PSPoint - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
    }//end method compare

}//end class PSPointComparator
