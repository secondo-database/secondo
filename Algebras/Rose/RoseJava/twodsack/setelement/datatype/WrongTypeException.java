/*
 * WrongTypeException.java 2005-05-03
 *
 * Dirk Ansorge FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype;


/**
 * A WrongTypeException is thrown everytime a method needs two or more objects of the same type, but gets objects of differnt
 * types. A typical reason is when two objects of type Element are passed to a compare() method which then detects that
 * one Element object is e.g. of type Triangle and the other is of type Segment. WrongTypeExceptions are also thrown by
 * comparators and equal() methods.
 */
public class WrongTypeException extends RuntimeException {
    /**
     * The 'empty' constructor.
     */
    public WrongTypeException() { super(); }

    /**
     * Constructs a new exception with a error message as parameter.
     *
     * @param s the error message
     */
    public WrongTypeException(String s) { super(s); }

}//end Exception WrongTypeException
