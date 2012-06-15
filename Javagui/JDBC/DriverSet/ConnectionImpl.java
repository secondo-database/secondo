package DriverSet;

import java.sql.Array;
import java.sql.Blob;
import java.sql.CallableStatement;
import java.sql.Clob;
import java.sql.DatabaseMetaData;
import java.sql.NClob;
import java.sql.PreparedStatement;
import java.sql.SQLClientInfoException;
import java.sql.SQLException;
import java.sql.SQLWarning;
import java.sql.SQLXML;
import java.sql.Savepoint;
import java.sql.Statement;
import java.sql.Struct;
import java.util.Map;
import java.util.Properties;
import SQL2Secondo.Translator;
import Utilities.WarningContainer;
import Ext_Tools.completeAnswer;

import communication.CommunicationInterface;

public class ConnectionImpl implements java.sql.Connection {

	private CommunicationInterface ComInt;
	private SQLWarning Warning;
	protected static boolean AutoCommit = true;
	private DatabaseMetaData dbmd;
	private Translator Trans; // Needed for nativeSQL
	private completeAnswer Answer; // Needed for nativeSQL
	
	
	public ConnectionImpl(CommunicationInterface Verb) {
		this.ComInt = Verb;
		this.Warning = WarningContainer.getInstance();
	}
	
	@Override
	public boolean isWrapperFor(Class<?> arg0) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public <T> T unwrap(Class<T> arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * <b> Task of this method </b> <br/>
	 * All generated warnings up to this point are deleted
	 * 
	 */
	public void clearWarnings() throws SQLException {
		this.Warning = WarningContainer.ClearWarning();
	}

	public void close() throws SQLException {
		this.ComInt.closeDB();
	}

	/**
	 *  Task of this method:
	 *  All changes will be permanently stored in the database
	 */
	public void commit() throws SQLException {
		this.ComInt.executeSecSettings("commit transaction");
	}

	@Override
	public Array createArrayOf(String arg0, Object[] arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Blob createBlob() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Clob createClob() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public NClob createNClob() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SQLXML createSQLXML() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * <b> Task of this function: </b> <br/>
	 * creates a statement by passing an instance of CommunicationInterface
	 * @return a statement
	 */
	
	public Statement createStatement() throws SQLException {
		
		return new StatementImpl(this.ComInt); 
	}

	@Override
	public Statement createStatement(int arg0, int arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Statement createStatement(int arg0, int arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Struct createStruct(String arg0, Object[] arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

		public boolean getAutoCommit() throws SQLException {
		return AutoCommit;
	}

	public String getCatalog() throws SQLException {
		// catalogs are not supported in secondo
		return null;
	}

	@Override
	public Properties getClientInfo() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getClientInfo(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getHoldability() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	public DatabaseMetaData getMetaData() throws SQLException {
		
		return new DatabaseMetaDataImpl(this.ComInt);
	}

	public int getTransactionIsolation() throws SQLException {
		int result;
		if (AutoCommit)
			result = TRANSACTION_NONE;
		else
			result = TRANSACTION_READ_COMMITTED; // The only transaction isolation level in Secondo
		
		return result;
	}

	@Override
	public Map<String, Class<?>> getTypeMap() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public SQLWarning getWarnings() throws SQLException {
		
		return this.Warning.getNextWarning();  // since Warning is actually a Warning-Container
	}

	public boolean isClosed() throws SQLException {
		
		return this.ComInt.isConnected();
	}

	/**
	 * read only is not supported by secondo. To be in the mode read only means for a database to be
	 * optimized for selections. It does not necessarily mean an update cannot be executed. If it does mean
	 * an update cannot be executed this procedure needs to return true in case the database is in the mode 
	 * read only 
	 */
	public boolean isReadOnly() throws SQLException {
		
		return false;
	}

	@Override
	public boolean isValid(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public String nativeSQL(String command) throws SQLException {
		
		this.Trans = new Translator();
		this.Trans.SetTestmode(false);
		this.Answer = this.Trans.translate(command);
				
		return this.Answer.getOutput();
	}

	@Override
	public CallableStatement prepareCall(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public CallableStatement prepareCall(String arg0, int arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public CallableStatement prepareCall(String arg0, int arg1, int arg2,
			int arg3) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PreparedStatement prepareStatement(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PreparedStatement prepareStatement(String arg0, int arg1)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PreparedStatement prepareStatement(String arg0, int[] arg1)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PreparedStatement prepareStatement(String arg0, String[] arg1)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PreparedStatement prepareStatement(String arg0, int arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PreparedStatement prepareStatement(String arg0, int arg1, int arg2,
			int arg3) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void releaseSavepoint(Savepoint arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

	public void rollback() throws SQLException {
		if (!AutoCommit)
			this.ComInt.executeSecSettings("abort transaction");
	}

	@Override
	public void rollback(Savepoint arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

	/**
	 * Taks of this method
	 * If AutoCommit is set Secondo needs a statement "begin transaction" before
	 * the actual statement is sent. Therefore we need a static variable.
	 */
	public void setAutoCommit(boolean ACmt) throws SQLException {
		AutoCommit = ACmt;
	}

	public void setCatalog(String arg0) throws SQLException {
		SQLWarning Warn = new SQLWarning("Catalogs are not supported in Secondo!");
		this.Warning.setNextWarning(Warn);
	}

	@Override
	public void setClientInfo(Properties arg0) throws SQLClientInfoException {
		// TODO Auto-generated method stub

	}

	@Override
	public void setClientInfo(String arg0, String arg1)
			throws SQLClientInfoException {
		// TODO Auto-generated method stub

	}

	@Override
	public void setHoldability(int arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

	public void setReadOnly(boolean arg0) throws SQLException {
		SQLWarning Warn = new SQLWarning("ReadOnly is not supported in Secondo!");
		this.Warning.setNextWarning(Warn);
	}
	
	/* probably not needed. Method by me
	public void setWarning(String NewWarn) throws SQLException {
		SQLWarning Warn = new SQLWarning(NewWarn);
		if(this.Warning == null)
			this.Warning = Warn;
		else
			this.Warning.setNextWarning(Warn);
	}*/

	@Override
	public Savepoint setSavepoint() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Savepoint setSavepoint(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public void setTransactionIsolation(int arg0) throws SQLException {
		if (arg0 != TRANSACTION_READ_COMMITTED || arg0 != TRANSACTION_NONE)
			throw new SQLException("Transaction Isolation levels other than TRANSACTION_NONE and TRANSACTION_READ_COMMITTED are not supported by secondo");
		if (arg0 == TRANSACTION_NONE)
			this.setAutoCommit(false);
	}

	@Override
	public void setTypeMap(Map<String, Class<?>> arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

}
