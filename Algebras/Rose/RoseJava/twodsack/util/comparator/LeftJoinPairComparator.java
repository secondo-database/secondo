/*
 * LeftJoinPairComparator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.*;
import twodsack.setelement.datatype.*;
import twodsack.util.collectiontype.*;

import java.util.*;

/**
 * Implemented in this class is one single compare() method. It compares the objects stored in two objects of type MultiSetEntry. These 
 * objects must be of type LeftJoinPair. Then, for these LeftJoinPair objects, the method LeftJoinPair.compare(). is called.
 */
public class LeftJoinPairComparator implements Comparator {
    /*
     * methods
     */
    /**
     * Compares the objects stored in ino1, ino2 and returns one of {0, 1, -1}.
     * Returns 0, if both objects are equal.<p>
     * Returns -1, if ino1.o is smaller than ino2.o.<p>
     * Returns 1 otherwise
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if one of ino1.o,ino2.o is not of type LeftJoinPair
     */
    public int compare (Object ino1, Object ino2) {
	if ((((MultiSetEntry)ino1).value instanceof LeftJoinPair) &&
	    (((MultiSetEntry)ino2).value instanceof LeftJoinPair)) {
	    LeftJoinPair ljp1 = (LeftJoinPair)((MultiSetEntry)ino1).value;
	    LeftJoinPair ljp2 = (LeftJoinPair)((MultiSetEntry)ino2).value;
	    return ljp1.element.compare(ljp2.element);
	} else {
	    throw new WrongTypeException("Exception in LeftJoinPairComparator: Expected LeftJoinPair - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
	}//else
    }//end method compare

}//end class LeftJoinPairComparator
