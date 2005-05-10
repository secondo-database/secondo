/*
 * DivisionByZeroException.java 2004-11-05
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

/**
 * This exception is thrown, when a division by zero occurs.
 * Usually, this should be avoided by the implementor of the method.
 */
public class DivisionByZeroException extends RuntimeException {
    /**
     * Constructs a new instance of DivisionByZeroException.
     * All fields default to null.
     */
    public DivisionByZeroException() { super(); }


    /**
     * Constructs a new instance of DivisionByZeroException using the argument supplied.
     * 
     * @param s the string with description about this exception.
     */
    public DivisionByZeroException(String s) { super(s); }
}//end Exception DivisionByZeroException
