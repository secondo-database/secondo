package LocalTest;

import SQL2Secondo.Translator;

/**
 * 
 * <b> Task of this class </b> <br/>
 * class to execute checker
 */
public class ParserTest {

	
	public static void main(String[] args) {
		
		Translator tr = new Translator();
		checker ch = new checker();
		String Output;
		
		tr.SetTestmode(false);
		
		//tr.translate("Schema.Tabellenname.Spaltenname");
		//tr.translate("6.5654654654E-20");
		//tr.translate("Select ALL");
		//tr.translate("CREATE TABLE Schemaname.myTable (Name CHARACTER(50), Alter DEC(10))");
		//Output = tr.translate("CREATE TABLE myTable (Name CHARACTER(50), Alter DEC(10))");
		//Output = "select 'H\"allo' from tentest";
		
		//Output = ch.OutputSelect();
		Output = ch.OutputSingle();
		
		TestReporter.startFileWriting("D:\\temp\\Secondo\\AuswertungParser.txt");
		while (!Output.endsWith("|ENDE|")) {
			TestReporter.writeInFile(Output);
			TestReporter.writeInFile(tr.translate(Output).getOutput());
			TestReporter.lnforward();
			Output=ch.OutputSingle();
			//Output=ch.OutputSelect();
		}
		
		
		TestReporter.endFileWriting();	
		
		//Output = tr.translate("SELECT * FROM TestTab");
		/* possible Create variations
		 * CREATE LOCAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DEC(10))
		 * CREATE LOCAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DEC(10)) ON COMMIT DELETE ROWS
		 * CREATE TABLE myTable (Name CHARACTER(50) DEFAULT CURRENT USER, Alter DEC(10))
		 * CREATE TABLE myTable (Name Schemaname.Varname, Alter DEC(10)) //column definition ::= <column name> <domain name> instead of ::= <column name> <data type>
		 * CREATE TABLE myTable (Name CHARACTER(50) COLLATE latin_1, Alter DEC(10))
		 * CREATE TABLE Schemaname.myTable (Name CHARACTER(50), Alter DEC(10))
		 */
		
		//System.out.println(Output);
	}

}
