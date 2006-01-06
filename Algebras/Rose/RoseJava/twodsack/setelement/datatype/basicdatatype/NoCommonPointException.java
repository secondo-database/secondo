/*
 * NoCommonPointException.java 2006-01-06
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
package twodsack.setelement.datatype.basicdatatype;


/**
 * A NoCommonPointException is thrown, if for two Segment instances a common point shall be computed and it doesn't exist.
 */
public class NoCommonPointException extends RuntimeException {

    /**
     * The 'empty' constructor.
     */
    public NoCommonPointException() { super(); }


    /**
     * Constructs a new esception with an error message.
     *
     * @param s the error messsage
     */
    public NoCommonPointException(String s) { super(s); }

}//end Exception
    
