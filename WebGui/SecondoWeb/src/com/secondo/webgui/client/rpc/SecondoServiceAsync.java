package com.secondo.webgui.client.rpc;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.AsyncCallback;
import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Point;

public interface SecondoServiceAsync {
	
	 void sendCommand(String s, AsyncCallback<String> callback);
	 void setSecondoConnectionData(ArrayList<String> data, AsyncCallback<ArrayList<String>> callback);
	 void openDatabase(String database, AsyncCallback<String> callback);
	 void closeDatabase(String database, AsyncCallback<String> callback);
	 void logout(AsyncCallback<String> callback);
	 
	 /*get Secondo-Data*/
	 void getFormattedResult(AsyncCallback<ArrayList<String>> callback);
	 void getFirstTuplesOfValues(AsyncCallback<ArrayList<String>> callback);
	 void getResultTypeList(AsyncCallback<ArrayList<DataType>> callback);
	 
	 /*get User Session Data*/
	 void getSecondoConnectionData(AsyncCallback<ArrayList<String>> callback);
	 void getOpenDatabase(AsyncCallback<String> callback);
	 void isLoggedIn(AsyncCallback<String> callback);
	 void getCommandHistory(AsyncCallback<ArrayList<String>> callback);
	 void getResultHistory(AsyncCallback<ArrayList<String>> callback);
		
	/*update results on selection of query history*/
	 void updateResult(String command, AsyncCallback<String> callback);
	 void deleteCommandHistory(AsyncCallback<String> callback);

}
