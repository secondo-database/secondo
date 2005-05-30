/*
 * WrongDigitValueException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;


/**
 * The abstract {@link Rational} class defines a <tt>round()</tt> method.
 * As a parameter for this method, a number must be specified which defines the number
 * of digits. Since this number must lie in a certain range, a <tt>WrongDigitValueException</tt> is thrown if it does not.
 */
public class WrongDigitValueException extends RuntimeException {

    /**
     * Constructs an 'empty' exception.
     */
    public WrongDigitValueException() { super(); }


    /**
     * Constructs an exception with an error message.
     *
     * @param s the error message
     */
    public WrongDigitValueException(String s) { super(s); }

}//end Exception WrongDigitValueException
