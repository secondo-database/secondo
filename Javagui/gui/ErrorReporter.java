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

import javax.swing.JOptionPane;

/**
  * This class is the central instance for all kinds of outputs.
  * Within Secondo's Javagui programmers should not be use MessageBoxes 
  * or outputs to the console directly. Rathe them have to use the methods
  * from this class. 
  **/
public class ErrorReporter{


/** Reports an error. 
  * @param message: the error message
  * @param ex: the exception responsible for the error, may be null
  * @param debug: create message only in debug mode
  * @param console: do not create a graphical object for displaying the error
  * @param writeStackTrace: write the stacktrace
  **/
public static void reportError(String message,
                          Exception ex, 
                          boolean debug, 
                          boolean console,
                          boolean writeStackTrace){

 if(debug && !Environment.DEBUG_MODE){
    return;
 }
 if(!console){
    if(ex!=null){
       message = message+"\n"+ex.getMessage();
    }
    try{
      JOptionPane.showMessageDialog(null,message,
                               "An error is detected",
                                JOptionPane.ERROR_MESSAGE);
    } catch(Exception e){
      System.err.println("could not create error frame for \n"+message);
    }
   return;
 }
 // output to console
 System.err.println(message);
 if(writeStackTrace && ex!=null){
    ex.printStackTrace();
 } 
 if(ex==null && writeStackTrace){
    new Throwable().printStackTrace();
 }
} 


/** Reports an error. 
  * @param message: the error message
  * @param ex: the exception responsible for the error, may be null
  * @param debug: create message only in debug mode
  * @param console: do not create a graphical object for displaying the error
  * @param writeStackTrace: write the stacktrace
  **/
public static void reportWarning(String message, 
                            Exception ex, 
                            boolean debug,
                            boolean console,
                            boolean writeStackTrace){
 if(debug && !Environment.DEBUG_MODE){
    return;
 }
 if(!console){
    if(ex!=null){
       message = message+"\n"+ex.getMessage();
    }
    try{
      JOptionPane.showMessageDialog(null,message,
                               "Warning",
                                JOptionPane.INFORMATION_MESSAGE);
    } catch(Exception e){
      System.err.println("could not create error frame for \n"+message);
    }
   return;
 }
 // output to console
 System.err.println(message);
 if(writeStackTrace && ex!=null){
    ex.printStackTrace();
 } 
 if(ex==null && writeStackTrace){
    new Throwable().printStackTrace();
 }

}

  



}
