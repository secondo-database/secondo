package de.fernunihagen.dna;

import java.util.ArrayList;
import java.util.List;
import sj.lang.ESInterface;
import sj.lang.ErrorCodes;
import sj.lang.IntObj;
import sj.lang.OptimizerInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import android.content.Context;
import android.util.Log;

/**
 * Service for using the databaseconnection
 * @author Michael Küpper
 *
 */
public class SecondoServerService {
	private static final String TAG = SecondroidMainActivity.class.getName();
	private boolean isConnected = false;
	private boolean isDatabaseOpen = false;
	private boolean closeDatabase = false;
	private boolean usingOptimizer = false;
	private ESInterface secondoInterface;
	private OptimizerInterface optimizerInterface;
	private String servername, port, username, password, database;
	private String optServername, optPort;

	private Context context;


	public void connect(String server, String port, String user,
			String password, String optServer, String optPort,
			boolean usingOptimizer) {

		this.username = user == null ? null : user.trim();
		this.password = password == null ? null : password.trim();
		this.servername = server.trim();
		this.port = port.trim();
		this.setOptServername(optServer.trim());
		this.setOptPort(optPort.trim());
		this.usingOptimizer = usingOptimizer;

		secondoInterface = new ESInterface();
		secondoInterface.setPort(Integer.parseInt(port));
		secondoInterface.setHostname(server);
		if (user != null) {
			secondoInterface.setPassWd(password);
			secondoInterface.setUserName(user);
		}

		secondoInterface.useBinaryLists(true);

		// Verbindung zum
		try {
			boolean ok = secondoInterface.connect();
			if (!ok) {
				if (context != null)
					Log.e(TAG, context.getString(R.string.noserverconnection));
				return;
			}
		} catch (Exception ex) {
			if (context != null)
				Log.e(TAG, context.getString(R.string.noserverconnection)
						+ ". Exception: " + ex.getMessage());
		}

		if (this.usingOptimizer) {
			optimizerInterface = new OptimizerInterface();
			optimizerInterface.setPort(Integer.parseInt(optPort));
			optimizerInterface.setHost(optServer);
			try {
				boolean ok = optimizerInterface.connect();
				if (!ok) {
					if (context != null)
						Log.e(TAG, context
								.getString(R.string.nooptimizerconnection));
					return;
				}
			} catch (Exception ex) {
				if (context != null)
					Log.e(TAG,
							context.getString(R.string.nooptimizerconnection)
									+ ". Exception: " + ex.getMessage());
			}
		}

		Log.i(TAG, "Serververbindung hergestellt");
		this.isConnected = true;
	}

	/** 
	 * Get the available Database from the Databasesystem with "list database"
	 * @return a List of Databases
	 */
	public List<String> getDatabases() {
		ArrayList<String> databases = new ArrayList<String>();
		IntByReference errorCode = new IntByReference();
		IntByReference errorPos = new IntByReference();
		StringBuffer errorMessage = new StringBuffer();

		if (this.isConnected == false) {
			// throw new NoDatabaseConnectionException();
		}

		ListExpr resultList = execute("list databases", errorCode, errorPos,
				errorMessage, true);

		if (resultList.isEmpty()) // no result
			return databases;
		
		resultList = resultList.second().second();

		while (!resultList.isEmpty()) {
			String databasename = resultList.first().symbolValue();
			resultList = resultList.rest();

			databases.add(databasename);
		}

		return databases;

	}

	public void setDatabase(String database) {

		if (this.database != null && !this.database.equals(database)) {
			this.closeDatabase = true;
			this.isDatabaseOpen = false;
		}
		this.database = database;
	}

	public String getDatabase() {
		return this.database;
	}

	public ListExpr execute(String command, IntByReference errorCode,
			IntByReference errorPos, StringBuffer errorMessage,
			boolean directQuery) {
		ListExpr resultList = new ListExpr();
		if (!secondoInterface.isConnected()) {
			secondoInterface.connect();
		}
		if (this.closeDatabase) {
			// Ist schon eine Datenbank geöffnet, dann schließen
			if (this.isDatabaseOpen == true) {
				secondoInterface.secondo("close database", resultList, errorCode,
					errorPos, errorMessage);
			}
			this.closeDatabase = false;
			this.isDatabaseOpen = false;
			if (errorCode.value != 0)
				Log.e(TAG, errorMessage.toString());
			else {
				Log.i(TAG, resultList.toString());
			}
		}

		Log.i(TAG, "Query " + command + " wird gestartet");
		try {
			if (usingOptimizer && !directQuery) {
				IntObj errorcode = new IntObj();
				String optimizedcommand = optimizerInterface.optimize_execute(
						command, database, errorcode, false);
				if(optimizedcommand.startsWith("::ERROR::")){
					errorMessage.append("Error while executung Optimizer: ");
					errorMessage.append(optimizedcommand);
			       return null;
				}
				if (!"".equals(optimizedcommand)) {
					command = optimizedcommand;
				}
			}
			secondoInterface.secondo(command, resultList, errorCode, errorPos,
					errorMessage);

			Log.i(TAG, "Query ausgeführt");
		} catch (Exception ex) {
			Log.e(TAG, "Query abgebrochen", ex);

		}
		if (errorCode.value != 0)
			Log.e(TAG, errorMessage.toString());
		else {
			Log.i(TAG, resultList.toString());
		}

		return resultList;
	}

	public String executeInFile(String command, IntByReference errorCode,
			IntByReference errorPos, StringBuffer errorMessage,
			boolean directQuery) {
		ListExpr resultList = new ListExpr();
		if (this.closeDatabase) {
			// is the database alredy opened, the close the connection
			secondoInterface.secondo("close database", resultList, errorCode,
					errorPos, errorMessage);
			this.closeDatabase = false;
			if (errorCode.value != 0)
				Log.e(TAG, errorMessage.toString());
			else {
				Log.i(TAG, resultList.toString());
			}
		}
		
		if (!isOptimizerQuery(command)){
			directQuery = true;  
		}

		Log.i(TAG, "Query " + command + " wird gestartet");
		try {
			if (usingOptimizer && !directQuery) {
				IntObj errorcode = new IntObj();
				String optimizedcommand = optimizerInterface.optimize_execute(
						command, database, errorcode, false);
				if(optimizedcommand.startsWith("::ERROR::")){
					errorMessage.append("Error while executung Optimizer: ");
					errorMessage.append(optimizedcommand);
			       return null;
				}
				if (errorCode.value == ErrorCodes.NO_ERROR
						&& !"".equals(optimizedcommand)) {
					command = optimizedcommand;
				}
			}

			secondoInterface.secondo(command, resultList, errorCode, errorPos,
					errorMessage);

			Log.i(TAG, "Query ausgeführt");
		} catch (Exception ex) {
			Log.e(TAG, "Query abgebrochen", ex);

		}
		if (errorCode.value != 0)
			Log.e(TAG, errorMessage.toString());
		else {
			Log.i(TAG, resultList.toString());
		}

		String fileName = context.getFilesDir() + "/resultlist";
		resultList.writeToFile(fileName);

		return fileName;
	}

	private boolean isOptimizerQuery(String command) {
		
		 // look for insert into, delete from and update rename 
		   boolean isOptUpdateCommand = false;
		   boolean isSelect = true;
		   boolean catChanged = false;

		   if(command.startsWith("sql ")){
		      isOptUpdateCommand = true;
		   } else if( command.matches("insert +into.*")){
		      isOptUpdateCommand = true;
		   } else if( command.matches("delete +from.*")){
		      isOptUpdateCommand = true;
		   } else if( command.matches("update +[a-z][a-z,A-Z,0-9,_]* *set.*")){
		      isOptUpdateCommand = true;
		   } else if(command.matches("create +table .*")){
		      isOptUpdateCommand = true;
		      isSelect = false;
		      catChanged = true;
		   } else if(command.matches("create +index .*")){
		      isOptUpdateCommand = true;
		      isSelect = false;
		      catChanged = true;
		   } else if(command.startsWith("drop ")){
		      isOptUpdateCommand = true;
		      isSelect = false;
		      catChanged = true;
		   } else if(command.startsWith("select ")){
		      isOptUpdateCommand = true;
		   } 

		   return isOptUpdateCommand;
	}

	public void reconnect() {
		connect(this.servername, this.port, this.username, this.password, this.optServername, this.optPort, this.usingOptimizer);
	}

	public boolean isConnected() {
		return secondoInterface.isConnected()
				&& (!usingOptimizer || optimizerInterface.isConnected());
	}

	public String reopenDatabase() {
		IntByReference errorCode = new IntByReference();
		IntByReference errorPos = new IntByReference();
		StringBuffer errorMessage = new StringBuffer();

		if (isDatabaseOpen) {
			execute("close database", errorCode, errorPos, errorMessage, true);
		}
		execute("open database " + this.database, errorCode, errorPos,
				errorMessage, true);
		// TODO: Fehlermeldungen abfangen und zurückliefern
		isDatabaseOpen = true;
		return null;

	}

	public void disconnect() {
		if (optimizerInterface != null) {
			optimizerInterface.disconnect();
		}
		secondoInterface.destroy();
	}

	public void setContext(Context context) {
		this.context = context;

	}

	public String getServername() {
		return servername;
	}

	public String getPort() {
		return port;
	}

	public String getUsername() {
		return username;
	}

	public String getPassword() {
		return password;
	}

	public String getOptServername() {
		return optServername;
	}

	public void setOptServername(String optServername) {
		this.optServername = optServername;
	}

	public String getOptPort() {
		return optPort;
	}

	public void setOptPort(String optPort) {
		this.optPort = optPort;
	}

	public boolean isUsingOptimizer() {
		return usingOptimizer;
	}

}
