/*
 * PointComparator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.comparator;

import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.util.collectiontype.*;
import java.util.*;
import java.io.Serializable;


/**
 * A PointComparator is used wherever objects of type Point shall be inserted in a sorted set working with objects of type MultiSetEntry.
 * Then, in the constructor an instance of this
 * constructor is needed. It sorts the Point objects by calling the Point.compare() method.
 */
public class PointComparator implements Comparator,Serializable {
    /*
     * methods
     */
    /**
     * Compares both objects and returns one of {0, 1, -1}.
     * Calls the compare() method for two Point values stored in MultiSetEntry types.
     * Returns 0, if both points are equal.<p>
     * Returns -1, if ino1.o is smaller than ino2.o.<p>
     * Returns 1 otherwise.
     *
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of  {0, 1, -1} as int
     * @throws WrongTypeException if ino1.o or ino2.o is not of type Point
     */
    public int compare(Object ino1, Object ino2) throws WrongTypeException {
	if ((((MultiSetEntry)ino1).value instanceof Point) &&
	    (((MultiSetEntry)ino2).value instanceof Point))
	    return ((Point)((MultiSetEntry)ino1).value).compare((Point)((MultiSetEntry)ino2).value);
	else 
	    throw new WrongTypeException("in PointComparator: Expected Point - found: "+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
    }//end method compare

}//end class PointComparator
