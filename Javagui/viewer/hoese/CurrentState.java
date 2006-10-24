

package viewer.hoese;

import java.awt.geom.*;
import tools.Reporter;

/**
This class holds the current configuration of the HoeseViewer.


**/

public class CurrentState{


public static double ActualTime=0.0;

public static AffineTransform transform= new AffineTransform();


public static Rectangle2D.Double getWorldBB(){
     return worldBB;
}

public static void setWorldBB(Rectangle2D.Double box){
   if(box!=null){
     worldBB=box;
   } else{
      tools.Reporter.debug("try to set null as world bounding box");
   }
}

/** returns the currently used selection mode for categories.
**/
public static int getCatSelectionMode(){
   return catSelectionMode;
}

/** Sets the category selection mode. 
  * If the given value does not represent a valid selection mode,
  * an error message will be written to the console and the call
  * will not gave any effect.
**/
public static void setCatSelectionMode(int mode){
  if( (mode<CATEGORY_MANUAL) || (mode > CATEGORY_BY_NAME)){
     Reporter.writeError("try to select an invalid category selection mode");
     return;
  }
  catSelectionMode = mode;
}


/*
Symbolic names for differnt ways to select the category.

*/
/** This constant can be used to select each category manually.
  **/
public static final int CATEGORY_MANUAL = 0;

/** If this constant is used, for each graphically attribute, a
    new category is computed.
*/

public static final int CATEGORY_AUTO   = 1;

/** If a category exists having the same name as the attribute,
  * this category will be used. Otherwise the category is to select
  * manually.
  */
public static final int CATEGORY_BY_NAME = 2;


private static Rectangle2D.Double worldBB = new Rectangle2D.Double(-50000,-50000,100000,100000);
private static int catSelectionMode = CATEGORY_MANUAL;

}
