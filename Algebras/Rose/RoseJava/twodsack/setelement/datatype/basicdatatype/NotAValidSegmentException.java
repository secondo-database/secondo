/*
 * NotAValidSegmentException.java 2005-05-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.setelement.datatype.basicdatatype;


/**
 * A NotAValidSegmentException is thrown by a {@link twodsack.setelement.datatype.basicdatatype.Segment} constructor if both points are equal.
 */
public class NotAValidSegmentException extends RuntimeException {

    /**
     * The 'empty' constructor.
     */
    public NotAValidSegmentException() { super(); }


    /**
     * Constructs a new exception with an error message.
     *
     * @param s the error messsage
     */
    public NotAValidSegmentException(String s) { super(s); }

}//end Exception
    
