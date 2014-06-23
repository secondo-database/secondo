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

package com.secondo.webgui.server.controller;

import java.io.IOException;
import java.util.ArrayList;
import com.secondo.webgui.server.optimizerinterface.ErrorCodes;
import com.secondo.webgui.server.optimizerinterface.IntObj;
import com.secondo.webgui.server.optimizerinterface.Interval;
import com.secondo.webgui.server.optimizerinterface.OptimizerInterface;

/**
 * This class offers methods to connect to the optimizer server and to send a query to the optimizer and return the optimized query or an
 * error message.
 * 
 * @author Kristina Steiger
 */
public class OptimizerConnector {

	//interface for connection to the optimizer
	private OptimizerInterface optimizerInterface = new OptimizerInterface();
	
	//temporary data 
	private String errorMessage = "";
	private String timeSpentForOptimization = "";

	public OptimizerConnector() {
	}

	/** Connects to the optimizer 
	 * 
	 * @return Returns true if connection was successful
	 * */
	public boolean connectOptimizer() {

		boolean ok = optimizerInterface.connect();
		return ok;
	}

	/** Disconnects from the optimizer **/
	public void disconnectOptimizer() {
		if (optimizerInterface.isConnected()) {
			optimizerInterface.disconnect();
		}
	}
	
	/**Checks if optimizer is connected
	 * 
	 * @return Returns true if the optimizer is connected
	 * */
	public boolean isOptimizerConnected(){
		return optimizerInterface.isConnected();
	}

	/** Sets the values for the connection with the optimizer 
	 * 
	 * @param Host New host of the optimizer
	 * @param Port New port of the optimizer
	 * */
	public void setOptimizerConnection(String Host, int Port) {
		optimizerInterface.setHost(Host);
		optimizerInterface.setPort(Port);
	}
	
	/**Reutrns the errormessage of the last error in executing the optimizer
	 * 
	 * @return The errormessage of the last error
	 * */
	public String getLastError(){
		return ErrorCodes.getErrorMessage(optimizerInterface.getLastError());
	}

	/**Returns the current connection data of the optimizer
	 * 
	 *  @return The optimizer connection data
	 *  */
	public ArrayList<String> getOptimizerConnection() {
		ArrayList<String> connectionData = new ArrayList<String>();
		connectionData.add(optimizerInterface.getHost());
		connectionData.add(String.valueOf(optimizerInterface.getPort()));
		return connectionData;
	}

	/** Sends the query to the optimizer and returns the optimized query or an error message
	 * 
	 * @param command The command to be send to the optimizer
	 * @param database The currently open database
	 * @param ef The execute Flag
	 * @return The optimized command
	 */
	public String getOptimizedQuery(String command, String database, boolean ef) throws IOException {
			
		   boolean isOptUpdateCommand = false;
		   boolean isSelect = true;
		   boolean catChanged = false;
		   String result = "";
		   errorMessage = "";
		   IntObj errorCode = new IntObj();
		 //boolean executeFlag = ef; //command starts with <execute>

			if (!optimizerInterface.isConnected()) { 
				optimizerInterface.connect();
			}				

            //check command syntax for insert into, delete from and update rename commands
			   if(command.trim().startsWith("sql ") || command.trim().startsWith("sql\n")){
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
			   
			   //check if optimizer is connected
			   if(isOptUpdateCommand){
			     if(!optimizerInterface.isConnected()){ // error: select clause found but no optimizer enabled
			        errorMessage = errorMessage + "optimizer not available";
			    	 return "";
			     }

			     //change command to lower case string
			     command = varToLowerCase(command);
			     
			     //check if a database is available
			     if(database.length()==0){
			    	 errorMessage = errorMessage + "\nno database open";
			    	 return "";
			     }
			     
			     //execute optimization
				result = optimizerInterface.optimize_execute(command, database, errorCode, false);
				   
				System.out.println("#####optimized query: " + result);	
				
			     if(errorCode.value!=ErrorCodes.NO_ERROR){  // errors in optimization
			    	 errorMessage = errorMessage + "\nerror in optimization of this query";
			    	 return "";
			      }
			     else 
			    	 if(result.startsWith("::ERROR::")){
			             errorMessage = errorMessage + "\nproblem in optimization: \n" + formatOptimizerError(result.substring(9))+"\n";
			             return "";
			            } 
			         else 
			    	    if(catChanged){
			               boolean ok = true;//reopenDatabase();
			               errorMessage = errorMessage + "Hotfix : reopen database";
			               if(ok){
			            	   errorMessage = errorMessage + "successful \n";
			               } 
			               else {
			                  errorMessage = errorMessage + "failed \n";
			               }
			            return "";
			           } 
			    	   else {
			            if(isSelect){
			                return "query " + result;
			            } 
			            else {
			            return result;
			            }
			    	  }
			   }
			   
			   // some more effort for select which can also be only a part within a query
			   if(command.length()<6) // command can't contain a select clause
			     return command;

			   String TmpCommand = "";
			   int First = 0;
			   Interval SelectClauseInterval=null;
			   String SelectClause="";
			   boolean isQuery = false;
			   int length = command.length();
			   
			   while((SelectClauseInterval=findSelectClause(command,First))!=null){
			      if(!optimizerInterface.isConnected()){ // error: select clause found but no optimizer enabled
			         errorMessage = errorMessage + "optimizer not available";
			 	     return "";
			      }

			      if(SelectClauseInterval.getMin()==0 && SelectClauseInterval.getMax()==length)
			          isQuery = true;

			      //  the text before the select clause
			      if(SelectClauseInterval.getMin()>First)
			         TmpCommand = TmpCommand + command.substring(First,SelectClauseInterval.getMin()-1);
			      // extract the select-clause
			      SelectClause = command.substring(SelectClauseInterval.getMin(),SelectClauseInterval.getMax());

			      // optimize the select-clause
			      long starttime=0;
			      if(tools.Environment.MEASURE_TIME)
			         starttime = System.currentTimeMillis();
			      SelectClause = varToLowerCase(SelectClause);
			      if(database.length()==0){
			        errorMessage = errorMessage + "\nno database open";
			        return "";
			      }
			      
			    //execute optimization
				result = optimizerInterface.optimize_execute(command, database, errorCode, false);
					   
				System.out.println("#####optimized query: " + result);
				
			      if(tools.Environment.MEASURE_TIME){
			         timeSpentForOptimization ="used time to optimize query: "+(System.currentTimeMillis()-starttime)+" ms";
			      }
			      if(errorCode.value!=ErrorCodes.NO_ERROR){  // error in optimization
			         errorMessage = errorMessage + "\nerror in optimization of this query";
			         return "";
			       }else 
			    	   if(result.startsWith("::ERROR::")){
			                errorMessage = errorMessage + "\nproblem in optimization of this query\n" + result.substring(9)+"\n";
			                return "";
			           } 
			    	   else {
			               TmpCommand += isQuery ?  "query "+ result : result;
			               First = SelectClauseInterval.getMax() + 1;
			           }
			    }// while
			   
				//Disconnect from optimizer
				optimizerInterface.disconnect(); 

			    // append the rest of the command
			    if(First<command.length())
			       command = TmpCommand + command.substring(First,command.length());
			    else
			       command = TmpCommand;
			    return  command;
	  }

	/**Changes the format of error messages coming from the optimizer. The Error
	 * messages from the optimizer are not nice formatted. For example line
	 * breaks are marked by \n. This is changed by this function.
	 * 
	 * @param errmsg The error message to be formatted
	 * @return The formatted error message
	 */
	private String formatOptimizerError(String errmsg) {
		errmsg = errmsg.replaceAll("\\\\n", "\n");
		errmsg = errmsg.replaceAll("\\\\t", "\t");
		errmsg = errmsg.replaceAll("\\\\'", "'");
		if (errmsg.startsWith("'") && errmsg.endsWith("'")
				&& errmsg.length() > 1) {
			errmsg = errmsg.substring(1, errmsg.length() - 1);
		}
		errmsg = errmsg.replaceAll("''", "\"");
		return errmsg;
	}
	
	  /**Changes the first letter of all words outside of quotes to a lower case.  
	   * 
	   * @param str The string to be checked
	   * @return The string with lower case letters
	   * */
	  private  String varToLowerCase(String str){

	     StringBuffer buf = new StringBuffer();
	     int state = 0; //normal = 0, inDoublequotes = 1 in quotes = 2
	     int pos = 0;
	     int wordPos = 0;
	     for(int i=0;i<str.length();i++){
	        char c = str.charAt(i);
	        switch(state){
	          case 0: { // normal 
	             if(c=='"'){ // begin of a string constant
	               state = 1;
	               wordPos = 0;
	               buf.append(c);
	             } else if(c=='\''){ //begin of a text constant
	               state = 2;
	               wordPos=0;
	               buf.append(c); 
	             } else if(isLetter(c)){
	                if(wordPos==0){
	                   wordPos++;    
	                   buf.append(toLower(c));
	                } else {
	                   buf.append(toLower(c));
	                }
	             } else {
	               wordPos = 0;
	               buf.append(c);
	             }
	             break;
	           }
	          case 1: {
	            if(c=='"'){
	               state = 0;
	               wordPos = 0;
	            }
	            buf.append(c);
	            break;
	          }
	          case 2: {
	            if(c=='\''){
	              state = 0;
	              wordPos = 0;
	            }
	            buf.append(c);
	          }
	        }
	     }
	     return buf.toString();
	  }
	  
	  /**Checks if the given character is a letter between A and Z
	   * 
	   * @param c The character to be checked
	   * @return True if the character is between A and Z
	   * */
	  private boolean isLetter(char c){
		     return ((c>='A') && (c<='Z') ) || ((c>='a' && c<='z'));
	  }
	  
	  /**Changes the given character to lower case
	   * 
	   * @param c The character to be changed to lower case
	   * @return The character in lower case
	   * */
	  private char toLower(char c){
		     if(c>='A' && c<='Z'){
		        return (char)(c - 'A' + 'a'); 
		     }
		     return c;
	 }
	  
	  /** Finds a select clause within command the search starts at position first
	    * returns the interval of command containing the clause or null if not found one
	    * 
	    * @param command The command to be checked for a select clause
	    * @param first The first position
	    */
	  private Interval findSelectClause(String command,int first){

	      int state=0;
	      int minpos=-1;
	      int maxpos=-1;
	      int length = command.length();
	      int curpos = first;
	      if(length-first <7) // no chance to find a select clause ( too little characters)
	          return null;

	      command = command.toLowerCase();

	      int selectPos = command.indexOf("select",first);
	      if(selectPos < first) // no select contained
	         return null;
	      int sqlPos = command.indexOf("sql ",first);
	      if(selectPos==first | sqlPos==first){ // the whole command is a select clause
	          return new Interval(first,length);
	      }


	      /*
	       A select clause is searched with help of a (extended) finite automate.
	       description of the used states:
	       state 0: is the start-state,
	       state 1: filters strings before a selects clause
	       state 2: a potential begin of a select clause which is not enclosed in brackets
	       state 3: a potential begin of a select clause whithin brackets
	       states 4-9: check for character-sequence "select" without brackets
	                   the string for the optimization is given from begin of "select" to the
			   end of command
		states 11-16: check for character-sequence "select within brackets
		              the string for the optimization is given from begin of "select" to the
			      appropriate closing bracket
	        state 19 : ignore closing brackets in quotes within a select-clause
		state 17 : count the opened brackets within a select clause, so it's possible to have
		           (select * from ....() (()) () (()) )
			   as valid select-clause
		state 18: characters within a select-clause
	      */

	      char c;
	      int noOfBraces = 0;
	      while(curpos<length){
	         if(curpos>length){
		     return null;
		 }
		 c = command.charAt(curpos);
		 if(state==0){
		   switch(c){
		      case '\"' : { curpos++;
		                    state=1;
				    break;
				   }
		      case ' '  : { curpos++;
		                    state=2;
				    break;
				  }
		      case '('  : { minpos=curpos; // a potentially start of a select clause
		                    state=3;
				    curpos++;
				    break;
				  }
	              default : curpos++;
	            }
		 }else if(state==1){
	            if(c=='\"')
		       state=0;
		    curpos++;
		 }else if(state==2){
		    switch(c) {
	              case ' ' : curpos++;
		                 break;
	              case '(' : state=3;
		                 minpos=curpos;
		                 curpos++;
	                         break;
		      case 's' : minpos=curpos;
		                 state=4;
				 curpos++;
				 break;
		      case '\"' : state=1;
		                  curpos++;
	                          break;
		      default : state=0;
		                  curpos++;
		    }
		 }else if(state==3){
		      switch(c){
	                case ' ' : curpos++;
			           break;
			case '(' : minpos=curpos;
			           curpos++;
				   break;
		        case '\"': state=1;
			           curpos++;
				   break;
	                case 's' : state=11;
			           curpos++;
				   break;
		        default  : state=0;
			           curpos++;
		      }
		 } else if(state==4){
		     switch(c){
		        case 'e' : curpos++;
			           state=5;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		 } else if(state==5){
		     switch(c){
		        case 'l' : curpos++;
			           state=6;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		 } else if(state==6){
		      switch(c){
		        case 'e' : curpos++;
			           state=7;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		 } else if(state==7){
		     switch(c){
		        case 'c' : curpos++;
			           state=8;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		 } else if(state==8){
	             switch(c){
		        case 't' : curpos++;
			           state=9;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		 } else if(state==9){
		     if(c==' ' | c=='('  | c=='\"')
		        return new Interval(minpos,length);
		     else{
		        state=0;
	             }
		 } else if(state==11){
		    switch(c){
		        case 'e' : curpos++;
			           state=12;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		 } else if(state==12){
		     switch(c){
		        case 'l' : curpos++;
			           state=13;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }

		 } else if(state==13){
		     switch(c){
		        case 'e' : curpos++;
			           state=14;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		 } else if(state==14){
		    switch(c){
		        case 'c' : curpos++;
			           state=15;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
	          } else if(state==15){
		      switch(c){
		        case 't' : curpos++;
			           state=16;
				   break;
	                case '(' : minpos=curpos;
			           curpos++;
				   state=3;
				   break;
			case ' ' : state=2;
			           curpos++;
				   break;
	                case '\"': state=1;
			           curpos++;
				   break;
	                default   : state=0;
			            curpos++;
		     }
		  } else if(state==16){
		      switch(c){
		        case ' ' : curpos++;
			           state=18;
				   break;
	                case '\"': curpos++;
			           state=19;
				   break;
	                case '(' : curpos++;
			           noOfBraces++;
				   state=17;
				   break;
			default  : curpos++;
	                           state=0;
		       }
		  } else if(state==19){
		      if(c=='\"'){
		         state=18;
		      }
		      curpos++;
		  } else if(state==18){
		     switch(c){
	                case '(' : state=17;
			           curpos++;
				   noOfBraces++;
	                           break;
			case '\"': state=19;
			           curpos++;
				   break;
			case ')' : return new Interval(minpos+1,curpos); // remove braces
			default  : curpos++;
		     }
		  } else if(state==17){
		     switch(c){
	                case '(' : curpos++;
				   noOfBraces++;
				   break;
			case '\"' : curpos++;
			            state=20;
	                break;
			case ')'  : noOfBraces--;
			            if(noOfBraces==0)
				        state=18;
			    	    curpos++;
	              break;
	     default : curpos++;
		     }
		  } else if(state==20){
		      if(c=='\"')
		          state=17;
		      curpos++;

		  }

	      } // while
	      return null; // no select found

	  } // findSelectClause

    /**Returns the current errormessage
	 * 
	 * @return The current error message*/
	public String getErrorMessage() {
		return errorMessage;
	}

	/**Returns the current time spent for optimization
	 * 
	 * @return The current time spent for optimization*/
	public String getTimeSpentForOptimization() {
		return timeSpentForOptimization;
	}
}
