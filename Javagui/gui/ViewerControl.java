package gui;

import java.awt.*;
import viewer.SecondoViewer;
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

}
