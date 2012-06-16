package LocalTest;

/**
 * 
 * <b>Task of this interface: </b> <br/>
 * Declares constants
 *
 */

public interface Declarations {

	
	public static final String IP_ADRESS = "192.168.106.136"; //Notebook library
	//public static final String IP_ADRESS = "192.168.30.129";  //Notebook home
	//public static final String IP_ADRESS = "192.168.181.128";  //Computer home
	public static final int OPT_PORT = 1235;
	public static final int SEC_PORT = 1234;
	/* Here we find Testqueries in order to find out the differences between 
	 * Secondo SQL and the original SQL. TQ stands for Testqueriy
	 */
	
	public static final String TQSelect1="select * from staedtetest where plz < 3000";
	public static final String TQCreateTable1="create table supplier columns [sno : string, sname : string, status : int, city : string]";
	public static final String TQCreateTable2="sql create table products columns [pno : string, pname : string, color : string, weight : int, city : string]";
	public static final String TQCreateTable3="sql create table supplprod columns [sno : string, pno : string, qty : int]";
	public static final String TQInsert1="sql insert into products values [\"P2\", \"Bolt\", \"Green\", 17, \"Paris\"]";
	public static final String TQInsert2="sql insert into products values [\"P3\", \"Screw\", \"Blue\", 17, \"Rome\"]";
	public static final String TQInsert3="sql insert into products values [\"P6\", \"Cog\", \"Red\", 19, \"London\"]";
	public static final String TQUpdate = "update supplier set status = 2*status where city = \"London\"";
	public static final String TQUpdateTest1 = "update supplier set status = 2 where city = \"London\"";
	public static final String TQUpdate2 = "sql update products set color = \"Green\" where color = \"Blue\" and pname = \"Bolt\"";
	public static final String TQDelete = "sql delete from products where pno = \"P6\"";
	public static final String TQSelect2="select distinct [p:color, p:city] from products as p where [p:weight > 10, p:city # \"Paris\"]";
	public static final String TQSelect3="select distinct [p:color, p:city] from products as p where p:weight > 15 or p:city = \"Paris\"";
	public static final String TQSelect4="select distinct [sp:pno, s:city] from [supplier as s, supplprod as sp] " + 
	                                     "where sp:sno = s:sno";
	public static final String TQSelect5="select count(distinct sp:sno) as anz from supplprod as sp"; 
	public static final String TQSelect6="select [sp:pno as n2, sum(sp:qty) as tot2] from supplprod as sp groupby sp:pno";
		
	public static final String TQUpdateTest = "sql update tentest set no = 2*no where no > 8";
	
	public static final String Testquery = "select * from tentest";
	public static final String TestDB = "testqueries";

}
