package LocalTest;

/**
 * <b>Task of this class</b><b/>
 * It checks weather all possible SQL constructions are evaluated correctly
 */

public class checker {
	final private int NoValueExpression = 3;
	final private int NoDistinct = 1;
	final private int Anzahl = 100;
	final private int NoQueryExpression = 0;  //8;
	final private int NoTableReference = 6;
	final private int NoSearchCond= 25;
	final private int NoGroupBy = 11;
	final private int NoOrderBy = 8;
	private String[] Testeingabe;
	private String[] ValueExprTest;
	private String[] TableRefTest;
	private String[] SearchCondTest;
	private String[] QueryExprTest;
	private String[] DistinctTest;
	private String[] Ausgabe;
	private String[] EingabeGroupBy;
	private String[] EingabeOrderBy;
	private int CounterOutput;
	
	private void testCreate() {
		Testeingabe = new String[Anzahl];
		Testeingabe[0] = "CREATE TABLE myTable (Name CHARACTER(50), Alter DECIMAL(10))";
		Testeingabe[1] = "CREATE TABLE myTable (Name CHARACTER(50))";
		Testeingabe[2] = "CREATE GLOBAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DECIMAL(10))";
		Testeingabe[3] = "CREATE LOCAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DECIMAL(10))";
		Testeingabe[4] = "CREATE LOCAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DECIMAL(10)) ON COMMIT PRESERVE ROWS";
		Testeingabe[5] = "CREATE LOCAL TEMPORARY TABLE myTable (Name CHARACTER(50), Alter DECIMAL(10)) ON COMMIT DELETE ROWS";
		
		DistinctTest = new String[NoDistinct];
		DistinctTest[0] = "distinct ";
		/*DistinctTest[1] = "all ";
		DistinctTest[2]= "";*/
		
		ValueExprTest = new String[NoValueExpression];
		/*ValueExprTest[0] = "5+5"; 
		ValueExprTest[1] = "5*5";
		ValueExprTest[2] = "5-4";
		ValueExprTest[3] = "10/5";
		ValueExprTest[4] = "5";
		ValueExprTest[5] = "5.323E-3";
		ValueExprTest[6] = "5.323E5*10";
		ValueExprTest[7] = "5+no";
		ValueExprTest[8] = "5*no";
		ValueExprTest[9] = "5.232E-2*no";
		ValueExprTest[10] = "5.344*no";
		ValueExprTest[11] = "+5";
		ValueExprTest[12] = "-5.343E-11";
		ValueExprTest[13] = "count(*)";
		ValueExprTest[14] = "min(distinct 5)";
		ValueExprTest[15] = "min(no)";
		ValueExprTest[16] = "min(all no)";
		ValueExprTest[17] = "nullif(no,5)";
		ValueExprTest[18] = "5+nullif(no,5)";
		ValueExprTest[19] = "coalesce(5, no, null)";
		ValueExprTest[20] = "case no when 5 then 8+no when 3 then 5+no else 5.34";
		ValueExprTest[21] = "case no when 5 then 8+no when 3 then 5+no else 5.34 +27";
		ValueExprTest[22] = "16 * case no when 5 then 8+no when 3 then 5+no else 5.34";
		ValueExprTest[23] = "case when 5<no  then no*2 when 5>no then no*4 else no*6.5 end";
		ValueExprTest[24] = "cast (einstring as int) +5";
		ValueExprTest[25] = "nullif(no,5)";
		ValueExprTest[26] = "5+nullif(no,5)";
		ValueExprTest[27] = "coalesce(5, no, null)";
		ValueExprTest[28] = "case no when 5 then 8+no when 3 then 5+no else 5.34";
		ValueExprTest[29] = "case no when 5 then 8+no when 3 then 5+no else 5.34 +27";
		ValueExprTest[30] = "16 * case no when 5 then 8+no when 3 then 5+no else 5.34";
		ValueExprTest[31] = "case when 5<no  then no*2 when 5>no then no*4 else no*6.5 end";
		ValueExprTest[32] = "cast (no as string)";
		ValueExprTest[33] = "cast (einstring as int) +5";*/
		ValueExprTest[0] = "no";
		ValueExprTest[1] = "no, 5+5";
		ValueExprTest[2] = "5*5 as myatt5";
		
		/*QueryExprTest = new String[NoQueryExpression];
		QueryExprTest[0] = "typetest mytable (Spalte1, Spalte2, Sp3) right outer join tentest cross join twentest";
		QueryExprTest[1] = "tentest cross join twentest";
		QueryExprTest[2] = "tentest cross join twentest union twentest";
		QueryExprTest[3] = "tentest cross join twentest except twentest";
		QueryExprTest[4] = "tentest cross join twentest except twentest corresponding by no";
		QueryExprTest[5] = "tentest cross join twentest except values (1), (5)";
		QueryExprTest[6] = "tentest left join twentest union select 8 from tentest";
		QueryExprTest[7] = "tentest cross join twentest intersect all corresponding by table tentest";
		*/
		
		TableRefTest = new String[NoTableReference+2*NoQueryExpression];
		/*for (int i = 0; i < QueryExprTest.length; i++) {
			TableRefTest[i]= QueryExprTest[i] + " as myquery (Sp1, Sp2, Sp3)";
			TableRefTest[i+NoQueryExpression] = QueryExprTest[i] + " myquery (Sp1, Sp2, Sp3)";
		}*/
		TableRefTest[2*NoQueryExpression+0] = "Tentest";
		TableRefTest[2*NoQueryExpression+1] = "tentest myTable";
		TableRefTest[2*NoQueryExpression+2] = "tentest  as mytable";
		/*TableRefTest[2*NoQueryExpression+3] = "Tentest as mytable (Spalte1)";
		TableRefTest[2*NoQueryExpression+4] = "typetest mytable (Spalte1, Spalte2, Sp3)";
		TableRefTest[2*NoQueryExpression+5] = "tentest cross join twentest";
		TableRefTest[2*NoQueryExpression+6] = "tentest left join twentest";
		TableRefTest[2*NoQueryExpression+7] = "typetest mytable (Spalte1, Spalte2, Sp3) right outer join tentest cross join twentest";
		TableRefTest[2*NoQueryExpression+8] = "Twentest myquery (Sp1, Sp2, Sp3) natural join Tentest as mytable (Spalte1)";
		TableRefTest[2*NoQueryExpression+9] = "tentest left join twentest union join twentest as myquery (Sp1, Sp2, Sp3)";
		TableRefTest[2*NoQueryExpression+10] = "typetest mytable (Spalte1, Spalte2, Sp3) right outer join tentest cross join twentest using (Erg1,Erg2,Erg3)";
		TableRefTest[2*NoQueryExpression+11] = "typetest mytable (Spalte1, Spalte2, Sp3) right outer join tentest cross join twentest on no >5";
		*/TableRefTest[2*NoQueryExpression+3] = "Tentest, twentest";
		TableRefTest[2*NoQueryExpression+4] = "tentest myTable, twentest as myTable2";
		TableRefTest[2*NoQueryExpression+5] = "tentest as mytable, twentest";
		
		SearchCondTest = new String[NoSearchCond];
		SearchCondTest[0] ="5>no HAVING no >6";
		SearchCondTest[1] ="5>tentest.no";
		SearchCondTest[2] ="no<8";
		SearchCondTest[3] ="no=10";
		SearchCondTest[4] ="no<=5";
		SearchCondTest[5] ="no>=6";
		SearchCondTest[6] ="no not between 3 AND 8";
		SearchCondTest[7] ="exists (select * from twentest where tentest.no = twentest.no)";
		SearchCondTest[8] ="exists (select no from twentytest where tentest.no = twentytest.no)";
		SearchCondTest[9] ="(select * from tentest where no = 5) match full (select * from twentest)";
		SearchCondTest[10] ="no>10 is false";
		SearchCondTest[11] ="not no > 8";
		SearchCondTest[12] ="no >3 and no <5";
		SearchCondTest[13] ="no<5 or no >8";
		SearchCondTest[14] ="(2, 'Ein String', 5) = (select * from typetest where tstring = 'Ein String')";
		SearchCondTest[15] ="(2, 'Ein String', null) = (select * from typetest where tstring = 'Ein String')";
		SearchCondTest[16] = "5*no in  (select tint from typetest)";
		SearchCondTest[17] = "5*no in (select no from tentest)";
		SearchCondTest[18] = "no > any (select tint from typetest)";
		SearchCondTest[19] ="no<5 or no >8 and no>6";
		SearchCondTest[20] ="no<5 and no >8 or no>6";
		SearchCondTest[21] ="tstring = 'Hallo'";
		SearchCondTest[22] ="(no<5 or no >8) AND no>6";
		SearchCondTest[23] ="(tentest.no<5 OR no >8) AND no>6";
		SearchCondTest[24] ="(tentest.no<5 OR tentest.no >8) AND tentest.no>6";
		
		
		
	}
	
	private void testGroupby() {
		this.EingabeGroupBy = new String[this.NoGroupBy];
		
		this.EingabeGroupBy[0] = "SELECT min(no) FROM tentest";
		this.EingabeGroupBy[1] = "SELECT min(tint), treal FROM typetest";
		this.EingabeGroupBy[2] = "SELECT min(no) as minno FROM tentest";
		this.EingabeGroupBy[3] = "SELECT min(treal), tint FROM typetest GROUP BY tint";
		this.EingabeGroupBy[4] = "SELECT min(treal) as minreal, tint FROM typetest GROUP BY tint";
		this.EingabeGroupBy[5] = "SELECT min(no) FROM tentest GROUP BY no";
		this.EingabeGroupBy[6] = "SELECT min(treal) FROM typetest GROUP BY tint";
		this.EingabeGroupBy[7] = "SELECT min(treal) as minreal FROM typetest GROUP BY tint";
		this.EingabeGroupBy[8] = "SELECT min(treal), tint FROM typetest as ty GROUP BY tint";
		this.EingabeGroupBy[9] = "SELECT min(treal), tint FROM typetest GROUP BY typetest.tint";
		this.EingabeGroupBy[10] = "SELECT min(typetest.treal), tint FROM typetest GROUP BY tint";		
		
	}
		
	private void testOrderby() {
		this.EingabeOrderBy = new String[this.NoOrderBy];
		
		this.EingabeOrderBy[0] = "SELECT * FROM typetest ORDER BY tint";
		this.EingabeOrderBy[1] = "SELECT * FROM typetest ORDER BY tint, treal";
		this.EingabeOrderBy[2] = "SELECT * FROM typetest ORDER BY tint asc, treal desc";
		this.EingabeOrderBy[3] = "SELECT * FROM typetest where tint<9 ORDER BY tint";
		this.EingabeOrderBy[4] = "SELECT tint, treal FROM typetest where typetest.tint<9 ORDER BY tint";
		this.EingabeOrderBy[5] = "SELECT tint, treal FROM typetest where typetest.tint<9 ORDER BY tint, treal desc";
		this.EingabeOrderBy[6] = "SELECT * FROM typetest ORDER BY 2";
		this.EingabeOrderBy[7] = "SELECT * FROM typetest where typetest.tint<9 ORDER BY tint";
				
	}
	
	private void checkSelect() {
		int CounterAusgabe = 0;
		int NoAusgabe;
		
		
		
		//NoAusgabe = (this.NoGroupBy)+DistinctTest.length * ValueExprTest.length * (NoTableReference+2*NoQueryExpression)*NoSearchCond;
		NoAusgabe = this.NoOrderBy;
		Ausgabe = new String[NoAusgabe];
		
		/* Test Order_By */
		this.testOrderby();
		for (int i = 0; i < this.EingabeOrderBy.length; i++)
			Ausgabe[CounterAusgabe++] = this.EingabeOrderBy[i];
		
		
		/* Test Group_By
		this.testGroupby();		
		for (int m = 0; m < this.EingabeGroupBy.length; m++)
			Ausgabe[CounterAusgabe++] = this.EingabeGroupBy[m]; */
		
		/* Erster Gesamttest
		for (int i = 0; i < DistinctTest.length; i++) 
			for (int j = 0; j < ValueExprTest.length; j++) 
				for (int k = 0; k < TableRefTest.length; k++) 
					for (int l = 0; l < SearchCondTest.length; l++) {
					//for (int l = 3; l <=3; l++) {
						Ausgabe[CounterAusgabe] = "select " + DistinctTest[i] + ValueExprTest[j] 
						  + " from " + TableRefTest[k] + " where " + SearchCondTest[l];
						CounterAusgabe++;
					} */
	}
	
	public checker() {
		CounterOutput = 0;
		testCreate();
		checkSelect();
	}
	
	public String OutputSelect() {
		String Rueckgabe;
		if (CounterOutput < this.Ausgabe.length)
		//if (CounterOutput < 1)
			Rueckgabe = this.Ausgabe[CounterOutput];
			//Rueckgabe = "select distinct 5+5 from typetest mytable (Spalte1, Spalte2, Sp3) right outer join tentest cross join twentest on no >5";
		else
			Rueckgabe = "|ENDE|";		
		CounterOutput++;
		return Rueckgabe;
	}
	
	public String OutputSingle() {
		String result;
		
		if (this.CounterOutput < 1)
			//result = "ALTER TABLE tentest ADD COLUMN names CHARACTER(50)";
			//result = "CREATE TABLE myTable (Name CHARACTER(50))";
			//result = "CREATE TABLE myTable (Name CHARACTER(50), AlterInJahren DECIMAL(10))";
			//result = "SELECT tint, treal FROM typetest where typetest.tint<9 ORDER BY tint";
			//result = "CREATE TABLE myTable (AlterInJahren DECIMAL(10))";
			//result = "CREATE TABLE myTable (Name CHARACTER(50) PRIMARY KEY)";
			//result = "CREATE TABLE myTable (Name CHARACTER(50) CONSTRAINT myrule UNIQUE)";
			//result = "CREATE TABLE myTable (Name CHARACTER(50) CONSTRAINT myrule UNIQUE REFERENCES mytabel(Sp1, Sp2))";
			//result = "CREATE TABLE myTable (CONSTRAINT myTableRule FOREIGN KEY (Sp3, Sp4) REFERENCES mytable2(Sp8, Sp9), Name CHARACTER(50) CONSTRAINT myrule UNIQUE REFERENCES mytabel(Sp1, Sp2), Alter DECIMAL(10))";
			//result = "CREATE TABLE myTable (CONSTRAINT myTableRule FOREIGN KEY (Sp3, Sp4) REFERENCES mytable2(Sp8, Sp9), Name CHARACTER(50), Alter DECIMAL(10))";
			//result = "CREATE TABLE myTable (CONSTRAINT myTableRule FOREIGN KEY (Sp3, Sp4) REFERENCES mytable2(Sp8, Sp9), Name CHARACTER(50))";
			//result = "CREATE TABLE myTable (Name CHARACTER(50) CONSTRAINT myrule UNIQUE REFERENCES mytabel(Sp1, Sp2), Alter DECIMAL(10))";
			//result = "CREATE TABLE myTable (Name CHARACTER(50) UNIQUE REFERENCES mytabel(Sp1, Sp2))";
			//result = "CREATE TABLE myTable (Name CHAR(50) REFERENCES myothertable(Sp1, Sp2))";
			//result = "CREATE TABLE myTable (Name CHAR(50) PRIMARY KEY, Alter DECIMAL(10))";
			//result = "INSERT INTO typetest values ('eins', 1.1, 1 , date '2012-04-02')";
			//result = "INSERT INTO typetest values ('eins', 1.1, 1 , true)";
			//result = "INSERT INTO typetest2 SELECT Tstring, treal, tint FROM typetest WHERE tint = 2";
			//result = "DROP TABLE dir";
			//result = "SELECT * FROM typetest WHERE tint IS NOT NULL";
			result = "SELECT Employees.First_Name, Employees.Last_Name, Cars.Make, Cars.Model, Cars.Year FROM Employees, Cars WHERE Employees.Car_Number = Cars.Car_Number";
			//result = "SELECT no FROM tentest WHERE exists (select no from twentytest where tentest.no = twentytest.no)";
			//result = "SELECT Employees.First_Name, Employees.Last_Name, Employees.Car_Number, Cars.Make, Cars.Model, Cars.Year, Cars.Car_Number FROM Employees, Cars WHERE Employees.Car_Number = Cars.Car_Number";
			//result ="select distinct no from Tentest where 5*no in (select no from tentest)";
		else
			result = "|ENDE|";
		this.CounterOutput++;
		return result;
	}
	
}
