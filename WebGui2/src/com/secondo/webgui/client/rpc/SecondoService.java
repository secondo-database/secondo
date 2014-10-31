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

import com.google.gwt.user.client.rpc.RemoteService;
import com.google.gwt.user.client.rpc.RemoteServiceRelativePath;
import com.secondo.webgui.shared.model.DataType;

/**This interface must be implemented from the SecondoServiceImpl class on the server side of the application to make RPC-Calls to the server.
 * 
 *  @author Kristina Steiger  
 **/
@RemoteServiceRelativePath("secondoService")
public interface SecondoService extends RemoteService {
	
	//secondo connection
	String sendCommand(String command);
	void setSecondoConnectionData(ArrayList<String> data);
	ArrayList<String> updateDatabaseList();
	String openDatabase(String database);
	String closeDatabase(String database);
	String logout();
	
	//optimizer connection
	String setOptimizerConnection(String Host, int Port);
	ArrayList<String> getOptimizerConnectionData();
	ArrayList<String> getOptimizedQuery(String command, String database, boolean executeFlag);
	
	/*get Secondo-Data*/
	ArrayList<String> getFormattedResult();
	ArrayList<DataType> getResultTypeList();
	
	/*get User Session Data*/
	ArrayList<String> getSecondoConnectionData();
	String getOpenDatabase();
	ArrayList<String> getCommandHistory();
	ArrayList<String> getResultHistory();
	
	/*delete User session data*/
	void deleteCommandHistory();
	void resetObjectCounter();
	
	void addCommandToHistory(String command);
	void saveTextFile(String text, String filename);
	public void saveGPXfileToServer(String filename);
	String sendCommandWithoutResult(String command);	
	
}
