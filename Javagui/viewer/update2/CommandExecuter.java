/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

	10-2013 Jasmine Ahmed: 
	Class taken from package viewer.update.CommandExecuter.
	Added methods beginTransaction, commitTransaction, abortTransaction.


*/
package viewer.update2;

import sj.lang.*;
import gui.*;
;

/*
This class executes commands by sending them to SECONDO. The response-values of SECONDO are
afterwards stored in members of this class and can be asked for by other classes. 
 
*/
public class CommandExecuter {
	
	private UpdateInterface updateInterface = null;
	private ListExpr resultList;
    private IntByReference errorCode ;
    private IntByReference errorPos ;
    private StringBuffer errorMessage ;
	
	public CommandExecuter (){
	  	resultList = new ListExpr();
	    errorCode = new IntByReference(0);
	    errorPos = new IntByReference(0);
	    errorMessage = new StringBuffer();
	}
	 
	public boolean executeCommand (String command, int commandLevel)
	{
		if (updateInterface == null)
		{
			updateInterface = MainWindow.getUpdateInterface();
		}
		// Executes the remote command.
		if(updateInterface.isInitialized())
		{
			updateInterface.secondo(command,           //Command to execute.
		         resultList, errorCode, errorPos, errorMessage);
		    boolean success = errorCode.value==0;
		    return success;
		}
		else
		{
			errorMessage = new StringBuffer("Connection to SECONDO lost!");
			return false;
		}

	}
	
	
	/*
	 Abort transaction
	 
	 */
	public boolean abortTransaction()
	{
		return this.executeCommand("(abort transaction)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX);
	}
	
	
	/*
	 Begin transaction
	 
	 */
	public boolean beginTransaction()
	{
		return this.executeCommand("(begin transaction)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX);
	}
	
	/*
	 Commit transaction
	 
	 */
	public boolean commitTransaction()
	{
		return this.executeCommand("(commit transaction)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX);
	}
	
	
	
	public ListExpr getResultList()
	{
		return resultList;
	}
	
	public IntByReference getErrorCode()
	{
		return errorCode;	
	}
	
	public IntByReference getErrorPos()
	{
		return errorPos;
	}
	
	public StringBuffer getErrorMessage()
	{
		return errorMessage;
	}
	
	/**
	 * Returns the ServerErrorCode for a 'list objects' command.
	 */
	public int testConnection()
	{
		this.executeCommand("(list objects)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX);
		return getErrorCode().value;
	}

}
