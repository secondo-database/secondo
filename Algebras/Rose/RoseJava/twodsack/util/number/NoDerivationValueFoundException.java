/*
 * NoDerivationValueFoundException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

/**
 * A NoDerivationValueException is thrown when a class implementing the abstract {@link Rational} class doesn't has a field <i>deriv</i> which
 * is the derivation value.
 */
public class NoDerivationValueFoundException extends RuntimeException {
    /**
     * The 'empty' constructor.
     */
    public NoDerivationValueFoundException() { super(); }


    /**
     * Constructs a new instance with an error message.
     *
     * @param s the error message
     */
    public NoDerivationValueFoundException(String s) { super(s); }

}//end Exception NoDerivationValueFoundException
