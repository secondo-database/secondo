/*
 * TooManyCyclesException.java 2004-11-11
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.compositetype;

/**
 * This exception is thrown, if the number of cycles passed to a method is greater than allowed.
 * Certain methods only allow a limited number of cycles, i.e. mostly only one cycle.
 * If the number of cycles is greater, this exception is thrown.
 */
public class TooManyCyclesException extends RuntimeException {
    /**
     * Constructs a new instance of TooManyCyclesException.
     * All fields default to null.
     */
    public TooManyCyclesException() { super(); }
    

    /**
     * Constructs a new instance of TooManyCyclesException using the argument supplied.
     *
     * @param s the string with description about this exception.
     */
    public TooManyCyclesException(String s) { super(s); }
}//end Exception TooManyCyclesException
