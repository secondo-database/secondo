/*
 * RationalClassUndefinedException.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;


/**
 * An instance of this exception is thrown, when methods on {@link Rational}s are called but the underlying class extending the Rational class was not
 * defined yet.
 */
public class RationalClassUndefinedException extends RuntimeException {
    /**
     * Constructs an 'empty' exception.
     */
    public RationalClassUndefinedException() { super(); }


    /**
     * Constructs an exception with an error message.
     *
     * @param s the error message
     */
    public RationalClassUndefinedException(String s) { super(s); }

}//end Exception RationalClassUndefinedException
