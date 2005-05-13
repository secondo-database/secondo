/*
 * SegmentComparator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;
import java.io.*;


/**
 * A SegmentComparator is used wherever objects of type Segment shall be inserted in a sorted set working with objects of type MultiSetEntry.
 * Then, in the constructor an instance of this class is needed. It sorts the Segment objects by calling the Segment.compare() method.
 */
public class SegmentComparator implements Comparator,Serializable {
    /*
     * methods
     */
    /**
     * Compares both objects and returns one of  {0, 1, -1}.
     * Calls the compare() method for two Segment values stored in MultiSetEntry types.
     * Returns 0, if both segments are equal.<p>
     * Returns -1, if ino1.o. is smaller than ino2.o.<p>
     * Returns 1 otherwise.
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as int
     * @throws WrongTypeException if one of ino1.o,ino2.o is not of type Segment
     */
    public int compare(Object ino1, Object ino2) {
	if ((((MultiSetEntry)ino1).value instanceof Segment) &&
	    (((MultiSetEntry)ino2).value instanceof Segment))
	    return ((Segment)((MultiSetEntry)ino1).value).compare((Segment)((MultiSetEntry)ino2).value);
	else
	    throw new WrongTypeException("in SegmentComparator: Expected Segment/Segment - found: "+
					 ((MultiSetEntry)ino1).value.getClass()+
					 "/"+
					 ((MultiSetEntry)ino2).value.getClass());
	
    }//end method compare
    
}//end class SegmentComparator
