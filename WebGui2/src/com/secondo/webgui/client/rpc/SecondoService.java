package com.secondo.webgui.client.rpc;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.RemoteService;
import com.google.gwt.user.client.rpc.RemoteServiceRelativePath;
import com.secondo.webgui.shared.model.DataType;

/**
 * This interface must be implemented from the SecondoServiceImpl class on the
 * server side of the application to make RPC-Calls to the server.
 * 
 * @author Irina Russkaya
 **/
@RemoteServiceRelativePath("secondoService")
public interface SecondoService extends RemoteService {

	// secondo connection
	String sendCommand(String command);

	void setSecondoConnectionData(ArrayList<String> data);

	String openDatabase(String database);

	String closeDatabase(String database);

	String logout();

	/* get Secondo-Data */
	ArrayList<String> getFormattedResult();

	ArrayList<DataType> getResultTypeList();

	/* get User Session Data */
	ArrayList<String> getSecondoConnectionData();

	String getOpenDatabase();

	ArrayList<String> getCommandHistory();

	ArrayList<String> getResultHistory();

	/* delete User session data */
	void deleteCommandHistory();

	void resetObjectCounter();

	void addCommandToHistory(String command);

	void saveTextFile(String text, String filename);

	public void saveGPXfileToServer(String filename);

	String sendCommandWithoutResult(String command);

	Boolean sendMail(String html);
}
