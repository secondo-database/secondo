package DriverSet;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.SQLWarning;
import sj.lang.ListExpr;
import communication.CommunicationInterface;
import SQL2Secondo.*;
//import java.util.Vector;
import Ext_Tools.*;
import Utilities.WarningContainer;
import SecExceptions.NotSuppDriverException;
import SecExceptions.NotCompSQL92Exception;




/**
 * <b> Task of this class </b> <br/>
 * Implements the SQL Interface Statement. 
 * This class contains the methods execute...() which transfers SQL querys to the database<br/>
 * The connection device CommunicationInterface is not directly transfered <br/>
 * to this class. It can only be used over the connection-class which is <br/>
 * a parameter of the constructor.
 */

public class StatementImpl implements java.sql.Statement {
	
	private CommunicationInterface CI;
	private Translator Trans;
	private ListExpr Answer;
	private SQLWarning Warning;
	private ResultSet RS;
	private int CountResult;	//Result of executeUpdate
	private completeAnswer TrAnswer;
	//private Vector<Qualifier> ShList;
	//private boolean ShListExists;
	
	public StatementImpl(CommunicationInterface Verb) {
		this.CI = Verb;
		this.Trans = new Translator();
		this.Warning = WarningContainer.getInstance();
	}
	
	/**
	 * <b> Task of this method </b> <br/>
	 * Executes all commands to the database, no matter wheather it is <br/>
	 * a query an update or a create statement to be executed. The result <br/>
	 * is written in Answer which is of type ListExpr
	 * @param Command the string to be executed
	 * @return true if execution was successful
	 * @throws SQLException
	 */
	
	private boolean ExecuteAll(String Command) throws SQLException {
		
		boolean BoolErgebnis = false;
		String Statm;
		
		this.Trans.SetTestmode(false);
		this.TrAnswer = this.Trans.translate(Command);
		if (this.TrAnswer != null) {
			Statm = this.TrAnswer.getOutput();
			this.CI.executeSettings(this.TrAnswer.isSubqueryInUse());
			if (!ConnectionImpl.AutoCommit)
				CI.executeSecSettings("begin transaction");
			BoolErgebnis = CI.executeCommand(Statm);
			if (BoolErgebnis)
				this.Answer = CI.getResult();
		}
		return BoolErgebnis;
	}
	
	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method isWrapperFor()");
	}

	public <T> T unwrap(Class<T> iface) throws SQLException {
		throw new NotCompSQL92Exception("The generic statement-method unwrap()");
	}

	public void addBatch(String arg0) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method addBatch()");

	}

	public void cancel() throws SQLException {
		throw new NotSuppDriverException("The statement-method cancel()");

	}

	public void clearBatch() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method clearBatch()");

	}

	public void clearWarnings() throws SQLException {
		this.Warning = WarningContainer.ClearWarning();

	}

	public void close() throws SQLException {
		this.Answer = null;
		this.CI = null;
		this.RS = null;
		//this.ShList = null;
		//this.ShListExists = false;
		this.Trans = null;
		this.TrAnswer = null;
	}

	public boolean execute(String arg0) throws SQLException {
		boolean boolResult = false;
		String LowCommand = arg0.toLowerCase();
		if (!LowCommand.startsWith("select")) {
			boolResult = true;
			this.executeQuery(arg0);
		}
		else
			this.executeUpdate(arg0);
		
		return boolResult;
	}

	public boolean execute(String arg0, int arg1) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method execute(String, int)");
	}

	public boolean execute(String arg0, int[] arg1) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method execute(String, int[])");
	}

	public boolean execute(String arg0, String[] arg1) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method execute(String, String[])");
	}

	public int[] executeBatch() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method executeBatch()");
	}

	/**
	 * <b> Task of this method <b/> <br/>
	 * Implementation of the method defined in Interface Statement
	 */
	public ResultSet executeQuery(String arg0) throws SQLException {
		ResultSetImpl PreResult;
		String LowCommand = arg0.toLowerCase();
		if (!LowCommand.startsWith("select")) // checks weather it is really a query
			throw new SQLException("Invalid query");
		if (!ExecuteAll(arg0))
			throw new SQLException("Invalid command");
		PreResult = new ResultSetImpl(this.Answer);
		if (this.TrAnswer.hasShadowList())
			PreResult.setShadowList(this.TrAnswer.getShadowList());
		this.RS = PreResult;
		return this.RS;
	}

	public int executeUpdate(String arg0) throws SQLException {
		if (!ExecuteAll(arg0))
			throw new SQLException("Invalid command");
		if (this.Answer.isEmpty())
			this.CountResult = 0; // if Answer is an empty ListExpr it was a DDL-command like create...
		else 
			this.CountResult = this.Answer.second().intValue();
		
		return this.CountResult;
	}

	public int executeUpdate(String arg0, int arg1) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method executeUpdate(String, int)");
	}

	public int executeUpdate(String arg0, int[] arg1) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method executeUpdate(String, int[])");
	}

	public int executeUpdate(String arg0, String[] arg1) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method execute(String, String[])");
	}

	public Connection getConnection() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method getConnection()");
	}

	public int getFetchDirection() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method getFetchDirection()");
	}

	public int getFetchSize() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method getFetchSize()");
	}

	public ResultSet getGeneratedKeys() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method getGeneratedKeys()");
	}

	public int getMaxFieldSize() throws SQLException {
		throw new NotSuppDriverException("The Statement-method getMaxFieldSize()");
	}

	public int getMaxRows() throws SQLException {
		throw new NotSuppDriverException("The Statement-method getMaxRows()");
	}

	public boolean getMoreResults() throws SQLException {
		throw new NotSuppDriverException("Multiple results");
	}

	public boolean getMoreResults(int arg0) throws SQLException {
		throw new NotSuppDriverException("Multiple results");
	}

	public int getQueryTimeout() throws SQLException {
		// No timelimit
		return 0;
	}

	public ResultSet getResultSet() throws SQLException {
		return this.RS;
	}

	public int getResultSetConcurrency() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method getResultSetConcurrency()");
	}

	public int getResultSetHoldability() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method getResultSetHoldability()");
	}

	public int getResultSetType() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method getResultSetType()");
	}

	public int getUpdateCount() throws SQLException {
		return this.CountResult;
	}

	public SQLWarning getWarnings() throws SQLException {
		return this.Warning;
	}

	public boolean isClosed() throws SQLException {
		return (this.CI == null && this.Trans == null);
	}

	public boolean isPoolable() throws SQLException {
		throw new NotCompSQL92Exception("The statement-method isPoolable()");
	}

	public void setCursorName(String arg0) throws SQLException {
		throw new NotSuppDriverException("Cursors");

	}

	public void setEscapeProcessing(boolean arg0) throws SQLException {
		throw new NotSuppDriverException("Escape Processing");

	}

	public void setFetchDirection(int arg0) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method setFetchDirection()");

	}

	public void setFetchSize(int arg0) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method setFetchSize()");

	}

	public void setMaxFieldSize(int arg0) throws SQLException {
		throw new NotSuppDriverException("setMaxFieldSize()");

	}

	public void setMaxRows(int arg0) throws SQLException {
		throw new NotSuppDriverException("setMaxRows()");

	}

	public void setPoolable(boolean arg0) throws SQLException {
		throw new NotCompSQL92Exception("The statement-method setPoolable()");

	}

	public void setQueryTimeout(int arg0) throws SQLException {
		throw new NotSuppDriverException("setQueryTimeout()");

	}

}
