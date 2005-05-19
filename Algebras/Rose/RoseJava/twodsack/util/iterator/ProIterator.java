/*
 * Iterator.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.iterator;

import twodsack.util.collectiontype.*;


/**
 * The ProIterator interface defines a set of methods which have to be implemented by all classes which implement this interface. In contrast
 * to Sun's Iterator interface, this interface offers a method {@link #reset} that sets the iterator back to the beginnig of the set it belongs to.
 * By using this method, it is no longer necessary to construct a new iterator everytime the iterator shall be reset.<p>
 * Note: The iterator implementing the ProIterator interface iterates over a set of Entry types.
 */
public interface ProIterator {
    /**
     * Returns true, if the set has at least one more element.
     */
    public boolean hasNext();


    /**
     * Returns the next object of the set.
     */
    public Object next();


    /**
     * Returns the next entry of the set.
     */
    public Entry nextEntry();
    

    /**
     * Returns true, if the set has at least one more element at the position before the actual element.
     */
    public boolean hasPrevious();


    /**
     * Returns the object before the actual element.
     */
    public Object previous();


    /**
     * Returns the next index as an int.
     */
    public int nextIndex();


    /**
     * Returns the previous index as an int.
     */
    public int previousIndex();


    /**
     * Removes the last element returned from the set.
     */
    public void remove();


    /**
     * Sets the value of the last returned object to o.
     *
     * @param o the object that shall be added
     */
    public void set(Object o);


    /**
     * Adds the object o before the object that was returned before.
     *
     * @param o the object that shall be added
     */
    public void addBefore(Object o);


    /**
     * Adds the object o after the object that was returned before.
     *
     * @param o the object that shall be added
     */
    public void add(Object o);


    /**
     * Sets the iterator to the beginning of the set.
     */
    public void reset();

}//end interface ProIterator
