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

package viewer;

import gui.SecondoObject;
import gui.ViewerControl;
import javax.swing.JComponent;

public abstract class SecondoViewer extends JComponent{

 /** Get the name of this viewer.
   * The name is used in the menu of MainWindow.
   * @return the name of this viewer, used by MainWindow's title bar and menu.
   */
 public abstract String getName();

 /** Adds a <code>SecondoObject</code> to the viewer.
   * @param o the object to be added.
   * @return <code>true</code> if this viewer can display o otherwise <code>false</code>.
   */ 
 public abstract boolean addObject(SecondoObject o);

 /** Removes o from viewer if displayed. 
   * @param o the object to be removed.
  **/
 public abstract void removeObject(SecondoObject o);

/** Remove all objects from viewer.*/
public abstract void removeAll();

/** Check if this viewer can display o.
  *
  */
public abstract boolean canDisplay(SecondoObject o);

 /** check if o displayed in the moment **/
public abstract boolean isDisplayed(SecondoObject o);

/** hightlighting of o **/
 public abstract boolean selectObject(SecondoObject O);

 /** Get the MenuExtension for MainWindow.
  *  This method should be overwritten if there is need for an own menu.
  *  @return The menu vector; we return null as we don't have one here.
  */
 public MenuVector getMenuVector(){
     return null;
 }


 /** set the Control for this viewer **/
 public void setViewerControl(ViewerControl VC){
      this.VC = VC;
 }

 protected ViewerControl VC=null;  // inform this Control if select/remove a Object
 
 protected boolean DEBUG_MODE=false;
 
 // set the debug mode of this viewer
 public void setDebugMode(boolean on){
   DEBUG_MODE=on;
 }


 /** check if O is a SecondoViewer
   * if not false is returned
   * otherwise the names from this and O are
   * checked for equality */
 public boolean equals(Object O){
   if (!(O instanceof SecondoViewer))
      return false;
   else
      return getName().equals( ((SecondoViewer)O).getName());

 }

 /** returns the quality of view for a given object in range [0..1]
   * by this method it is possible to choose the best viewer for a given object
   * a viewer should overwrite this method
   * returns 0 if this viewer can't display SO and 1 if this viewer is
   * excellent appropriate to display this SecondoObject
   */
 public double getDisplayQuality(SecondoObject SO){
    if(canDisplay(SO))
       return 0.5;
    else
       return 0;
 }


}

