/*
 * SegmentsDontIntersectException.java 2005-05-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.setelement.datatype.basicdatatype;


/**
 * A SegmentsDontIntersectionException is thrown by a method which needs intersecting segments as parameters. In general,
 * simply having common points doesn't suffice for these kinds of method. Make sure, that the segments properly intersect
 * in one intersection point.
 */
public class SegmentsDontIntersectException extends RuntimeException {

    /**
     * The 'empty' constructor.
     */
    public SegmentsDontIntersectException() { super(); }


    /**
     * Constructs a new esception with an error message.
     *
     * @param s the error messsage
     */
    public SegmentsDontIntersectException(String s) { super(s); }

}//end Exception
    
