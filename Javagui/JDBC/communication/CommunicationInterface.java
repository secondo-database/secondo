package communication;
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
	private ESInterface SI;
	private ListExpr LEresult;
	private IntByReference ErrCode;
	private IntByReference ErrPos;
	private StringBuffer ErrMess;
	private boolean connectedToDB;
	private String connectedDB;
	
	private int SecPort;
	private int OptPort;
	private String HostName;
	
	private String AlterCommand2, AlterCommand3, AlterCommand4;	//Additional commands for ALTER TABLE
	
	public CommunicationInterface() {
		ErrCode = new IntByReference();
		ErrPos = new IntByReference();
		ErrMess = new StringBuffer();
				
		LEresult = new ListExpr();
		SI = new ESInterface();
		connectedToDB = false;
		
	}
	
	
	/**
	 * <b> Task of this method </b> <br/>
	 * initializes the connection to the Secondo server. The optimizer runs
	 * inside that server, so there is no second connection to open; OPort is
	 * kept for the sake of existing jdbc:secondo:// URLs and is not used.
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
		
		if (IstOk && !SI.optimizerAvailable()) {
			Reporter.writeError("ERROR: the Secondo server provides no optimizer");
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
	 * The connection to the Secondo Server is terminated <br/>
	 * It reports an error if the connection has not been established
	 */
	public void closeDB() {
		if (connectedToDB) {
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
		
		LEresult = new ListExpr();  // after executeCommand has been invoked for the second time
									// it still has the result from the first call. Therfore it is
									// reinitialized here
		
		if(command.startsWith("altertable")) {
			// ALTER TABLE is rewritten into a sequence of kernel commands
			String result = this.getAlterCommands(command);
			if (result == null)
				return false;
			SI.secondo(result, LEresult, ErrCode, ErrPos, ErrMess);
			if (ErrCode.value != 0) {
				Reporter.writeError(ErrMess.toString());
				return false;
			}
			SI.secondo(AlterCommand2, LEresult, ErrCode, ErrPos, ErrMess);
			SI.secondo(AlterCommand3, LEresult, ErrCode, ErrPos, ErrMess);
			SI.secondo(AlterCommand4, LEresult, ErrCode, ErrPos, ErrMess);
			Reporter.reportInfo(LEresult.toString(), true);
			return true;
		}
		
		// SQL: the server optimizes the command and runs the plan in one step
		ListExpr answer = sqlCommand(command, false);
		if (answer == null)
			return false;
		if (planOf(answer).equals("done"))
			// DDL-Command like CREATE TABLE: the optimizer carried it out
			// itself while translating, so there is no plan to run
			return true;
		LEresult = answer.second();
		Reporter.reportInfo(LEresult.toString(), true);
		IstOk = true;
		
		return IstOk;
	}
	
	/**
	 * <b> Task of this method </b> <br/>
	 * Sends an SQL command to the Secondo server, which optimizes it itself
	 * (command level 2) and answers with the list (plan result costs).
	 * @param command the command in the SQL dialect
	 * @param planOnly if true the server stops after optimizing and executes
	 * nothing, so only the plan and its costs come back
	 * @return the answer list, or null if the command could not be optimized
	 */
	private ListExpr sqlCommand(String command, boolean planOnly) {
		ListExpr answer = new ListExpr();
		SI.secondo(command, answer, ErrCode, ErrPos, ErrMess, 2, planOnly);
		if (ErrCode.value != 0) {
			Reporter.writeError(ErrMess.toString());
			return null;
		}
		if (answer.listLength() < 2) {
			Reporter.writeError("unexpected answer from the optimizer: " + answer.toString());
			return null;
		}
		return answer;
	}
	
	/**
	 * <b> Task of this method </b> <br/>
	 * Returns the generated plan of an answer of sqlCommand. Note that this is
	 * a ready to run command, not a bare plan expression: the server already
	 * wrapped it into "query ..." resp. "let X = ...".
	 * @param answer an answer list of sqlCommand
	 * @return the plan text
	 */
	private String planOf(ListExpr answer) {
		ListExpr plan = answer.first();
		return plan.atomType() == ListExpr.TEXT_ATOM ? plan.textValue().trim()
		                                             : plan.toString().trim();
	}
	
	public void executeSettings(boolean UseSubqueries) {
		
		String Ausgabe="";
		String command="";
		
		if (UseSubqueries)
			command = "setOption(subqueries)";
		else
			command = "delOption(subqueries)";
		
		Ausgabe = SI.optimizerCommand(command);
		if (Ausgabe == null)
			Reporter.writeError("ERROR: " + command + " failed");
	}
	
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
			// only the plan is wanted here (it carries the attribute name with
			// its real spelling), so ask the server not to run the query
			ListExpr PreAnswer = sqlCommand(PreCommand, true);
			if (PreAnswer == null)
				return null;
			PreResult = planOf(PreAnswer);
			pos1 = PreResult.indexOf('[');
			pos2 = PreResult.indexOf(']');
			if (pos1 < 0 || pos2 < pos1) {
				Reporter.writeError("cannot read the attribute name from the plan: " + PreResult);
				return null;
			}
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
