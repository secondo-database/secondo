package SQL2Secondo;

import java.io.*;

import tools.Reporter;
import Ext_Tools.completeAnswer;
import java_cup10.runtime.*;

public class Translator {
	/** <b> Task of this class: </b> <br/>
	/* It translates queries and updates given in sql into the Secondo <br/>
	 * Optimizer syntax
	*/
	
	private String Eingang;
	private boolean Testmode = false;    // needs to be deleted
	public static completeAnswer TrOutput;
	
	public void SetTestmode(boolean TM) {
		this.Testmode = TM;
	}
	
	public completeAnswer translate(String Ein) {
		Reader StreamEingabe; // the sql string needs to be converted into a stream to be processed by the lexer
		ScanCommand MyLexer; // Lexer created by JFLex
		ParsCommand MyParser; // Parser created by Cup
		Object Result = null; // The parser returns an instance of type object
		
		this.Eingang = Ein;
		if (this.Testmode)
			TrOutput = new completeAnswer(null, this.Eingang, false);
		else {
			StreamEingabe = new StringReader(Eingang);
			MyLexer = new ScanCommand(StreamEingabe);
			MyParser = new ParsCommand(MyLexer);
		
			try {
				Result = MyParser.parse().value;
				//Reporter.reportInfo("Parserlauf fertig, Eingabe korrekt!", true); 
			}
			catch(Exception e) {
				ErrorCodes.reportException(e.toString());
				TrOutput = null;
			}
		
		//Ausgang needs to be set by the parser. That's why it is static
		}
		return TrOutput;
	}
}
