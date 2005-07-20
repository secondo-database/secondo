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

package gui;

import viewer.SecondoViewer;
import sj.lang.ListExpr;
import sj.lang.IntByReference;
import java.awt.*;

public interface ViewerControl{

 /** check if if the current viewer can display SO **/
 public boolean canActualDisplay(SecondoObject SO);

 /** check if SO displayed in current viewer **/
 public boolean isActualDisplayed(SecondoObject SO);


 /** give SO to the current viewer **/
 public boolean showObject(SecondoObject SO);

 /** sends the remove command to the current viewer **/
 public void hideObject(Object Sender,SecondoObject SO);


 /** invoked if a viewer removed SO */
 public void removeObject(SecondoObject SO);

 
/** a object is selected in Sender-Component */ 
public void selectObject(Object Sender,SecondoObject SO);


/** this method should be invoked from viewer if the menu is changed **/
public void updateMenu();


/** allow a Viewer to use Dialogs */
public Frame getMainFrame();

/** allow a object update or input new objects by a viewer */
public void updateObject(SecondoObject SO);

/** allow a viewer to add a object */
public boolean addObject(SecondoObject SO);


/** returns all loaded Viewers */
public SecondoViewer[] getViewers();

/** add a ViewerChangeListener */
public void addViewerChangeListener(ViewerChangeListener VCL);

/** remove a ViewerChangeListener */
public void removeViewerChangeListener(ViewerChangeListener VCL);

/** Sends the command to the secondo server.
  * The resulting list  of this command is ignored. 
  * The return value is the secondo client error code, e.g.
  * 0 if no error is occurred.
  **/
public int execCommand(String cmd);

/** Sends cmd to the secondo server. The resulting list
  * is returned from this method for postprocessing. If an
  * error occurs, the result will be null.
  **/
public ListExpr getCommandResult(String cmd);

/** Sends cmd to the secondo server. The command is
  * processed like a command input from the user.
  **/
public boolean execUserCommand(String cmd);

/** A general method for ececuting commands */
public boolean execCommand(String cmd, IntByReference errorCode, ListExpr resultList, StringBuffer errorMessage);
  

}

