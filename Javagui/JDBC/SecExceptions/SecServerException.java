package SecExceptions;
import SQL2Secondo.Translator;
import Ext_Tools.completeAnswer;

public class SecServerException extends Exception {
	public SecServerException(String s) {
		//super(s);
		super();
		System.out.println(s);
	}
}
