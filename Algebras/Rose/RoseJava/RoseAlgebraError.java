/*
 * RoseAlgebraError.java 2006-01-03
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

/**
 * An exception of this type is thrown for any error that occurs during the execution of a ROSE algebra operation.
 * Use the string to pass detailed information about the error.
 */
public class RoseAlgebraError extends RuntimeException {

    /**
     * This member stores the error message.
     */
    public String errorMessage;

    /**
     * Constructs a new instance of RoseAlgebraError using the argument supplied.
     *
     * @param s the string with description about this exception.
     */
    public RoseAlgebraError(String s) {
	super(s);
	this.errorMessage = s;
    }
}//end class RoseAlgebraError
