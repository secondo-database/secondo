/*
 * RationalClassConstructorNotFoundException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;


/**
 * This exception is thrown when the {@link RationalFactory} class tries to use a certain constructor for {@link Rational} types which doesn't exist in that class.
 */
public class RationalClassConstructorNotFoundException extends RuntimeException {
    /**
     * Constructs an 'empty' exception.
     */
    public RationalClassConstructorNotFoundException() { super(); }


    /**
     * Constructs an exception with an error message.
     *
     * @param s the error message
     */
    public RationalClassConstructorNotFoundException(String s) { super(s); }

}//end Exception RationalClassConstructorNotFoundException
