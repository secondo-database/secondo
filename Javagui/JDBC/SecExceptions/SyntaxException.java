package SecExceptions;
import SQL2Secondo.Translator;
import Ext_Tools.completeAnswer;

public class SyntaxException extends Exception {
	
	public SyntaxException(String s) {
		super();
		//super("SYNTAX ERROR: Cannot resolve \""+ s.toUpperCase() +"\"");
		System.out.println("SYNTAX ERROR: Cannot resolve \""+ s.toUpperCase() +"\"");
	}

}
