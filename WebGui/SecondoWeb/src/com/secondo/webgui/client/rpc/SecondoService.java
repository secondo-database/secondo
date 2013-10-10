package com.secondo.webgui.client.rpc;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.RemoteService;
import com.google.gwt.user.client.rpc.RemoteServiceRelativePath;
import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Point;


@RemoteServiceRelativePath("secondoService")
public interface SecondoService extends RemoteService {
	
	String sendCommand(String command);
	ArrayList<String> setSecondoConnectionData(ArrayList<String> data);
	String openDatabase(String database);
	String closeDatabase(String database);
	String logout();
	
	/*get Secondo-Data*/
	ArrayList<String> getFormattedResult();
	ArrayList<String> getFirstTuplesOfValues();
	ArrayList<DataType> getResultTypeList();
	
	/*get User Session Data*/
	ArrayList<String> getSecondoConnectionData();
	String getOpenDatabase();
	String isLoggedIn();
	ArrayList<String> getCommandHistory();
	ArrayList<String> getResultHistory();
	
	 /*update results on selection of query history*/
	String updateResult(String command);
	String deleteCommandHistory();
}
