package LocalTest;


/**
 * 
 * <b> Task of this class </b> <br/>
 * It contains the tests according to the tutorials
 */
public class CheckerII {
	private final int NoTut1 = 12;
	private final int NoTut2 = 24;
	private final int NoTut3 = 0;
	private final int NoTut4 = 0;
	private final int NoTut5 = 0;
	
	private String[] stmtsTut1;
	private String[] stmtsTut2;
	private String[] stmtsTut3;
	private String[] stmtsTut4;
	private String[] stmtsTut5;
	
	private String[] resultTut1;
	private String[] resultTut2;
	private String[] resultTut3;
	private String[] resultTut4;
	private String[] resultTut5;
	
	private int CounterOutput;
	
	public CheckerII() {
		this.CounterOutput = 0;
		
		stmtsTut1 = new String[NoTut1];
		resultTut1 = new String[NoTut1];
		
		stmtsTut2 = new String[NoTut2];
		resultTut2 = new String[NoTut2];
		
		stmtsTut3 = new String[NoTut3];
		resultTut3 = new String[NoTut3];
		
		stmtsTut4 = new String[NoTut4];
		resultTut4 = new String[NoTut4];
		
		stmtsTut5 = new String[NoTut5];
		resultTut5 = new String[NoTut5];
		/* Tutorial 1 */
		
		stmtsTut1[0] = "SELECT * FROM typetest WHERE tint = 5";
		resultTut1[0] = "i;tint";
		
		stmtsTut1[1] = "INSERT INTO typetest VALUES ('Fuenfter String', 5.555, 5, false)";
		resultTut1[1] = "u;nothing";
		
		stmtsTut1[2] = "CREATE TABLE dir (did       INT, dname     CHAR(100),fatherdid INT, entries   INT)";
		resultTut1[2] = "u;nothing";
		
		stmtsTut1[3] = "CREATE TABLE file (fid INT , did INT, fname CHAR(100), fsize INT, fdate CHAR(8), ftime CHAR(5))";
		resultTut1[3] = "u;nothing";
		
		int did = 1; String name = "Datei1"; int fatherdid = 5;
		stmtsTut1[4] = "INSERT INTO dir VALUES (" + did + "," + "\'" + name + "\'," + fatherdid + "," +"0)";
		resultTut1[4] = "u;nothing";
		
		int entries = 1;
		stmtsTut1[5] = "UPDATE dir SET entries = " + entries + "  WHERE did = " + did;
		resultTut1[5] = "u;nothing";
		
		stmtsTut1[6] = "SELECT count(*) FROM dir";
		resultTut1[6] = "i; ";
		
		stmtsTut1[7] = "SELECT * FROM dir WHERE did = " + did;
		resultTut1[7] = "s;dname";
		
		// if ((did = rs.getInt("fatherdid")) == 0)
		stmtsTut1[8] = "SELECT * FROM dir WHERE dname LIKE \'" + name + "\'";
		resultTut1[8] = "s;dname";
		
		stmtsTut1[9] = "SELECT * FROM dir ORDER BY did DESC";
		resultTut1[9] = "s;dname";
		
		stmtsTut1[10] = "DROP TABLE dir";
		resultTut1[10] = "u;nothing";
		
		stmtsTut1[11] = "DROP TABLE file";
		resultTut1[11] = "u;nothing";
		
		// Connection.TRANSACTION_NONE 	Connection.TRANSACTION_READ_UNCOMMITTED	Connection.TRANSACTION_READ_COMMITTED
		// Connection.TRANSACTION_REPEATABLE_READ Connection.TRANSACTION_SERIALIZABLE 
		
		/* Tutorial 2 http://docs.oracle.com/javase/tutorial/jdbc/overview/index.html*/
		
		stmtsTut2[0] = "CREATE TABLE Employees (Employee_Number INT, First_Name CHAR(100),Last_Name CHAR(100), Date_of_Birth CHAR(9))";
		resultTut2[0] = "u;nothing";
		
		stmtsTut2[1] = "INSERT INTO Employees VALUES (10001 , 'Axels', 'Washington', '28-Aug-43')";
		resultTut2[1] = "u;nothing";
		
		stmtsTut2[2] = "INSERT INTO Employees VALUES (10083 , 'Arvid', 'Sharma', '24-Nov-54')";
		resultTut2[2] = "u;nothing";
		
		stmtsTut2[3] = "INSERT INTO Employees VALUES (10120 , 'Jonas', 'Ginsberg', '01-Jan-69')";
		resultTut2[3] = "u;nothing";
		
		stmtsTut2[4] = "INSERT INTO Employees VALUES (10005 , 'Florence', 'Wojokowski', '04-Jul-71')";
		resultTut2[4] = "u;nothing";
		
		stmtsTut2[5] = "INSERT INTO Employees VALUES (10099 , 'Sean', 'Washington', '21-Sep-66')";
		resultTut2[5] = "u;nothing";
		
		stmtsTut2[6] = "INSERT INTO Employees VALUES (10035 , 'Elizabeth', 'Yamaguchi', '24-Dez-59')";
		resultTut2[6] = "u;nothing";
		
		stmtsTut2[7] = "ALTER TABLE Employees ADD Car_Number DEC(20)";
		resultTut2[7] = "u;nothing";
		
		stmtsTut2[8] = "UPDATE Employees SET First_Name='Axel' WHERE Employee_Number=10001";
		resultTut2[8] = "u;nothing";
		
		stmtsTut2[9] = "UPDATE Employees SET Car_Number=5 WHERE Employee_Number=10001";
		resultTut2[9] = "u;nothing";
		
		stmtsTut2[10] = "UPDATE Employees SET Car_Number=12 WHERE Employee_Number=10005";
		resultTut2[10] = "u;nothing";
		
		
		stmtsTut2[11] = "CREATE TABLE Cars (Car_Number INT, Make CHAR(100),Model CHAR(100), Year CHAR(4))";
		resultTut2[11] = "u;nothing";
		
		stmtsTut2[12] = "INSERT INTO Cars VALUES (5 , 'Honda', 'Civic DX', '1996')";
		resultTut2[12] = "u;nothing";
		
		stmtsTut2[13] = "INSERT INTO Cars VALUES (12 , 'Toyota', 'Corolla', '1999')";
		resultTut2[13] = "u;nothing";
		
				
		stmtsTut2[14] = "SELECT First_Name, Last_Name FROM Employees WHERE Car_Number IS NOT NULL";
		resultTut2[14] = "s;First_Name";
		
		stmtsTut2[15] = "SELECT * FROM Employees";
		resultTut2[15] = "s;First_Name";
		
		stmtsTut2[16] = "SELECT First_Name, Last_Name FROM Employees	WHERE Last_Name LIKE 'Washington%'";
		resultTut2[16] = "s;Last_Name";
		
		stmtsTut2[17] = "SELECT First_Name, Last_Name FROM Employees	WHERE Last_Name LIKE 'Ba_man'";
		resultTut2[17] = "s;Last_Name";
		
		stmtsTut2[18] = "SELECT First_Name, Last_Name FROM Employees WHERE Car_Number = 12";
		resultTut2[18] = "s;Last_Name";
		
		stmtsTut2[19] = "SELECT First_Name, Last_Name FROM Employees WHERE Employee_Number > 10005";
		resultTut2[19] = "s;Last_Name";
				
		stmtsTut2[20] = "SELECT First_Name, Last_Name FROM Employees WHERE Employee_Number < 10100 and Car_Number IS NULL";
		resultTut2[20] = "s;Last_Name";
		
		stmtsTut2[21] = "SELECT Employees.First_Name, Employees.Last_Name, Cars.Make, Cars.Model, Cars.Year FROM Employees, Cars WHERE Employees.Car_Number = Cars.Car_Number";
		resultTut2[21] = "s;Last_Name";
		
		stmtsTut2[22] = "DROP TABLE Employees";
		resultTut2[22] = "u;nothing";
		
		stmtsTut2[23] = "DROP TABLE Cars";
		resultTut2[23] = "u;nothing";
		
		/* Tutorial 3 */
		
		// stmtsTut3[0] = "";

		
		/* Tutorial 4 */
		
		// stmtsTut4[0] = "";

		
		/* Tutorial 5 */
		
		// stmtsTut5[0] = "";
		
	}
	
	public String OutputSelect(int Tut) {
		
		String result;
		String[] Outp;
		switch (Tut) {
			case 1: {
				Outp = stmtsTut1;
				break;
			}
			case 2: {
				Outp = stmtsTut2;
				break;
			}
			case 3: {
				Outp = stmtsTut3;
				break;
			}
			case 4: {
				Outp = stmtsTut4;
				break;
			}
			default: {
				Outp = stmtsTut5;
			}
		}
		
		if (CounterOutput < Outp.length)
			result = Outp[CounterOutput];
		else
			result = "|ENDE|";		
		CounterOutput++;
		return result;
	}
	
	public String OutputRS(int Tut) {
		String[] RS;
		String result;
		
		switch (Tut) {
			case 1: {
				RS = resultTut1;
				break;
			}
			case 2: {
				RS = resultTut2;
				break;
			}
			case 3: {
				RS = resultTut3;
				break;
			}
			case 4: {
				RS = resultTut4;
				break;
			}
			default: {
				RS = resultTut5;
			}
		}
		
		result = RS[this.CounterOutput - 1];
		
		return result;
	}
}
