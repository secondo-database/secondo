/*
 * NoOverlappingBoxFoundException.java 2005-05-02
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.operation.setoperation;

/**
 * A NoOverlappingboxFoundException is thrown when there is no pair of overlapping bounding boxes found for two sets of elements.
 * This exception is generally thrown by the <tt>overlappingPairs()</tt> method. When caught, a new {@link twodsack.operation.setoperation.EarlyExit}
 * exception is thrown which indicates, that the execution of an operation was terminated early.
 */
public class NoOverlappingBoxFoundException extends Exception {
    /**
     * The 'empty constructor.
     */
    public NoOverlappingBoxFoundException() { super(); }


    /**
     * An error message can be passed using this constructor.
     */
    public NoOverlappingBoxFoundException(String s) { super(s); }
}//end Exception NoOverlappingBoxFoundException
