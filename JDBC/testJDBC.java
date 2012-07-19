import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.io.*;

public class testJDBC {

	/**
	 * <b> Task of this method </b> <br/>
	 * 
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		Connection Con1; 
		String Abfrage="";
		String SAusgabe;
		double DAusgabe;
		int IAusgabe;
		String BAusgabe;
		ResultSet rs;
		boolean IsQuery = false;
		boolean IsDBMD = false;
		BufferedReader console;
		String Line1="";
		String Line2="";
		Character L2;
		
		OpenDB oDB;
		
		Statement stmt;
		DatabaseMetaData dbmd;

		
		//IsDBMD = true;
		oDB = new OpenDB();
		oDB.openCon();
		Con1 = oDB.getCon();
		
		
		try {
			stmt = Con1.createStatement();
			//Con1.setAutoCommit(false);
			
			System.out.println("Enter a query: ");
			try {
				console = new BufferedReader(new InputStreamReader(System.in));
				Abfrage = console.readLine();
			} catch (IOException e) {
				System.out.println("Error in reading at console!!!");
			}
			if (Abfrage.startsWith("select") || Abfrage.startsWith("SELECT") || Abfrage.startsWith("Select"))
				IsQuery = true;
			
			if (IsDBMD) {
				dbmd = Con1.getMetaData();
				rs = dbmd.getTables("", "", "", null);
				while (rs.next()) {
					SAusgabe = rs.getString(3);
					System.out.println(SAusgabe);
				}
			}
			else if (IsQuery){
				rs = stmt.executeQuery(Abfrage);
				console = new BufferedReader(new InputStreamReader(System.in));
				try {
					System.out.println("Enter name of column: ");
					Line1 = console.readLine();
					System.out.println("Enter type of column (string, real, bool, int): ");
					Line2 = console.readLine();
				} 
				catch (IOException e) {
					System.out.println("Error in reading at console!!!");
				}
				L2 = Line2.charAt(0);
				switch (L2) {
					case 's': 
						while (rs.next()) {
							SAusgabe = rs.getString(Line1);
							TestReporter.showMessage(SAusgabe+"\n");
						}
						break;
					case 'r':
						while (rs.next()) {
							DAusgabe = rs.getDouble(Line1);
							TestReporter.showMessage(DAusgabe+"\n");
						}
						break;
					case 'b':
						while (rs.next()) {
							BAusgabe = rs.getBoolean(Line1) ? "True" : "False";
							TestReporter.showMessage(BAusgabe+"\n");
						}
						break;
					case 'i':
						while (rs.next()) {
							IAusgabe = rs.getInt(Line1);
							TestReporter.showMessage(IAusgabe+"\n");
						}
				}
				
				rs.close();
			}
			else {
				int Anz = stmt.executeUpdate(Abfrage);
				TestReporter.showMessage(""+Anz);
			}
			stmt.close();
			//Con1.commit();
			Con1.close();
		}
		
		catch(SQLException e) {
			System.out.print(e.toString()+"\nHat leider nicht geklappt");
		}
		
		
	}


}
