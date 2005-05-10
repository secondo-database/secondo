/*
 * InfiniteGradientException.java 2005-05-03
 *
 * Dirk Ansorge FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype.basicdatatype;


/**
 * An InfiniteGradientException is thrown, when a method finds a vertical segment and cannot handle it.
 */
public class InfiniteGradientException extends Exception {
    /**
     * The 'empty' constructor.
     */
    InfiniteGradientException() { super(); }


    /**
     * Constructs a new InfiniteGradientException with an error message
     *
     * @param s the error message
     */
    InfiniteGradientException(String s) { super(s); }

}//end Exception InfiniteGradientException
