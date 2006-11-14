//This file is part of SECONDO.

//Copyright (C) 2006, University in Hagen, Department of Computer Science, 
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

package gui;

import tools.Reporter;
import java.io.*;
import sj.lang.ListExpr;

/**
 This class provides the functionality of the TestRunner of the
 TTY version of secondo. 
 This means, this class is able to analyse files in the testrunner 
 format and to execute the commands contained in it together with
 error analyse.
**/


class TestRunner{

/** instance executing the commands in the testfile **/
private CommandPanel cp;
/** the current state of this TestRunner **/
private int state = START;
/** the tolerance used for comparisions of the expected and the
  * actual result 
  **/
private double tolerance=0.0;
/** flag for absulute or procentual derivation of expected and
  * actual results.
  **/
private boolean absoluteTolerance = true;

/** the next command to execute **/
private String cmd = null;

/** the number of errors found durig porocessing **/
private int errors=0;


/** the current yield state **/
private int yieldState = YIELD_UNKNOWN;

/** the currently expected result list **/
private ListExpr resultList = null;




/* constants for the yield states */
/** yield state constant for no comparison of results **/
private static final int YIELD_UNKNOWN = 0;
/** yield state expecting an error in execution of the command **/
private static final int YIELD_ERROR = 1;
/** yield state expecting success of the execution of the command **/
private static final int YIELD_SUCCESS = 2;
/** yield state expecting a result **/
private static final int YIELD_RESULT = 3;


/* come constants for the different testrunner  states */
private static final int START = 0;
private static final int SETUP = 1;
private static final int TESTCASE = 2;
private static final int TEARDOWN = 3;
private static final int STOP = 4;

/** symbol describing contineous lines **/
private static String contSymbol="\\";



/** creates a new TestRunner instance **/
public TestRunner(CommandPanel cp){
   this.cp = cp;
   init();
}

private String getStateAsString(){
 switch(state){
  case START : return "START";
  case SETUP : return "SETUP";
  case TESTCASE : return "TESTCASE";
  case TEARDOWN : return "TEARDOWN";
  case STOP : return "STOP";
  default : return "INVALID";
 }

}

/** initialized this instance **/
private void init(){
   state = START;
   tolerance = 0.0;
   absoluteTolerance = true;
   errors = 0;
   yieldState=YIELD_UNKNOWN;
   resultList = null;
}


/** executes the current command with the current state of the testrunner **/
private void execute(){
   if(cmd==null){
     Reporter.writeError("try to execute null command in testrunner ");
     errors++;
     return;
   }
   boolean isTest = yieldState!=YIELD_UNKNOWN && state==TESTCASE;
   if(!isTest){
      String info = "not a proper test: ";
      if(state!=TESTCASE){
          info += " not a testcase ";
      } else {
          info += "yieldstate unknown ";
      }
      Reporter.writeInfo(info);
   } 
   boolean success = (yieldState==YIELD_SUCCESS) || (yieldState==YIELD_RESULT);
   Reporter.writeInfo("execute command "+cmd);
   boolean ok = cp.execUserCommand(cmd,isTest,success,
                                      tolerance,absoluteTolerance, 
                                      resultList);
   Reporter.writeInfo("command finished");
  
   if(!ok){
      Reporter.writeError("command failed : " + cmd);
      if(isTest){
        errors++;
      }
   }
}


/** gets the next action from the input 
  * @return true if a command was found (not only state switching and so on)
  **/
private boolean nextCommand(BufferedReader in){
  String line;
  boolean complete;
  try{
    if(!in.ready()){ // file ends here
      state=STOP;
      return false;
    }  
    line = in.readLine();
    if(line==null){
       return false; // error in reading line
    }
    if(line.trim().length()<2){ // comment or empty line
       return false;
    }
    if(!line.startsWith("#")){ // secondo command
       // get the command
       if(line.endsWith(";")){ // single line command
         cmd = line.substring(0,line.length()-1); // remove the ';'
         return true;
       }
       // multi line command
       cmd = line +" ";
       while(in.ready() && line.trim().length()>0){
          line = in.readLine();
          if(line.length()>0){
             if(line.endsWith(";")){
                cmd += "\n" + line.substring(0,line.length()-1);
                return true; // command complete
             }
             cmd += "\n" + line; // add line to command
          }
       } 
       return true; // command complete
    } else{  // comment or directive 
        // connect all lines ending with contSymbol  
        boolean done = false;
        String connected="";  
        line = line.substring(1); // remove the '#'
        boolean first = true;
        while(!done){
          boolean cont = line.endsWith(contSymbol);
          done = !cont || !in.ready();
          if(cont){
              line = line.substring(0,line.length()-1); // remove the contSymbol
          } 
          if(first){
             connected = line+" ";
             first=false;
          } else{
             connected += "\n"+line;
          }
          if(!done){
              line = in.readLine();
          }
        }
        // connected holds all lines of the command
        java.util.StringTokenizer st = new java.util.StringTokenizer(connected);
        if(st.hasMoreElements()){
           String command = st.nextToken();
           String restOfLine;
           if(command.length()==connected.length()){
                restOfLine="";
           }else{
              restOfLine = connected.substring(command.length()+1);
           }
           if(command.equals("setup")){
              if(state!=START){
                  Reporter.writeError("try to switch to setup state when state != START");
                  Reporter.writeInfo("the currentState is "+getStateAsString()); 
                  Reporter.writeInfo("the complete line is "+connected); 
              } else{
                  state = SETUP;
                  Reporter.writeInfo("switch to setup: "+restOfLine);
              }
              return false;
           } else if(command.equals("stop")){
               state = STOP;
               Reporter.writeInfo("stop " + restOfLine);
               return false;
           } else if(command.equals("testcase")){
               state = TESTCASE;
               Reporter.writeInfo("switch to TestCase: "+restOfLine);
               return false;
           } else if(command.equals("tolerance_real")){
               // analyse the rest of the line
               restOfLine = restOfLine.trim();
               if(restOfLine.startsWith("%")){
                  Reporter.writeInfo("use relative tolerance");
                  absoluteTolerance=false; 
                  restOfLine = restOfLine.substring(1).trim();
               } else{
                  Reporter.writeInfo("use absolute tolerance");
                  absoluteTolerance=true;
               }
               try{
                  tolerance = Double.parseDouble(restOfLine);
               }catch(NumberFormatException e){
                   Reporter.writeError("invalid value for tolerance "+restOfLine);
                   tolerance=0;
                   errors++;
               } 
               return false;
           } else if(command.equals("yields")){
              if(state!=TESTCASE){ // invalid position for yields
                  Reporter.writeError("yields found in non-Testcase section");
                  errors++;
                  resultList=null;
                  return false;
              }
              restOfLine = restOfLine.trim();
              if(restOfLine.length()==0){ // no result given
                 Reporter.writeInfo("yields is unknown");
                 yieldState=YIELD_UNKNOWN;
                 resultList=null;
                 return false;
              }
              if(restOfLine.startsWith("@")){ // result given in file
                 restOfLine = restOfLine.substring(1);
                 Reporter.writeInfo("expect result in file "+restOfLine);
                 restOfLine = tools.CommonUtils.expandVar(restOfLine);
                 resultList = ListExpr.getListExprFromFile(restOfLine);
                 
                 if(resultList==null){
                     errors++; // file not found is an error
                     Reporter.writeError("cannot read ListExpr from file " + restOfLine);
                 }else {
                     Reporter.writeInfo("expected result is load from file "+restOfLine);
                 }
                 yieldState=YIELD_RESULT;
                 return false;
              }
              if(restOfLine.startsWith("(")){ // result given as string
                  resultList=new ListExpr();
                  if(resultList.readFromString(restOfLine)!=0){
                     Reporter.writeError("invalid List found as result "+ restOfLine);
                     resultList=null;
                  }
                  yieldState=YIELD_RESULT;
                  return false;
              }
              restOfLine=restOfLine.toLowerCase();
              if(restOfLine.indexOf("error")>=0){ // error expected
                  Reporter.writeInfo("expect an error");
                  yieldState=YIELD_ERROR;
                  resultList=null;
                  return false;
              }
              if(restOfLine.indexOf("success")>=0){ // success expected
                 Reporter.writeInfo("expect success");
                 yieldState=YIELD_SUCCESS;
                 resultList=null;
                 return false;
              }
              // no known command
              Reporter.writeError("Error in testrunner file\n"+
                                  "yields got an unknown argument :\n " + restOfLine);
              errors++; 
              yieldState=YIELD_UNKNOWN;
              resultList=null;
              return false;
           } else if(command.equals("teardown")){
              state=TEARDOWN;
              yieldState=YIELD_UNKNOWN;
              resultList=null;
              return false;
           }else{ // a comment
              return false;
           }
        } else{ // empty line
          return false;
        }
    }

  } catch(Exception e){
    Reporter.debug(e);
    errors++;
    return false; 
  } 
}


/** processes a testrunner file returning the number of errors occurred
  */
public int processFile(String fileName, boolean ignoreErrors){
   init();
   BufferedReader in=null;
   try{
      in = new BufferedReader(new FileReader(fileName));
      boolean done=false;
      while(in.ready()&&!done && state!=STOP){
          if(nextCommand(in)){
             execute();
          }
          if(!ignoreErrors&&errors>0){
             done=true;
          }
      }
   } catch(Exception  e){
       Reporter.debug(e);
       Reporter.writeError("error in processsing file "+fileName);
   }
   finally{
      if(in!=null){
        try{in.close();
           }catch(Exception e2){}
      }
   }
   return errors;
}



}



