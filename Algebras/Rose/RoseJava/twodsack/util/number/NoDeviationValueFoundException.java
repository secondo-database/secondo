/*
 * NoDeviationValueFoundException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

/**
 * A NoDeviationValueException is thrown when a class implementing the abstract {@link Rational} class doesn't have a field <tt>deviation</tt> which
 * is the deviation value.
 */
public class NoDeviationValueFoundException extends RuntimeException {
    /**
     * The 'empty' constructor.
     */
    public NoDeviationValueFoundException() { super(); }


    /**
     * Constructs a new instance with an error message.
     *
     * @param s the error message
     */
    public NoDeviationValueFoundException(String s) { super(s); }

}//end Exception NoDeviationValueFoundException
