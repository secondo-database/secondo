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

package communication.optimizer;

public class ErrorCodes{

public static final int NO_ERROR=0;
public static final int NO_OPTIMIZER=1;
public static final int UNKNOW_HOST=2;
public static final int IO_ERROR=3;
public static final int SECURITY_ERROR=4;
public static final int NOT_CONNECTED =5;
public static final int PROTOCOL_ERROR = 6;
public static final int OPTIMIZATION_FAILED = 7;
public static final int NO_OPTIMIZATION_POSSIBLE = 8;
public static final int CONNECTION_BROKEN = 9;


public  static String getErrorMessage(int ErrorCode){

 switch(ErrorCode){
   case NO_ERROR     : return "no error";
   case NO_OPTIMIZER : return "server is not an optimizer";
   case UNKNOW_HOST  : return "host-name unknown";
   case IO_ERROR     : return "io-error";
   case SECURITY_ERROR : return "security error";
   case NOT_CONNECTED  : return "not connected";
   case PROTOCOL_ERROR : return "protocol error";
   case OPTIMIZATION_FAILED : return "optimization failed";
   case NO_OPTIMIZATION_POSSIBLE : return "no optimization possible";
   case CONNECTION_BROKEN : return "connection to server lost";
   default : return "unknow error-code";
 }



}






}
