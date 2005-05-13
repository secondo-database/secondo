/*
 * NoEqualVertexFoundException.java 2005-05-12
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.graph;

/**
 * An instance of this class is thrown, when an equal vertex was searched but not found.
 */
public class NoEqualVertexFoundException extends RuntimeException {

    /**
     * The empty constructor.
     */
    public NoEqualVertexFoundException() { super(); }


    /**
     * An error mesage is passed using this constructor.
     *
     * @param s the error message
     */
    public NoEqualVertexFoundException(String s) { super(s); }

}//end Exception
