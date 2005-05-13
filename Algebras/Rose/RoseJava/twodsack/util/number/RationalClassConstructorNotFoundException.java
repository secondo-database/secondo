/*
 * RationalClassConstructorNotFoundException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;


/**
 * This exception is thrown when the RationalFactory class tries to use a certain constructor for Rational types which doesn't exist in that class.
 */
public class RationalClassConstructorNotFoundException extends RuntimeException {
    /**
     * Constructs an 'empty' exception.
     */
    RationalClassConstructorNotFoundException() { super(); }


    /**
     * Constructs an exception with an error message.
     *
     * @param s the error message
     */
    RationalClassConstructorNotFoundException(String s) { super(s); }

}//end Exception RationalClassConstructorNotFoundException
