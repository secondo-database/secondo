/*
 * Element.java 2005-05-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement;

import twodsack.util.*;
import twodsack.util.number.*;
import twodsack.setelement.datatype.*;
import twodsack.setelement.datatype.basicdatatype.*;
import java.io.*;

/**
 * The Element class is an abstract class which has to be extended by all geometrical object classes that shall be used in
 * {@link ElemMultiSets}. In particular, {@link Point}, {@link Segment} and {@link Triangle} extend this class. For these
 * classes, special extensions of ElemMultiSet exist. They are {@link PointMultiSet}, {@link SegMultiSet} and 
 * {@link TriMultiSet}. Objects of type Element and ElemMultiSet are the central parameters of all generic set operations
 * in the 2D-SACK package, especially in the {@link SetOps} class.<p>
 * All the methods declared here are used in the generic set operations.
 */
abstract public class Element implements Serializable, ComparableMSE{
    /**
     * methods
     */  

    /**
     * Returns a <i>deep</i> copy of the element.
     * That means, that the copy is completely independent from the original object and no change in the copy
     * affects the original.
     *
     * @return the copy
     */
    public abstract Element copy();

    
    /**
     * Returns true, if both objects are equal.
     *
     * @param inObject the object to compare with
     * @return true, if both objects are equal
     * @throws WrongTypeException if objects are not of the same type
     */
    public abstract boolean equal(Element inObject) throws WrongTypeException;


    /**
     * Returns one of {0, 1, -1} depending on comparing some specified point of the object.
     * 0 is returned, if both objects are equal<p>
     * -1 is returned, if inObject is greater than <i>this</i><p>
     * 1 otherwise
     *
     * @param inObject the object to compare with
     * @return {0, 1, -1} as int
     * @throws WrongTypeException if objects are not of the same type
     */    
    public abstract int compare(ComparableMSE inObject) throws WrongTypeException;

    
    /**
     * Returns one of {0, 1, -1} depending on comparing the x-coordinate of some specified point of the object
     * 0 is returned, if both coordinates are equal<p>
     * -1 is returned, if the x-coordinate of inObject is greater than the coordinate of <i>this</i><p>
     * 1 otherwise
     *
     * @param inObject the object to compare with
     * @return {0, 1, -1} as byte
     * @throws WrongTypeException if objects are not of the same type
     */
    public abstract byte compX(Element inObject) throws WrongTypeException;
    

    /**
     * Returns one of {0, 1, -1} depending on comparing the y-coordinate of some specified point of the object
     * 0 is returned, if both coordinates are equal<p>
     * -1 is returnd, if the y-coordinate of inObject is greater than the coordinate of <i>this</i><p>
     * 1 otherwise
     *
     * @param the object to compare with
     * @return {0, 1, -1} as byte
     * @throws WrongTypeException if objects are not of the same type
     */
    public abstract byte compY(Element inObject) throws WrongTypeException;


    /**
     * Prints the object's data to the standard output.
     */    
    public abstract void print();


    /**
     * Returns true, if both objects have common points.
     *
     * @param inObject 
     * @return true, if the objects have at least one common point
     * @throws WrongTypeException if objects are not of the same type
     */    
    public abstract boolean intersects(Element inObject) throws WrongTypeException;
    

    /**
     * Returns the minimal bounding box of the object
     *
     * @return the bounding box
     */
    public abstract Rect rect();


    /**
     * Return the Euclidean distance of both objects.
     * If this.intersects(inObject) == true, the distance is 0.
     *
     * @param inObject the object which has a 'distance' to <i>this</i>
     * @return the Euclidean distance as {@link Rational}
     * @throws WrongTypeException if objects are not of the same type
     */    
    public abstract Rational dist(Element inObject) throws WrongTypeException;


    /**
     * Returns the hashcode for the object
     * This is needed to store objects of type Element in hashtables
     *
     * @return the hashcode as int
     */
    public abstract int hashCode();


    /**
     * Returns true if both objects are equal.
     * This is pretty much the same method as {@link #equal}, but must be implemented to be able to get back objects
     * from a hashtable.
     *
     * @param o the object to compare with
     * @return true, if both objects are equal
     */
    public abstract boolean equals(Object o);
    
}//end class Element
