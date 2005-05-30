/*
 * TriangleComparator.java 2005-05-12
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
 * A TriangleComparator is used wherever objects of type {@link Triangle} shall be inserted in a sorted set working with objects of type
 * {@link MultiSetEntry}.
 * Then, in the constructor an instance of this class is needed. It sorts the Triangle objects by calling the <tt>Triangle.compare()</tt> method.
 */
public class TriangleComparator implements Comparator,Serializable {
    /*
     * constructors
     */
    /**
     * The standard constructor.
     */
    public TriangleComparator(){}


    /*
     * methods
     */
    /**
     * Compares both objects and returns one of {0, 1, -1}.<p>
     * Calls the <tt>compare()</tt> method for two {@link Triangle} values stored in {@link MultiSetEntry} types.<p>
     * Returns 0, if both triangles are equal.<p>
     * Returns -1, if <tt>ino1.o</tt> is smaller than <tt>ino2.o</tt>.<p>
     * Returns 1 otherwise.
     * 
     * @param ino1 the first object
     * @param ino2 the second object
     * @return one of {0, 1, -1} as <tt>int</tt>
     * @throws WrongTypeException if one of <tt>ino1.o</tt>, <tt>ino2.o</tt> is not of type <tt>Triangle</tt>
     */
    public int compare(Object ino1, Object ino2) throws WrongTypeException {
	if ((((MultiSetEntry)ino1).value instanceof Triangle) &&
	    (((MultiSetEntry)ino2).value instanceof Triangle))
	    return ((Triangle)((MultiSetEntry)ino1).value).compare((Triangle)((MultiSetEntry)ino2).value);
	else
	    throw new WrongTypeException("in TriangleComparator: Expected Triangle - found :"+((MultiSetEntry)ino1).value.getClass()+"/"+((MultiSetEntry)ino2).value.getClass());
    }//end method compare

}//end class TriangleComparator
