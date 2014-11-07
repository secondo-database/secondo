//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package com.secondo.webgui.client.rpc;

import java.util.ArrayList;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.google.gwt.user.client.rpc.AsyncCallback;
import com.secondo.webgui.shared.model.DataType;

/**This interface is the asynchronous representation of the secondo service interface.
 * 
 *  @author Kristina Steiger
 *  
 **/
public interface SecondoServiceAsync {
	
	//secondo connection
	 void sendCommand(String s, AsyncCallback<String> callback);
	 void sendCommandWithoutResult(String s, AsyncCallback<String> callback);																																																																																																																																																																																																																																																																																																																																																																																																																																																																											
	 void setSecondoConnectionData(ArrayList<String> data, AsyncCallback<Void> callback);
	 void updateDatabaseList(AsyncCallback<ArrayList<String>> callback);
	 void openDatabase(String database, AsyncCallback<String> callback);
	 void closeDatabase(String database, AsyncCallback<String> callback);
	 void logout(AsyncCallback<String> callback);
	 
	//optimizer connection
	 void setOptimizerConnection(String Host, int Port, AsyncCallback<String> callback);
	 void getOptimizerConnectionData(AsyncCallback<ArrayList<String>> callback);
	 void getOptimizedQuery(String command, String database, boolean executeFlag, AsyncCallback<ArrayList<String>> callback);
	 
	 /*get Secondo-Data*/
	 void getFormattedResult(AsyncCallback<ArrayList<String>> callback);
	 void getResultTypeList(AsyncCallback<ArrayList<DataType>> callback);
	 
	 /*get User Session Data*/
	 void getSecondoConnectionData(AsyncCallback<ArrayList<String>> callback);
	 void getOpenDatabase(AsyncCallback<String> callback);
	 void getCommandHistory(AsyncCallback<ArrayList<String>> callback);
	 void getResultHistory(AsyncCallback<ArrayList<String>> callback);
		
	 /*delete User session data*/
	 void deleteCommandHistory(AsyncCallback<Void> callback);	 
	 void resetObjectCounter(AsyncCallback<Void> callback);
	 
	 void addCommandToHistory(String command, AsyncCallback<Void> callback);
	 void saveTextFile(String text, String filename, AsyncCallback<Void> callback);
	void saveGPXfileToServer(String filename, AsyncCallback<Void> callback);
	
	void getNumberOfTuplesInRelationFromResultList(AsyncCallback<Integer> callback);
	
	
}
