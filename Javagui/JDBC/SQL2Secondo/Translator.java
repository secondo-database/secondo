package SQL2Secondo;

import java.io.*;

import tools.Reporter;
import Ext_Tools.completeAnswer;
import java_cup10.runtime.*;

public class Translator {
	/** <b> Task of this class: </b> <br/>
	/* It translates queries and updates given in SQL92 into the Secondo <br/>
	 * Optimizer syntax
	*/
	
	private String incSQL;
	private boolean Testmode = false;    // needs to be deleted
	public static completeAnswer TrOutput;
	
	public void SetTestmode(boolean TM) {
		this.Testmode = TM;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * It invokes the scanner and the parser to have the SQL92 stream translated 
	 * @param inSql The incoming SQL92 order
	 * @return the answer of the parser
	 */
	public completeAnswer translate(String inSql) {
		Reader StreamEingabe; // the sql string needs to be converted into a stream to be processed by the scanner
		ScanCommand MyLexer; // Scanner created by JFLex
		ParsCommand MyParser; // Parser created by Cup
		Object Result = null; // The parser returns an instance of type object
		
		this.incSQL = inSql;
		if (this.Testmode)
			TrOutput = new completeAnswer(null, this.incSQL, false);
		else {
			StreamEingabe = new StringReader(incSQL);
			MyLexer = new ScanCommand(StreamEingabe);
			MyParser = new ParsCommand(MyLexer);
		
			try {
				Result = MyParser.parse().value;
			}
			catch(Exception e) {
				ErrorCodes.reportException(e.toString());
				TrOutput = null;
			}
		
		//TrOutput needs to be set by the parser. That's why it is static
		}
		return TrOutput;
	}
}
