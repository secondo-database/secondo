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

/** remove all containing objects */
public abstract void removeAll();

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

