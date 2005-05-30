/*
 * WrongTypeException.java 2005-05-03
 *
 * Dirk Ansorge FernUniversitaet Hagen
 *
 */

package twodsack.setelement.datatype;


/**
 * A WrongTypeException is thrown everytime a method needs two or more objects of the same type, but gets objects of different types.<p>
 * A typical reason is when two objects of type {@link twodsack.setelement.Element} are passed to a <tt>compare()</tt> method which then detects that
 * one <tt>Element</tt> object is e.g. of type {@link twodsack.setelement.datatype.basicdatatype.Triangle} and the other is of type
 * {@link twodsack.setelement.datatype.basicdatatype.Segment}. <tt>WrongTypeException</tt>s are also thrown by
 * comparators and <tt>equal()</tt> methods.
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
