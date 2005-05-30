/*
 * NoOverlapException.java 2005-04-28
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.operation.basictypeoperation;

/**
 * This exception is used for reporting an exception, when two objects should overlap, but do not.
 */
public class NoOverlapException extends RuntimeException {
    /**
     * The 'empty' constructor.
     */
    public NoOverlapException() { super(); }


    /**
     * Constructs an exception with an error message.
     *
     * @param s the error message
     */
    public NoOverlapException(String s) { super(s); }
}//end class NoOverlapException
