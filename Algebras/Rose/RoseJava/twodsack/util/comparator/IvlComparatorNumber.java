/*
 * IvlComparatorNumber.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;

/**
 * The IvlComparatorNumber class implements one single method, compare(). It is used to sort objects of the type Interval by their number field.
 */
public class IvlComparatorNumber implements Comparator {
    
    /*
     * methods
     */
    /**
     * Compares two objects of type Interval and returns one of {0, 1, -1}.
     * Returns 0, if both numbers are equal.<p>
     * Returns -1, if ino1 has the smaller number.<p>
     * Returns 1 otherwise.
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if ino1 or ino2 are not of type Interval
     */
    public int compare(Object ino1, Object ino2) throws WrongTypeException {
	if ((ino1 instanceof Interval) &&
	    ino2 instanceof Interval)
	    if (((Interval)ino1).number < ((Interval)ino2).number) return -1;
	    else if (((Interval)ino1).number > ((Interval)ino2).number) return 1;
	    else return 0;
	else
	    throw new WrongTypeException("in IvlNumberComparator: Expected Interval - found: "+ino1.getClass()+"/"+ino2.getClass());
    }//end method compare
}//end class IvlNumberComparator
		
