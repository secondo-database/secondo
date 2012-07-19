import java.sql.Connection;
import java.sql.DriverManager;
//import java.util.Properties;
import LocalTest.Declarations;

import java.io.*;


	
public class OpenDB {
	private Connection con;
	//private Properties Uebergabeparameter; 
	private String parametersToConnect, DriverName;
	private int SepDriverName;
	
	public Connection getCon() {
		return con;
	}
	
	public OpenDB() {
		parametersToConnect = openConfigFile();
	}
	
	public void openCon() throws Exception {
		
		SepDriverName = parametersToConnect.indexOf('|');
		DriverName = parametersToConnect.substring(0, SepDriverName);
		parametersToConnect = parametersToConnect.substring(SepDriverName+1);
		//parametersToConnect = "//"+Declarations.IP_ADRESS+":"+Declarations.SEC_PORT+":"+Declarations.OPT_PORT+"/"+"Testqueries";
		Class.forName(DriverName);
		con=DriverManager.getConnection(parametersToConnect);
		//con=DriverManager.getConnection("jdbc:secondo:" + parametersToConnect, "myUser", "myPassword");
		
	}
	
	private String openConfigFile() {
		File configFile;
		final int NoLines = 6;
		String[] configOutput;
		BufferedReader f;
		String line;
		int i = 0;
		
			
		configOutput = new String[NoLines];
		try {
			configFile = new File(OpenDB.class.getProtectionDomain().getCodeSource().getLocation().toURI().getPath()+"secondojdbc.cfg");
			f = new BufferedReader(new FileReader(configFile));
			while (i<NoLines && (line = f.readLine()) != null) {
				configOutput[i] = line;
				i++;
			}
			f.close();
		} catch (Exception e) {
			System.out.println("Error reading file");
		}
		
		configOutput[0] = configOutput[0].substring(13);
		configOutput[1] = configOutput[1].substring(10);
		configOutput[2] = configOutput[2].substring(12);
		configOutput[3] = configOutput[3].substring(14);
		configOutput[4] = configOutput[4].substring(16);
		configOutput[5] = configOutput[5].substring(10);
		
		return configOutput[0]+"|"+configOutput[1]+"//"+configOutput[2]+":"+configOutput[3]+":"+configOutput[4]+"/"+configOutput[5];
	}
	
}
