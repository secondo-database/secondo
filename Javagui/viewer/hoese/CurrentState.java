

package viewer.hoese;

import java.awt.geom.*;

/**
This class holds the current configuration of the HoeseViewer.


**/

public class CurrentState{


public static double ActualTime=0.0;


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

private static Rectangle2D.Double worldBB = new Rectangle2D.Double(-50000,-50000,100000,100000);

}
