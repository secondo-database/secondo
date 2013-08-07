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

   04-2005, Matthias Zielke


*/
package sj.lang;

/*
This interface provides all methods the 'UpdateViewer' needs to connect to Secondo.
 
*/
public interface UpdateInterface {
	
	
/*
~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands.

Error Codes: see definition module.

If value 0 is returned, the command was executed without error.

*/
/*	public void secondo( String commandText,
            ListExpr commandLE,
            int commandLevel,
            boolean commandAsText,
            boolean resultAsText,
            ListExpr resultList,
            IntByReference errorCode,
            IntByReference errorPos,
            StringBuffer errorMessage );
 */
     public void secondo(String command,
                         ListExpr resultList,
                         IntByReference errorCode,
                         IntByReference errorPos,
                         StringBuffer errorMessage);
	
/* 
returns true if this interface is connected to secondo server 

*/
	  public boolean isInitialized();

}
