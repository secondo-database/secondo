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

package tools;

import javax.swing.JOptionPane;

/**
  * This class is the central instance for all kinds of outputs.
  * Within Secondo's Javagui programmers should not be use MessageBoxes 
  * or outputs to the console directly. Rathe them have to use the methods
  * from this class. 
  **/
public class Reporter{


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

 if(debug && !gui.Environment.DEBUG_MODE){
    return;
 }
 if(!console && gui.Environment.TESTMODE==gui.Environment.NO_TESTMODE){
    if(ex!=null){
       message = message+"\n"+ex.getMessage();
    }
    try{
      JOptionPane.showMessageDialog(null,message,
                               "An error is detected",
                                JOptionPane.ERROR_MESSAGE);
    } catch(Exception e){
      TextFormat.printError("could not create error frame for \n"+message);
    }
   return;
 }
 // output to console
 TextFormat.printError(message);
 if(writeStackTrace && ex!=null){
    ex.printStackTrace();
 } 
 if(ex==null && writeStackTrace){
    new Throwable().printStackTrace();
 }
} 


/** Reports a warning. 
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
 if(debug && !gui.Environment.DEBUG_MODE){
    return;
 }
 if(!console && gui.Environment.TESTMODE==gui.Environment.NO_TESTMODE){
    if(ex!=null){
       message = message+"\n"+ex.getMessage();
    }
    try{
      JOptionPane.showMessageDialog(null,message,
                               "Warning",
                                JOptionPane.WARNING_MESSAGE);
    } catch(Exception e){
      TextFormat.printError("could not create error frame for \n"+message);
    }
   return;
 }
 // output to console
 TextFormat.printWarning(message);
 if(writeStackTrace && ex!=null){
    ex.printStackTrace();
 } 
 if(ex==null && writeStackTrace){
    new Throwable().printStackTrace();
 }

}

 
/** reports an Information **/
public static void reportInfo(String message, boolean console){
   if(console || gui.Environment.TESTMODE!=gui.Environment.NO_TESTMODE){
     TextFormat.printInfo(message);
   }else{
    try{
      JOptionPane.showMessageDialog(null,message,
                               "Information",
                                JOptionPane.INFORMATION_MESSAGE);
    } catch(Exception e){
      TextFormat.printError("could not create error frame for \n"+message);
    }
   }    
}

/** writes an information to Console **/
public static void writeInfo(String message){
   reportInfo(message,true);
}

/** writes an information in a frame **/
public static void showInfo(String message){
    reportInfo(message,false);
}

 

/** writes a simple error message to the console **/
public static void writeError(String message){
    reportError(message,null,false,true,false);
}

/** writes a simple warning message to the console **/
public static void writeWarning(String message){
    reportWarning(message,null,false,true,false);
}


/** Shows an error message within a graphical window **/
public static void showError(String message){
   reportError(message,null,false,false,false);
}


/** Shows an warning  message within a graphical window **/
public static void showWarning(String message){
   reportWarning(message,null,false,false,false);
}

/** prints out the stacktrace of ex when DEBUG_MODE
  * is enabled.
  **/
public static void debug(Exception e){
   if(gui.Environment.DEBUG_MODE){
     e.printStackTrace();
   }
}

/** Prints out the message and if DEBUG_MODE is activated also
  * the stacktrace of ex 
  **/
public static void debug(String message, Exception e){
  reportError(message,e,false,true,gui.Environment.DEBUG_MODE);   
}

/** prints the message as error when debug mode is enabled **/
public static void debug(String message){
  reportError(message,null,true,true,false);
}

/** Prints out a Stacktrace **/
public static void printTrace(){
   new Throwable().printStackTrace();
}

/** Writes the string to std out **/
public static void write(String message){
   System.out.println(message);
}

public static int showQuestion(String ASK){
  int res = JOptionPane.showConfirmDialog(null,ASK,null,
            JOptionPane.YES_NO_OPTION,JOptionPane.QUESTION_MESSAGE);
  if(res==JOptionPane.YES_OPTION)
     return YES;
  if(res==JOptionPane.NO_OPTION)
    return NO;
  return ERROR;
 }

public static final int YES = 0;
public static final int NO = 1;
public static final int ERROR = -1;


}
