/*
 * ComparatorNotDefinedException.java 2005-05-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.collection;

/**
 * A ComparatorNotDefinedException is thrown, when a method for a collection is called that needs a comparator. One example is the 
 * <code>remove()</code> method for {@link ProLinkedList}. All of the other operations of that class work without the comparator.
 * However, if remove shall be used, the class instance has to be constructed using a comparator.
 */
public class ComparatorNotDefinedException extends RuntimeException {

    /**
     * The 'empty' constructor.
     */
    public ComparatorNotDefinedException() { super(); }


    /**
     * A constructor that stores an error message.
     * 
     * @param s the error message
     */
    public ComparatorNotDefinedException(String s) { super(s); }
}//end class ComparatorNotDefinedException
