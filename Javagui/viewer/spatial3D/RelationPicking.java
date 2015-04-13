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

package  viewer.spatial3D; 

import viewer.Spatial3DViewer;
import tools.Reporter;

import javax.media.j3d.*;
import com.sun.j3d.utils.picking.*;
import com.sun.j3d.utils.picking.behaviors.*;
import com.sun.j3d.utils.geometry.*;
import com.sun.j3d.utils.behaviors.mouse.*;

/**
 * This class is used to select the objects of a relation in the display.
 * It determines the index of the object which it habe in relation.
 */
public class RelationPicking extends PickMouseBehavior {
  
  private Spatial3DViewer parent;
  private TextWindow window;
  
  /**
   * constructor
   */
  public RelationPicking( Spatial3DViewer viewer, BranchGroup root, Canvas3D canvas, Bounds bounds) {
    super(canvas, root, bounds);
    parent=viewer;
    window=viewer.getTextWindow();
    setSchedulingBounds(bounds);
  }
  
 public void updateScene(int xpos,  int ypos) {
    Shape3D pickedShape = null;
    pickCanvas.setShapeLocation(xpos,ypos);
    //get object with pickresult
    PickResult pResult = pickCanvas.pickClosest();
    if (pResult != null) {
      pickedShape = (Shape3D)pResult.getNode(PickResult.SHAPE3D);
    }
    
    if (pickedShape !=null) {
      int data =(int) pickedShape.getUserData();            // TODO changed to int
      parent.setSelectedObject(data, false);
      if (mevent.getClickCount()==2) 
        window.showPropertiesDialog();
    }
    else
      parent.setSelectedObject(-1, true);                 // clear selection
  }
}