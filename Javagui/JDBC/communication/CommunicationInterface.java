package communication;
import communication.optimizer.OptimizerInterface;
import communication.optimizer.IntObj;
import java.sql.SQLException;

import tools.Reporter;
//import Verbindungstest.Declarations;
import sj.lang.ESInterface;
import sj.lang.ListExpr;
import sj.lang.IntByReference;
/**
 * 
 * <b> Task of this class </b> <br/>
 *It handles all the communication between secondo and any other packet. <br/>
 *Since most of the classes within communication are protected it makes sense to 
 *have such a class
 */
public class CommunicationInterface {
	
	private final String AlterTempTabName = "typeTesttmp";   //AlterTabTmp
	private OptimizerInterface OI;
	private ESInterface SI;
	private ListExpr LEresult;
	private IntByReference ErrCode;
	private IntByReference ErrPos;
	private StringBuffer ErrMess;
	private boolean connectedToDB;
	private String connectedDB;
	private IntObj OptIO; //needed for the Optimizer, it contains the errorcode
	
	private int SecPort;
	private int OptPort;
	private String HostName;
	
	private String AlterCommand2, AlterCommand3, AlterCommand4;	//Additional commands for ALTER TABLE
	
	public CommunicationInterface() {
		ErrCode = new IntByReference();
		ErrPos = new IntByReference();
		ErrMess = new StringBuffer();
				
		LEresult = new ListExpr();
		OI = new OptimizerInterface();
		SI = new ESInterface();
		OptIO = new IntObj();
		connectedToDB = false;
		
	}
	
	
	/**
	 * <b> Task of this method </b> <br/>
	 * initializes connection to Secondo server and Secondo optimizer 
	 * @return true if connection has been established
	 */
	public boolean initialize(String HName, int SPort, int OPort) {
		
		boolean IstOk = false;
		HostName = HName;
		SecPort = SPort;
		OptPort = OPort;
		
		if (SI.initialize("", "", HostName, SecPort)) {
			Reporter.writeInfo("connected to Secondo server");
			SI.useBinaryLists(true);
			IstOk = true;
		}
		else 
			Reporter.writeError("ERROR: Connection to Secondo server could not be established");
		
		OI.setHost(HostName);
		OI.setPort(OptPort);
		if (OI.connect()) 
			Reporter.writeInfo("Connected to Secondo optimizer");
		else {
			Reporter.writeError("ERROR: Connection to Secondo optimizer could not be established");
			IstOk = false;
		}
		return IstOk;	
	}
	
	/**
	 * <b> Task of this method </b> <br/>
	 * It establishes a connection to a given database
	 * @param DBName Name of the database
	 * @return true if connection has been established
	 */
	public boolean connectToDB(String DBName) throws SQLException {
		if (!connectedToDB) {
			//Reporter.writeInfo("Connect to DB " + DBName);
			SI.secondo("open database "+ DBName +";", LEresult, ErrCode, ErrPos, ErrMess);
			/* System.out.println(Ergebnis.toString());
			   System.out.println(ErrMess.toString()); */
			if (ErrMess.toString().equals("")) {
				connectedToDB = true;
				connectedDB = DBName;
			}
			else {
				SQLException ex = new SQLException("Connection to database could not be established", "08001");
				SQLException next = new SQLException(ErrMess.toString());
				ex.setNextException(next);
				throw ex;
			}
		}
		else
			Reporter.writeWarning("Database is already connected to " + connectedDB);
		return connectedToDB;
		
	}
	
	public boolean isConnected() {
		
		if (!SI.isInitialized()) {
			this.connectedDB = "";
			this.connectedToDB = false;
		}
		
		return this.connectedToDB;
	}
	
	/**
	 * <b> Task of this method </b> <br/>
	 * The connection to the Optimizer Server and to the Secondo Server is terminated <br/>
	 * It reports an error if the connection has not been established
	 */
	public void closeDB() {
		if (connectedToDB) {
			OI.disconnect();
			SI.terminate();
			connectedToDB = false;
			connectedDB = "";
		}
		else
			Reporter.writeError("DB was already disconnected");
	}
	
	/**
	 * <b> Task of this method </b> <br/>
	 * getter for result of a query or an update
	 * @return Ergebnis usually used after having executed a query or update
	 */
	public ListExpr getResult() {
		return this.LEresult;
	}
	
	
	/**
	 * <b> Task of this method </b> <br/>
	 * it gives a test query to the optimizer and then transfers the optimizers <br/>
	 * answer to the secondo server. It just works if the database testqueries has been opened. 
	 * @return true if the result of the test was as expected
	 */
	
	/*public boolean Testit() {
		boolean IstOk = false;
		if ((connectedToDB) && (connectedDB.equalsIgnoreCase("testqueries"))) {
			String Ausgabe = OI.optimize_execute(Declarations.Testquery, Declarations.TestDB, OptIO, false);
			// false means the optimizer is evaluating a query
			SI.secondo("query "+Ausgabe, Ergebnis, ErrCode, ErrPos, ErrMess);
			if (Ergebnis.toString().contains("(1)\n        (2)\n        (3)\n        (4)"))
				IstOk = true;
		}
		
		return IstOk;
	}*/
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * It sends the command to the optimizer and in case of a query <br/>
	 * or an update sends the execution plan to the secondo server.
	 * @param command a string which contains the command to be executed 
	 * by the secondo server
	 * @return true if the execution worked
	 */
	public boolean executeCommand(String command) {
		boolean IstOk = false;
		boolean IsUpdate = false;
		boolean AlterTCommand = false;
		String result; 
		/* IsUpdate is always false. If changed to true it is expected
		 * than a sql mquerie command is given. In this case the command 
		 * would be processed by the Optimizer directly
		 * if (command.startsWith("select")) 
			IsUpdate = false;
		else
			IsUpdate = true; */ 
		
		LEresult = new ListExpr();  // after executeCommand has been invoked for the second time
									// it still has the result from the first call. Therfore it is
									// reinitialized here
		
		if(command.startsWith("altertable")) {
			AlterTCommand = true;
			result = this.getAlterCommands(command);			
		}
		else	
			result = OI.optimize_execute(command, connectedDB, OptIO, IsUpdate);
		
		if (result.startsWith("::ERROR"))
			Reporter.writeError(result);
		else if (result.equals("done"))
			// DDL-Command like CREATE TABLE
			IstOk = true;
		else {	
			if (!AlterTCommand)
				result="query "+result;
			SI.secondo(result, LEresult, ErrCode, ErrPos, ErrMess);
			if (ErrCode.value == 0) {
				if (AlterTCommand) {
					SI.secondo(AlterCommand2, LEresult, ErrCode, ErrPos, ErrMess);
					SI.secondo(AlterCommand3, LEresult, ErrCode, ErrPos, ErrMess);
					SI.secondo(AlterCommand4, LEresult, ErrCode, ErrPos, ErrMess);
					//OI.optimize_execute("updateCatalog", connectedDB, OptIO, true);
				}
				IstOk = true; //query was successful
				Reporter.reportInfo(LEresult.toString(), true);
			}
			else
				Reporter.writeError(ErrMess.toString());
		}
		
		return IstOk;
	}
	
	public void executeSettings(boolean UseSubqueries) {
		
		String Ausgabe="";
		String command="";
		
		if (UseSubqueries)
			command = "setOption(subqueries)";
		else
			command = "delOption(subqueries)";
		
		Ausgabe = OI.optimize_execute(command, connectedDB, OptIO, true);
	}
	
	/*test
	public void executeSettings1(String command) {
		
		String Ausgabe="";
		
		
		Ausgabe = OI.optimize_execute(command, connectedDB, OptIO, true);
	}*/
	
	public void executeSecSettings(String statm) {
		
		LEresult = new ListExpr();
		SI.secondo(statm, LEresult, ErrCode, ErrPos, ErrMess);
		
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * has been implemented for DatabaseMetaDataImpl
	 * @return connection parameter for the current database connection
	 */
	public String getUrl() {
		String result;
		
		if (this.connectedToDB) 
			result = "jdbc:secondo://"+this.HostName+":"+this.SecPort+":"+this.OptPort+"/"+this.connectedDB;
		else
			result = null;
		
		return result;
	}
	
	private String getAlterCommands(String OrgCommand) {
		String tabName, colName, commandTemp, PreCommand, result;
		String colType = "";
		int pos1, pos2;
		
		LEresult = new ListExpr();
		commandTemp = OrgCommand.substring(11);
		pos1 = commandTemp.indexOf(";");
		tabName = commandTemp.substring(0, pos1);
		commandTemp = commandTemp.substring(pos1+1);
		pos1 = commandTemp.indexOf(';');
		
		if (OrgCommand.contains(";add;")) {
			String FirstLetter;
			
			pos2 = commandTemp.substring(pos1+1).indexOf(';');
			FirstLetter = commandTemp.substring(pos1+1, pos1+2);  // First letter of a column needs to be a capital letter
			colName = FirstLetter.toUpperCase() + commandTemp.substring(pos1 + 2, pos1 + 1 + pos2);
			colType = commandTemp.substring(pos1+pos2+2);			
		} 
		else {		// ALTER TABLE DROP
			String PreResult;
			
			colName = commandTemp.substring(pos1 + 1);
			PreCommand = "select " + colName + " from " + tabName;
			PreResult = OI.optimize_execute(PreCommand, connectedDB, OptIO, false);
			pos1 = PreResult.indexOf('[');
			pos2 = PreResult.indexOf(']');
			colName = PreResult.substring(pos1+1, pos2);
		}
		
		PreCommand = "query getcatalog() filter[ tolower(\'\' + .ObjectName) = \""+tabName+"\"] filter[.TypeExpr startsWith \"(rel\"] extract[ObjectName]";
		SI.secondo(PreCommand, LEresult, ErrCode, ErrPos, ErrMess);
		
		tabName = LEresult.second().stringValue();
		
		
		
		// get real name of table (with correct upper case spelling)
		
		
		// create the 4 orders for the alter table command
		if (OrgCommand.contains(";add;"))
			result = "let " + AlterTempTabName + " = " + tabName + " feed extend[" +colName+ " : [const "+colType+" value undef]] consume";
		else
			result = "let " + AlterTempTabName + " = " + tabName + " feed remove[" +colName+ "] consume";
		AlterCommand2 = "delete " + tabName;
		AlterCommand3 = "let " + tabName + " = " + AlterTempTabName;
		AlterCommand4 = "delete " + AlterTempTabName;
		
		return result;
	}
	
	
}
