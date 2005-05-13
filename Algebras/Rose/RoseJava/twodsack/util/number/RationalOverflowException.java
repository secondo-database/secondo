/*
 * RationalOverflowException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;


/**
 * Depending on the implementation of a Rational class extension, internal fields may overflow. In such cases, an instance of this class is 
 * thrown.
 */
public class RationalOverflowException extends RuntimeException {
    /**
     * Constructs an 'empty' exception.
     */
    public RationalOverflowException() { super(); }


    /**
     * Constructs an exception with an error message.
     *
     * @param s the error message
     */
    public RationalOverflowException(String s) { super(s); }

}//end Exception RationalOverflowException
