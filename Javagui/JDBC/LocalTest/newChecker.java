package LocalTest;


/**
 * 
 * <b> Task of this class </b> <br/>
 * provides a general test with common queries 
 */
public class newChecker {
	
	final private int NoCreate = 9;
	final private int NoSelect = 30;
	final private int NoInsert = 4;
	final private int NoUpdate = 2;
	final private int NoDelete = 1;
	final private int NoDrop   = 3 ;
	final private int NoAlter  = 2;
	
	final private int NoUnsupported = 26;
	
	private String[] TeCreate;
	private String[] TeSelect;
	private String[] TeInsert;
	private String[] TeUpdate;
	private String[] TeDelete;
	private String[] TeDrop;
	private String[] TeAlter;
	private String[] TeUnsupported;
	
	private int CounterOutput;
	
	public newChecker() {
		TeCreate = new String[NoCreate];
		TeSelect = new String[NoSelect];
		TeInsert = new String[NoInsert];
		TeUpdate = new String[NoUpdate];
		TeDelete = new String[NoDelete];
		TeDrop = new String[NoDrop];
		TeAlter = new String[NoAlter];
		TeUnsupported = new String[NoUnsupported];
		this.createTest();
		this.CounterOutput = 0;
	}
	
	private void createTest() {
		TeCreate[0] = "CREATE TABLE typetest2 (tstring CHARACTER(50), treal REAL, tint DEC(10))";
		TeCreate[1] = "CREATE TABLE typetest3 (tstring CHARACTER(50), tint DEC(10))";
		TeCreate[2] = "CREATE TABLE typetest4 (tstring CHARACTER(50))";
		TeCreate[3] = "CREATE TABLE typetest5 (tint DEC(10))";
		TeCreate[4] = "CREATE TABLE typetest6 (treal REAL, tint DEC(10))";
		TeCreate[5] = "CREATE INDEX myindex ON typetest2 (treal)";
		TeCreate[6] = "CREATE INDEX myindex USING btree ON typetest2 (treal)";
		TeCreate[7] = "CREATE TABLE typetest (tstring CHARACTER(50), treal REAL, tint DEC(10), tbool BIT(1))";
		TeCreate[8] = "CREATE TABLE typetest (tstring CHARACTER(50), treal REAL, tint DEC(10), tbool BOOLEAN)";
		
		TeSelect[0] = "SELECT 5.232E-2*no FROM tentest";
		TeSelect[1] = "SELECT 5.323E-3 FROM tentest";
		TeSelect[2] = "SELECT no, 5+5 FROM tentest";
		TeSelect[3] = "SELECT 5*5 as myatt5 FROM tentest";
		TeSelect[4] = "SELECT DISTINCT no FROM tentest";
		TeSelect[5] = "SELECT ALL no FROM tentest";
		TeSelect[6] = "SELECT count(*) FROM tentest";
		TeSelect[7] = "SELECT min(distinct no) FROM tentest";
		TeSelect[8] = "SELECT min(no) FROM tentest";
		TeSelect[9] = "SELECT min(all no) FROM tentest";
		TeSelect[10] = "SELECT no FROM tentest myTable";
		TeSelect[11] = "SELECT no FROM tentest as mytable";
		TeSelect[12] = "SELECT no FROM tentest WHERE 5>tentest.no";
		TeSelect[13] = "SELECT no FROM tentest WHERE no<8";
		TeSelect[14] = "SELECT no FROM tentest WHERE exists (select no from twentytest where tentest.no = twentytest.no)";
		TeSelect[15] = "SELECT no FROM tentest WHERE exists (select no from twentytest where twentytest.no = tentest.no)";
		TeSelect[16] = "SELECT no FROM tentest WHERE no >3 and no <5";
		TeSelect[17] = "SELECT no FROM tentest WHERE no<5 or no >8";
		TeSelect[18] = "SELECT no FROM tentest WHERE 5*no in (SELECT tint FROM typetest)";
		TeSelect[19] = "SELECT no FROM tentest WHERE no<5 or no >8 and no>6";
		TeSelect[20] = "SELECT no FROM tentest WHERE no<5 and no >8 or no>6";
		TeSelect[21] = "SELECT no FROM tentest WHERE tstring = 'Hallo'";
		TeSelect[22] = "SELECT min(treal) as minreal FROM typetest GROUP BY tint";
		TeSelect[23] = "SELECT min(treal), tint FROM typetest as ty GROUP BY tint";
		TeSelect[24] = "SELECT * FROM typetest ORDER BY tint asc, treal desc";
		TeSelect[25] = "SELECT * FROM typetest where tint<9 ORDER BY tint";
		TeSelect[26] = "SELECT no FROM tentest myTable, twentest as myTable2";
		TeSelect[27] = "SELECT no FROM tentest as mytable, twentest";
		TeSelect[28] = "SELECT Employees.First_Name, Employees.Last_Name, Cars.Make, Cars.Model, Cars.Year FROM Employees, Cars WHERE Employees.Car_Number = Cars.Car_Number";
		TeSelect[29] = "SELECT * FROM typetest where typetest.tint<9 ORDER BY tint";
		
		
		TeInsert[0] = "INSERT INTO typetest2 VALUES ('Sieben', 7.777, 7)";
		TeInsert[1] = "INSERT INTO tentest VALUES (11)";
		TeInsert[2] = "INSERT INTO typetest2 SELECT Tstring, treal, tint FROM typetest WHERE tint = 2";
		TeInsert[3] = "INSERT INTO typetest VALUES ('Sieben', 7.777, 7, false)";
		
		TeUpdate[0] = "UPDATE typetest2 SET tint=6, treal=6.666, tstring='6. String'";
		TeUpdate[1] = "UPDATE tentest SET no=5 WHERE no >3 and no <5";
		
		TeDelete[0] = "DELETE FROM typetest2 WHERE tint<2 OR tint>9 AND tstring = 'Michael'";
		
		TeDrop[0] = "DROP TABLE supplier2";
		TeDrop[1] = "DROP INDEX typetest2_treal_btree ON typetest2";
		TeDrop[2] = "DROP INDEX ON typetest treal";
		//TeDrop[3] = "DROP INDEX ON typetest treal indextype rtree";
		
		TeAlter[0] = "ALTER TABLE typetest ADD COLUMN Vorname CHAR(20)";
		TeAlter[1] = "ALTER TABLE typetest DROP vorname";
		
		TeUnsupported[0] = "CREATE TABLE myTable (Name CHARACTER(50) PRIMARY KEY)";
		TeUnsupported[1] = "CREATE TABLE myTable (Name CHARACTER(50) CONSTRAINT myrule UNIQUE)";
		TeUnsupported[2] = "CREATE TABLE myTable (Name CHARACTER(50) CONSTRAINT myrule UNIQUE REFERENCES mytabel(Sp1, Sp2))";
		TeUnsupported[3] = "CREATE TABLE myTable (CONSTRAINT myTableRule FOREIGN KEY (Sp3, Sp4) REFERENCES mytable2(Sp8, Sp9), Name CHARACTER(50) CONSTRAINT myrule UNIQUE REFERENCES mytabel(Sp1, Sp2), Alter DECIMAL(10))";
		TeUnsupported[4] = "CREATE TABLE myTable (CONSTRAINT myTableRule FOREIGN KEY (Sp3, Sp4) REFERENCES mytable2(Sp8, Sp9), Name CHARACTER(50), Alter DECIMAL(10))";
		TeUnsupported[5] = "CREATE TABLE myTable (CONSTRAINT myTableRule FOREIGN KEY (Sp3, Sp4) REFERENCES mytable2(Sp8, Sp9), Name CHARACTER(50))";
		TeUnsupported[6] = "CREATE TABLE myTable (Name CHARACTER(50) CONSTRAINT myrule UNIQUE REFERENCES mytabel(Sp1, Sp2), Alter DECIMAL(10))";
		TeUnsupported[7] = "CREATE TABLE myTable (Name CHARACTER(50) UNIQUE REFERENCES mytabel(Sp1, Sp2))";
		TeUnsupported[8] = "CREATE TABLE myTable (Name CHAR(50) REFERENCES myothertable(Sp1, Sp2))";
		TeUnsupported[9] = "CREATE TABLE myTable (Name CHAR(50) PRIMARY KEY, Alter DECIMAL(10))";
		TeUnsupported[10] = "CREATE GLOBAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DECIMAL(10))";
		TeUnsupported[11] = "CREATE LOCAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DECIMAL(10))";
		TeUnsupported[12] = "SELECT no FROM tentest cross join twentest union twentest";
		TeUnsupported[13] = "SELECT no FROM tentest cross join twentest except twentest";
		TeUnsupported[14] = "SELECT no FROM tentest cross join twentest except twentest corresponding by no";
		TeUnsupported[15] = "SELECT no FROM tentest cross join twentest except values (1), (5)";
		TeUnsupported[16] = "SELECT nullif(no,5) FROM tentest";
		TeUnsupported[17] = "SELECT coalesce(5, no, null) FROM tentest";
		TeUnsupported[18] = "SELECT case no when 5 then 8+no when 3 then 5+no else 5.34 FROM tentest";
		TeUnsupported[19] = "SELECT cast (einstring as int) +5 FROM tentest";
		TeUnsupported[20] = "DROP TABLE typetest6 RESTRICT";
		TeUnsupported[21] = "INSERT INTO typetest values ('eins', 1.1, 1 , date '2012-04-02')";
		TeUnsupported[22] = "SELECT * FROM typetest where typetest.tint<9 ORDER BY tint";
		TeUnsupported[23] = "CREATE INDEX myindex USING btree ON typetest2 (treal, tint)";
		TeUnsupported[24] = "CREATE FULLTEXT INDEX myindex ON typetest2 (treal)";
		TeUnsupported[25] = "CREATE UNIQUE INDEX myindex ON typetest2 (treal)";
				
	}
	
	public String Output(String OT) {
		String Answer = "|ENDE|";
		Character OutputType;
		
		if (OT.equalsIgnoreCase("drop"))
			OutputType = 'r';
		else if (OT.equalsIgnoreCase("unsupported"))
			OutputType = 'n';
		else
			OutputType = OT.charAt(0);
		
		switch (OutputType) {
			case 'c':		
				if (CounterOutput < this.NoCreate)
					Answer = this.TeCreate[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
				break;
			case 's':		
				if (CounterOutput < this.NoSelect)
					Answer = this.TeSelect[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
				break;
			case 'i':
				if (CounterOutput < this.NoInsert)
					Answer = this.TeInsert[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
				break;
			case 'u':
				if (CounterOutput < this.NoUpdate)
					Answer = this.TeUpdate[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
				break;
			case 'd':
				if (CounterOutput < this.NoDelete)
					Answer = this.TeDelete[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
				break;
			case 'r':
				if (CounterOutput < this.NoDrop)
					Answer = this.TeDrop[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
				break;
			case 'a':
				if (CounterOutput < this.NoAlter)
					Answer = this.TeAlter[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
				break;
			case 'n':
				if (CounterOutput < this.NoUnsupported)
					Answer = this.TeUnsupported[CounterOutput];
				else {
					Answer = "|ENDE|";
					this.CounterOutput = -1;
				}
				CounterOutput++;
		}
				
		return Answer;
	}
	
	
	
	
}
