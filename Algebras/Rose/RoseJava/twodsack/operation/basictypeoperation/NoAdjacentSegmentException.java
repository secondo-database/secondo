/*
 * NoAdjacentSegmentException.java 2005-04-28
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.basictypeoperation;

/**
 * This exception is used for reporting an error when an operation shall be executed on two
 * adjacent segments that actually are not adjacent.
 */

public class NoAdjacentSegmentException extends RuntimeException {
    /**
     * The 'empty' constructor for the exception.
     */
    public NoAdjacentSegmentException() { super(); }


    /**
     * Constructs an exception with an error description as parameter.
     *
     * @param s the error description
     */
    public NoAdjacentSegmentException(String s) { super(s); }
}//end class NoAdjacentSegmentException
