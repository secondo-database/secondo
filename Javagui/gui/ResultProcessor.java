package gui;

import sj.lang.ListExpr;
import sj.lang.IntByReference;
import java.awt.Dimension;

interface ResultProcessor{

 /** process the result of a query **/
 public void processResult(String command,
                        ListExpr ResultList,
                        IntByReference ErrorCode,
                        IntByReference ErrorPos,
                        StringBuffer ErrorMessage);

 /** executes a command - result is ignored **/
 public int internCommand(String cmd);

 /** executes a guiCommand */
 public boolean execGuiCommand(String command);

 public Dimension getSize();
}

