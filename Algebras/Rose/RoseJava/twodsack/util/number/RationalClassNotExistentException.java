/*
 * RationalClassNotExistentException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;


/**
 * An instance of this exception is thrown when a class extending the {@link Rational} class is defined but doesn't exist.
 */
public class RationalClassNotExistentException extends RuntimeException {
    /**
     * Constructs an 'empty' exception.
     */
    public RationalClassNotExistentException() { super(); }


    /**
     * Constructs an exception with an error message.
     *
     * @param s the error message
     */
    public RationalClassNotExistentException(String s) { super(s); }

}//end Exception RationalClassNotExistentException
