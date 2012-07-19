import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import LocalTest.CheckerII;


public class TutorialTest {
	
	

	
	public static void main(String[] args) throws Exception {
		Connection Con1; 
		String testQuery="";
		
		OpenDB oDB;
		Statement stmt;
		DatabaseMetaData dbmd;
		
		CheckerII chkII;
		int whichTutorial=0;
		
		BufferedReader console;
		
		
		oDB = new OpenDB();
		oDB.openCon();
		Con1 = oDB.getCon();
		
		chkII = new CheckerII();
		
		System.out.println("Which tutorial should be executed. Enter 1 - 2: ");
		try {
			console = new BufferedReader(new InputStreamReader(System.in));
			whichTutorial =  Integer.valueOf(console.readLine());
		} catch (IOException e) {
			System.out.println("Error in reading at console!!!");
		}
		
		testQuery = chkII.OutputSelect(whichTutorial);
		stmt = Con1.createStatement();
		dbmd = Con1.getMetaData();
		while (!testQuery.endsWith("|ENDE|")) {
			System.out.println(testQuery);
			//rs = stmt.executeQuery(testQuery);
			getResult(stmt, testQuery, chkII.OutputRS(whichTutorial), dbmd);
			TestReporter.showMessage("Hit Enter to carry on!");
			testQuery = chkII.OutputSelect(whichTutorial);
		}
		
		stmt.close();
		Con1.close();
	}
	
	
	
	private static void getResult(Statement stmt, String TestQuery, String chosenColumn, DatabaseMetaData dbmd) {
		Character ColType;
		String Col;
		String SResult;
		Double DResult;
		String BResult;
		int IResult;
		ResultSet rs;
		
		ColType = chosenColumn.charAt(0);
		Col = chosenColumn.substring(2);
		try {
			switch (ColType) {
				case 's': 
					rs = stmt.executeQuery(TestQuery);
					while (rs.next()) {
						SResult = rs.getString(Col);
						System.out.print(SResult+"\n");
					}
					break;
				case 'r':
					rs = stmt.executeQuery(TestQuery);
					while (rs.next()) {
						DResult = rs.getDouble(Col);
						System.out.print(DResult+"\n");
					}
					break;
				case 'b':
					rs = stmt.executeQuery(TestQuery);
					while (rs.next()) {
						BResult = rs.getBoolean(Col) ? "True" : "False";
						System.out.print(BResult+"\n");
					}
					break;
				case 'i':
					rs = stmt.executeQuery(TestQuery);
					while (rs.next()) {
						if (Col.equalsIgnoreCase(" "))
							IResult = rs.getInt("");
						else
							IResult = rs.getInt(Col);
						System.out.print(IResult+"\n");
					}
					break;
				case 'u':	//update
					IResult = stmt.executeUpdate(TestQuery);
					System.out.print(IResult+"\n");
					break;
				case 't':	//metadata gettables
					rs = dbmd.getTables("", "", "", null);
					while (rs.next()) {
						SResult = rs.getString(3);
						System.out.print(SResult+"\n");
					}
					break;
				case 'c':	//metadata getcolumns
					rs = dbmd.getColumns(null, null, TestQuery, null);
					while (rs.next()) {
						SResult = rs.getString(Col);
						System.out.println(SResult);
					}
					
			}
		} catch(SQLException e) {
			System.out.print(e.toString()+"\nDid not work out!!!");
		}
	}
}
