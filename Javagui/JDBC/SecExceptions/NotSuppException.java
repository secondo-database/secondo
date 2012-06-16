package SecExceptions;

import SQL2Secondo.ErrorCodes;
import SQL2Secondo.Translator;
import java.io.*;
import Ext_Tools.completeAnswer;

public class NotSuppException extends IOException {
	
	public NotSuppException(String s) {
		super();
		/*super(ErrorCodes.NoComp1Ln+ErrorCodes.NoComp2Ln+"\""
		+s.toUpperCase()+ErrorCodes.NoComp3Ln);*/
		System.out.println(ErrorCodes.NoComp1Ln+ErrorCodes.NoComp2Ln+"\"" 
		+s.toUpperCase()+ErrorCodes.NoComp3Ln);
	}
}
