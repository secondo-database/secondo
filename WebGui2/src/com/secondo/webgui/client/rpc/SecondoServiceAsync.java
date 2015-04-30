package com.secondo.webgui.client.rpc;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.AsyncCallback;
import com.secondo.webgui.shared.model.DataType;

/**
 * This interface is the asynchronous representation of the secondo service
 * interface.
 * 
 * @author Irina Russkaya
 * 
 **/
public interface SecondoServiceAsync {

	// secondo connection
	void sendCommand(String s, AsyncCallback<String> callback);

	void sendCommandWithoutResult(String s, AsyncCallback<String> callback);

	void setSecondoConnectionData(ArrayList<String> data,
			AsyncCallback<Void> callback);

	void openDatabase(String database, AsyncCallback<String> callback);

	void closeDatabase(String database, AsyncCallback<String> callback);

	void logout(AsyncCallback<String> callback);

	/* get Secondo-Data */
	void getFormattedResult(AsyncCallback<ArrayList<String>> callback);

	void getResultTypeList(AsyncCallback<ArrayList<DataType>> callback);

	/* get User Session Data */
	void getSecondoConnectionData(AsyncCallback<ArrayList<String>> callback);

	void getOpenDatabase(AsyncCallback<String> callback);

	void getCommandHistory(AsyncCallback<ArrayList<String>> callback);

	void getResultHistory(AsyncCallback<ArrayList<String>> callback);

	/* delete User session data */
	void deleteCommandHistory(AsyncCallback<Void> callback);

	void resetObjectCounter(AsyncCallback<Void> callback);

	void addCommandToHistory(String command, AsyncCallback<Void> callback);

	void saveTextFile(String text, String filename, AsyncCallback<Void> callback);

	void saveGPXfileToServer(String filename, AsyncCallback<Void> callback);

	void sendMail(String html, AsyncCallback<Boolean> callback);
}
