package LocalTest;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import SQL2Secondo.Translator;

/**
 * 
 * <b> Task of this class </b> <br/>
 * class to execute newChecker
 */
public class newParserTest {

	private static void generateOutput(newChecker ch, Translator tr, String OutputType) {
		String Output;
		
		Output=ch.Output(OutputType);
		while (!Output.endsWith("|ENDE|")) {
			TestReporter.writeInFile(Output);
			TestReporter.writeInFile(tr.translate(Output).getOutput());
			TestReporter.lnforward();
			Output=ch.Output(OutputType);
		}
	}
	
	private static void generateErrorOutput(newChecker ch, Translator tr, String OutputType) {
		String Output, Err;
		
		Output=ch.Output(OutputType);
		while (!Output.endsWith("|ENDE|")) {
			TestReporter.writeInFile(Output);
			try{
				tr.translate(Output).getOutput();
			}
			catch (Exception e) {
				TestReporter.writeInFile(e.toString());
			}
			TestReporter.lnforward();
			Output=ch.Output(OutputType);
		}
	}
	
	
	/**
	 * <b> Task of this method </b> <br/>
	 *
	 * @param args
	 */
	public static void main(String[] args) {
		
		Translator tr = new Translator();
		newChecker ch = new newChecker();
		BufferedReader console;
		String Answer = "";
		
		tr.SetTestmode(false);
		
		TestReporter.startFileWriting("D:\\temp\\Secondo\\AuswertungParser.txt");
		
		TestReporter.writeInFile("****** CREATE TESTS ******");
		TestReporter.lnforward();
		TestReporter.lnforward();
		generateOutput(ch, tr, "create");
		
		TestReporter.writeInFile("****** SELECT TESTS ******");
		TestReporter.lnforward();
		TestReporter.lnforward();
		generateOutput(ch, tr, "select");
		
		TestReporter.writeInFile("****** INSERT TESTS ******");
		TestReporter.lnforward();
		TestReporter.lnforward();
		generateOutput(ch, tr, "insert");
		
		TestReporter.writeInFile("****** UPDATE TESTS ******");
		TestReporter.lnforward();
		TestReporter.lnforward();
		generateOutput(ch, tr, "update");
		
		TestReporter.writeInFile("****** DELETE TESTS ******");
		TestReporter.lnforward();
		TestReporter.lnforward();
		generateOutput(ch, tr, "delete");
		
		TestReporter.writeInFile("****** DROP TESTS ******");
		TestReporter.lnforward();
		TestReporter.lnforward();
		generateOutput(ch, tr, "drop");
		
		TestReporter.writeInFile("****** ALTER TESTS ******");
		TestReporter.lnforward();
		TestReporter.lnforward();
		generateOutput(ch, tr, "alter");
		
		
		console = new BufferedReader(new InputStreamReader(System.in));
		try {
			System.out.println("Should the unsupported tests be executed as well (y/n) ?: ");
			Answer = console.readLine();
		} 
		catch (IOException e) {
			System.out.println("Error in reading at console!!!");
		}
		
		if (Answer.startsWith("y")) {
			TestReporter.writeInFile("****** UNSUPPORTED TESTS ******");
			TestReporter.lnforward();
			TestReporter.lnforward();
			generateErrorOutput(ch, tr, "unsupported");
		}
		
		TestReporter.endFileWriting();

	}

}
