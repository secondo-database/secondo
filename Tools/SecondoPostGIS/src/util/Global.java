package util;

public class Global {

	public static StringBuffer gsbDir;				//Pfadtrenner je nach OS
	
	public static StringBuffer gsbHomeDir; 			//
	
	public static StringBuffer gsbConfigFile;
	
	public static StringBuffer gsbTableDelimiter;
	
	//Parameter für die configDatei:
	
	//PostGis
	public static StringBuffer gsbPG_Host;
	public static StringBuffer gsbPG_Port;
	public static StringBuffer gsbPG_User;
	public static StringBuffer gsbPG_Pwd;
	
	
	//Secondo
	public static StringBuffer gsbSEC_Host;
	public static StringBuffer gsbSEC_Port;
	public static StringBuffer gsbSEC_User;
	public static StringBuffer gsbSEC_Pwd;
	public static boolean gbSEC_UseBinaryList;
	
	
	
	public Global ()
	{
		//Init:
		gsbDir = new StringBuffer();
		gsbHomeDir = new StringBuffer();
		gsbConfigFile = new StringBuffer();
		gsbTableDelimiter = new StringBuffer();

		gsbPG_Host = new StringBuffer();
		gsbPG_Port = new StringBuffer();
		gsbPG_User = new StringBuffer();
		gsbPG_Pwd = new StringBuffer();

		
		gsbSEC_Host = new StringBuffer();
		gsbSEC_Port = new StringBuffer();
		gsbSEC_User = new StringBuffer();
		gsbSEC_Pwd = new StringBuffer();
		gbSEC_UseBinaryList = true;

		
		gsbTableDelimiter.append(" - ");
		
		//Zuweisen
		if(OSValidator.isUnixOrMac() == true)
			gsbDir.append("/");
		else
			gsbDir.append("\\");
		
		gsbHomeDir.append(System.getProperty("user.home"));
		gsbHomeDir.append(gsbDir);
		gsbHomeDir.append(".Hjort");
		gsbHomeDir.append(gsbDir);
		
		gsbConfigFile.append(gsbHomeDir);
		gsbConfigFile.append(".hjort.cfg");
	
	
	}
			
}
