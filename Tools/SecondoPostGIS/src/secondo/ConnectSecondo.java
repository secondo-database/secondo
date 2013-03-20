package secondo;

import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;

public class ConnectSecondo {

	private StringBuffer msbhostName;//="192.168.142.128";
	private int miport;
	private StringBuffer msbUser;
	private StringBuffer msbPwd;
	private boolean mbUseBinaryList;
	
	
	private static ESInterface si = null;
	private static IntByReference errorCode= new IntByReference();
	private static IntByReference errorPos = new IntByReference();
	private static ListExpr resultList= new ListExpr();
	private static StringBuffer errorMessage = new StringBuffer();
	//private volatile boolean mhaltOnError = true;
	
	//ggf die Datensätze anders Preparieren
	
	/**
	 * @param msbhostName
	 * @param miport
	 * @param msbUser
	 * @param msbPwd
	 * @param mbUseBinaryList
	 */
	public ConnectSecondo(StringBuffer _msbhostName, int _miport,
			StringBuffer _msbUser, StringBuffer _msbPwd, boolean _mbUseBinaryList) {
		super();
	
		this.msbhostName = new StringBuffer();
		this.miport = 0;
		this.msbUser = new StringBuffer();
		this.msbPwd = new StringBuffer();
		this.mbUseBinaryList = true;
		
		this.msbhostName = _msbhostName;
		this.miport = _miport;
		this.msbUser = _msbUser;
		this.msbPwd = _msbPwd;
		this.mbUseBinaryList = _mbUseBinaryList;
		
		this.si = new ESInterface();
	}
	
	
	
	public boolean connect()
	{
		//Hier die initalizie Methode, dann kann ich das auch bei close prüfen
		
		//si.initialize("1", "2", this.msbhostName.toString(), String.valueOf(this.miport));
		
		si.setHostname(this.msbhostName.toString());
		si.setPort(this.miport);

		si.setUserName(this.msbUser.toString());
		si.setPassWd(this.msbPwd.toString());

		si.useBinaryLists(this.mbUseBinaryList); 
		
		tools.Environment.MEASURE_TIME=false; // supress some messages
		
	
		
		if(!si.connect())
		{
		     System.err.println("problem in connecting with a secondo server");
		     
		     return false;
		}

		//System.out.println(resultList.listLength());
		//System.out.println(sendCommand("create database mydb"));
		System.out.println(resultList.listLength());
		//System.out.println(sendCommand("delete database ctest"));
		//System.out.println(sendCommand("delete testrel"));
		
		System.out.println(sendCommand("list databases"));
		System.out.println(resultList.listLength());
		
		StringBuffer test = new StringBuffer();
		resultList.writeToString(test);
		System.out.println(test);
		
		ListExpr resultList1 = resultList.first();
		resultList1.writeToString(test);
		System.out.println(test);
		
		
		resultList1 = resultList.second();
		resultList1.writeToString(test);
		System.out.println(test);
		
		resultList1 = resultList1.second();
		resultList1.writeToString(test);
		System.out.println(test);
		
		resultList1 = resultList1.second();
		resultList1.writeToString(test);
		System.out.println(test);
		
		
		//System.out.println(resultList1.listLength());
		
		
		
		
		/*resultList = resultList.second();
		resultList = resultList.first();
		System.out.println(resultList.symbolValue());
		*/
		/*
		if(resultList.listLength() != 2)
		{
			if(resultList.first().atomType()!= ListExpr.SYMBOL_ATOM || !re)
			
		}
		*/
		//System.out.println(resultList.);
		/*
			  haltOnError=false;
			  sendCommand("create database ctest");
			  sendCommand("open database ctest");
			  sendCommand("delete testrel");
			  haltOnError=true;

			  for(int i=0; i<10 ; i++){
			    sendCommand("let i"+i+" = " + i);
			  }


	*/		 
	
	  System.out.println("successful");
	  return true;
		
	}
	
	/**
	 * Verbindung wieder schließen
	 */
	public void closeConnection()
	{
		if(si != null && si.isConnected()==true)
		{
			//System.out.println("Trennen");
			si.terminate();
		}
			
	}
	
	
	/**
	 * Die Ergebnisse stehen dann in den Elementen drin die übergeben wurden
	 */
	public boolean sendCommand(StringBuffer _sbCommand)
	{
		si.secondo(_sbCommand.toString() ,resultList,errorCode,errorPos,errorMessage);
		
		
		if(errorCode.value != 0)
		  {
		     System.err.println("error in command " + _sbCommand.toString());
		     System.err.println(errorMessage.toString());
		     
		     //if(mhaltOnError)
		     {
		         //si.terminate();
		        // System.exit(2);
		     }
		     return false;
		  }
		  return true;
		
		
	}
	
	
	public boolean sendCommand(String strCommand)
	{
		return sendCommand(new StringBuffer(strCommand));
	}

	
	
	
	public StringBuffer getMsbhostName() {
		return msbhostName;
	}
	public void setMsbhostName(StringBuffer msbhostName) {
		this.msbhostName = msbhostName;
	}
	public int getMiport() {
		return miport;
	}
	public void setMiport(int miport) {
		this.miport = miport;
	}
	public StringBuffer getMsbUser() {
		return msbUser;
	}
	public void setMsbUser(StringBuffer msbUser) {
		this.msbUser = msbUser;
	}
	public StringBuffer getMsbPwd() {
		return msbPwd;
	}
	public void setMsbPwd(StringBuffer msbPwd) {
		this.msbPwd = msbPwd;
	}
	public boolean isMbUseBinaryList() {
		return mbUseBinaryList;
	}
	public void setMbUseBinaryList(boolean mbUseBinaryList) {
		this.mbUseBinaryList = mbUseBinaryList;
	}

	
	
	
	
	
	
	
	
	
	
	
}
