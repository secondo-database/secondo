/*
 * NoProperCyclesException.java 2006-01-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.compositetype;

/**
 * This exception is thrown, if the passed set of segments doesn't form a proper cycles.
 * The execution of a triangulation for example is stopped, then.
 */
public class NoProperCyclesException extends RuntimeException {
    /**
     * Constructs a new instance of NoProperCyclesException.
     */
    public NoProperCyclesException() { super(); }
    
    
    /**
     * Constructs a new instance of NoProperCyclesExcepton using the argument supplied.
     *
     * @param s the string with the description of this exception.
     */
    public NoProperCyclesException(String s) { super(s); }
}//end class NoProperCyclesException
