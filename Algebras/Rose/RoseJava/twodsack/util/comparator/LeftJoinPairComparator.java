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
 * Implemented in this class is one single {@link #compare(Object,Object)} method. It compares the objects stored in two objects of type
 * {@link twodsack.util.collectiontype.MultiSetEntry}. These objects must be of type {@link twodsack.setelement.LeftJoinPair}.
 * Then, for these LeftJoinPair objects, the method <tt>LeftJoinPair.compare()</tt>. is called.
 */
public class LeftJoinPairComparator implements Comparator {
    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public LeftJoinPairComparator(){}

    /*
     * methods
     */
    /**
     * Compares the objects stored in <tt>ino1</tt>, <tt>ino2</tt> and returns one of {0, 1, -1}.<p>
     * Returns 0, if both objects are equal.<p>
     * Returns -1, if <tt>ino1.o</tt> is smaller than <tt>ino2.o</tt>.<p>
     * Returns 1 otherwise
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if one of <tt>ino1.o</tt>,<tt>ino2.o</tt> is not of type LeftJoinPair
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
