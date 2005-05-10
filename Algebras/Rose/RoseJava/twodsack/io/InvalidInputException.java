/*
 *
 * InvalidInputException, 2004-11-29
 *
 * Thomas Behr, FernUniversitaet Hagen
 * adapted by Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.io;

import java.lang.RuntimeException;

/**
 * An InvalidInputException expresses an input that is not accepted by a certain method.
 */
public class InvalidInputException extends RuntimeException{
    
    /**
     * Constructs a new InvalidInputException.
     * Use the argument to pass a message for better understanding and debugging.
     *
     * @param S the error message
     */
    public InvalidInputException(String S){ super(S);}
}
