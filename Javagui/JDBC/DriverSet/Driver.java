package DriverSet;

import java.sql.Connection;
import java.sql.DriverPropertyInfo;
import java.sql.SQLException;
import java.util.Properties;

import communication.CommunicationInterface;


/**
 * <b> Task of this class </b> <br/>
 * Driver class to establish Connection to Secondo
 */
public class Driver implements java.sql.Driver {

	// This nameless method it to register the driver with the DriverManager
	//
	static {
		try {
			java.sql.DriverManager.registerDriver(new Driver());
		}
		catch (SQLException E) {
			throw new RuntimeException("Can't register driver!");
		}
	}

	/**
	 * <b>Task of this variable</b> <br/>
	 * The connection to the communication packet is made here.
	 */
	private CommunicationInterface connWithDatabase;
	private String URL;
	private String Host;
	private int SPort;
	private int OPort;
	
	private String username;
	private String password;
	
	
	
	public boolean acceptsURL(String url) throws SQLException {
		// checks whether the driver class can use the url needed to 
		//establish connection to the database Secondo
		String LowURL = url.toLowerCase();
		return LowURL.startsWith("jdbc:secondo:");
	}

	public Connection connect(String url, Properties info) throws SQLException {
		/**
		 * <b>Task of this method</b><br/>
		 * establishes connection to database <br/>
		 * Hostname and Portnumbers can be transferred via the URL <br/>
		 * Username and password are transferred by info <br/>
		 * A user control management is not supported sufficiently by secondo
		 */
		// URL looks like this: jdbc:secondo://Server:SecondoPort:OptimizerPort/"DBName"
		// Assuming that the Secondo Server and the Optimizer Server are accessed by the same IP address 
		
		/*Host = Declarations.IP_ADRESS;
		String SecondoPort = "1234";
		String OptimizerPort = "1235";
		SPort = Declarations.SEC_PORT;
		OPort = Declarations.OPT_PORT;*/
		URL = url;
		String DBName = getDBName();
		if (DBName.equals("")) throw new SQLException("No valid url given", "08001"); 
		ConnectionImpl con;
		
		if (info != null) {
			// username and password in not supported sufficiently by secondo
			// in case it will be implemented at some stage in secondo
			// username and password are stored here.
			this.username = info.getProperty("user");
			this.password = info.getProperty("password");
		}
		connWithDatabase = new CommunicationInterface();
		connWithDatabase.initialize(Host, SPort, OPort);
		if (!connWithDatabase.connectToDB(DBName))
			throw new RuntimeException("Connection to datasource cannot be established!");
		
		con = new ConnectionImpl(connWithDatabase);
				
		return con;
		
		
		
		
		
	}

	public int getMajorVersion() {
		// returns the major versionnumber of the driver
		return 1;
	}

	public int getMinorVersion() {
		// returns the under versionnumber of the driver
		return 0;
	}

	/**
	 <b>Task of this method</b><br/>
	 * Provides information to a gui-tool so that it can ask a user for parameters 
	 * necessary to start the database 
	 * In case username and password are required the information should be given here
	 * url needs to be interpreted in case different information are needed for different
	 * databases. info is needed to give the user a preselection of information he might want 
	 * to ask for.
	 * At this current stage no additional information are needed to start the database
	 * 
	 */
	public DriverPropertyInfo[] getPropertyInfo(String url, Properties info)
			throws SQLException {
		
		DriverPropertyInfo[] DPI = new DriverPropertyInfo[1];
		DPI[0] = new DriverPropertyInfo(null,null);
		
		return DPI;
	}

	
	// Secondo is not fully jdbc compliant
	public boolean jdbcCompliant() {
		return false;
	}
	
	private String getDBName() throws SQLException {
		/**
		 * <b> Task of this method:</b> <br/>
		 * It extracts the database name of the url <br/>
		 * and checks whether the url is valid <br/>
		 * returns "" if not.
		 */
		String result;
		String part1;
		String part2;
		String part3;
		int Separator1;
		int Separator2;
		int Separator3;
		int Separator4;
		int Separator5;
		
		Separator1 = URL.indexOf(":");
		part1 = URL.substring(0, Separator1);
		Separator2 = URL.indexOf(":", Separator1 +1);
		part2 = URL.substring(Separator1+1, Separator2);
		part3 = URL.substring(Separator2+1);
		
		part1 = part1.trim();
		part2 = part2.trim();
		part3 = part3.trim();
		//Teil3 = Teil3.toLowerCase();
		
		if (!part1.equalsIgnoreCase("jdbc") || !part2.equalsIgnoreCase("secondo"))
			throw new SQLException("Invalid URL", "08001");
		else if (!part3.startsWith("//"))	
			throw new SQLException("Invalid URL", "08001");
		else {
			part3 = part3.substring(2);
			Separator3 = part3.indexOf(":");
			if (Separator3 == -1) throw new SQLException("Invalid URL", "08001");
			Host = part3.substring(0, Separator3);
			part3 = part3.substring(Separator3+1);
			Separator4 = part3.indexOf(":");
			if (Separator4 == -1) throw new SQLException("Invalid URL", "08001");
			SPort = new Integer(part3.substring(0, Separator4)).intValue();
			part3 = part3.substring(Separator4+1);
			Separator5 = part3.indexOf("/");
			if (Separator5 == -1) throw new SQLException("Invalid URL", "08001");
			OPort = new Integer(part3.substring(0, Separator5)).intValue();
			result = part3.substring(Separator5+1).toLowerCase().trim();
		}
			
		
		return result;
	}
	
}
