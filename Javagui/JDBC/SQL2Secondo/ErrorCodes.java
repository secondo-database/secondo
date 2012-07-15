
package SQL2Secondo;


/**
 * 
 * <b> Task of this class </b> <br/>
 *	This class provides the content of the SQLException outputs thrown by parser, scanner or DriverSet classes
 */
public class ErrorCodes{

	public static final String NoComp1Ln = "COMPATIBILITY ERROR:\n";
	public static final String NoComp2Ln = "Secondo is not fully compatible with SQL!\n";
	public static final String NoComp3Ln = "\" might be used in the right context but is not supported by Secondo!";
	public static final String NoCompMetaData = " is not part of secondo metadata!";
	public static final String NoCompDriver = " is not supported by secondo!";
	// global variables to be used which are not related to errors
	public static int UNumber = 0;	// stands for unique number
	
	public static void reportException(String Mess) {
		System.err.println(Mess);
	}

	
	
	

}
