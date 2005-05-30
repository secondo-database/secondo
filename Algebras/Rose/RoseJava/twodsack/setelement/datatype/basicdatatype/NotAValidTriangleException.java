/*
 * NotAValidTriangleException.java 2005-05-04
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.setelement.datatype.basicdatatype;


/**
 * A NotAValidTriangleException is thrown by a {@link twodsack.setelement.datatype.basicdatatype.Triangle} constructor if the three given points don't form a valid triangle.
 */
public class NotAValidTriangleException extends RuntimeException {

    /**
     * The 'empty' constructor.
     */
    public NotAValidTriangleException() { super(); }


    /**
     * Constructs a new exception with an error message.
     *
     * @param s the error messsage
     */
    public NotAValidTriangleException(String s) { super(s); }

}//end Exception
    
