import java.sql.Connection;
import java.sql.DriverManager;
//import java.util.Properties;
//import Verbindungstest.Declarations;


	
public class OpenDB {
	private Connection con;
	//private Properties Uebergabeparameter; 
	private String Uebergabeparameter;
	
	// Für Secondo 2 und für Access 1
	private int Art;
	
	public Connection getCon() {
		return con;
	}
	
	public OpenDB(String DBArt) {
		setDBArt(DBArt);
	}
	
	public void setDBArt(String DBArt) {
		if (DBArt.equalsIgnoreCase("Access"))
			Art = 1;
		else if (DBArt.equalsIgnoreCase("Secondo"))
			Art = 2;
		else
			Art = 0;// ungültig
	}
	
	public void openCon() throws Exception {
		if (Art == 1) {
			Class.forName("sun.jdbc.odbc.JdbcOdbcDriver");
			con=DriverManager.getConnection("jdbc:odbc:TJDBC");
		}
		else if (Art == 2){
			Class.forName("DriverSet.Driver");
			/*Uebergabeparameter = new Properties();
			Uebergabeparameter.put("Host", "192.168.106.128");
			Uebergabeparameter.put("Port", "1235");*/
			Uebergabeparameter = "//127.0.0.1"+":1234:1235/Testqueries";
			con=DriverManager.getConnection("jdbc:secondo:" + Uebergabeparameter);
		}
		else
			; //ToDo Hier muss eine Exception erzeugt werden
		
	}
	
}
