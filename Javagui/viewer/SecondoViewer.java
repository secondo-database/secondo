package viewer;

import gui.SecondoObject;
import gui.ViewerControl;
import javax.swing.JComponent;

public abstract class SecondoViewer extends JComponent{

 /** get the name of this viewer
   * this name is used in the menu of MainWindow
   **/
 public abstract String getName();

 /** adds a SecondoObject to the viewer
   * @param o the object to be added
   * @return true if this viewer can display o otherwise false
   **/ 
 public abstract boolean addObject(SecondoObject o);

 /** removes o from viewer if displayed **/
 public abstract void removeObject(SecondoObject o);

 /** check if this viewer can display o
   **/
 public abstract boolean canDisplay(SecondoObject o);

 /** check if o displayed in the moment **/
 public abstract boolean isDisplayed(SecondoObject o);

/** hightlighting of o **/
 public abstract boolean selectObject(SecondoObject O);

 /** get the MenuExtension for MainWindow
   *
   **/
 public abstract MenuVector getMenuVector();


 /** set the Control for this viewer **/
 public void setViewerControl(ViewerControl VC){
      this.VC = VC;
 }

 protected ViewerControl VC=null;  // inform this Control if select/remove a Object


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


}
