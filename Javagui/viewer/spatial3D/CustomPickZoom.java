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

import javax.media.j3d.Bounds;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Canvas3D;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;

import com.sun.j3d.utils.picking.behaviors.PickMouseBehavior;
import com.sun.j3d.utils.behaviors.mouse.MouseBehaviorCallback;
import com.sun.j3d.utils.behaviors.mouse.MouseZoom;
import com.sun.j3d.utils.picking.PickResult;
import com.sun.j3d.utils.picking.behaviors.PickingCallback;
import com.sun.j3d.utils.behaviors.mouse.MouseBehavior;


/**
 * A mouse behavior that allows user to pick and translate, rotate and zoom
 * scene graph objects.
 */

public class CustomPickZoom extends PickMouseBehavior implements
MouseBehaviorCallback {

  MouseZoom zoom;
  private PickingCallback callback = null;
  private TransformGroup currentTG;

/**
 * Creates a pick/zoom behavior that waits for user
 * mouse events for the scene graph.
 * 
 * @param root
 *            Root of your scene graph.
 * @param canvas
 *            Java 3D drawing canvas.
 * @param bounds
 *            Bounds of your scene.
 **/

  public CustomPickZoom(BranchGroup root, Canvas3D canvas,
    Bounds bounds) {
      super(canvas, root, bounds);
    
      // set WheelZoom
      zoom = new MouseZoom(MouseBehavior.MANUAL_WAKEUP);
      zoom.setTransformGroup(currGrp);
      currGrp.addChild(zoom);
      zoom.setSchedulingBounds(bounds);  
      this.setSchedulingBounds(bounds);

      }

  /**
   * Update the scene to manipulate any nodes. This is not meant to be called
   * by users. Behavior automatically calls this. You can call this only if
   * you know what you are doing.
   *
   * @param xpos
   *            Current mouse X pos.
   * @param ypos
   *            Current mouse Y pos.
   **/
  public void updateScene(int xpos, int ypos) {
    TransformGroup tg = null;
    if (!mevent.isControlDown() && mevent.isAltDown() && !mevent.isMetaDown()){
      pickCanvas.setShapeLocation(xpos, ypos);
      PickResult pr = pickCanvas.pickClosest();
      if ((pr != null)
        && ((tg = (TransformGroup) pr
          .getNode(PickResult.TRANSFORM_GROUP)) != null)
        && (tg.getCapability(TransformGroup.ALLOW_TRANSFORM_READ))
        && (tg.getCapability(TransformGroup.ALLOW_TRANSFORM_WRITE))) {
          zoom.setTransformGroup(tg);
          zoom.wakeup();
          currentTG = tg;
          
          } else if (callback != null)
      callback.transformChanged(PickingCallback.NO_PICK, null);
      }
    }

  /**
   * Callback method from MouseTranslate This is used when the Picking
   * callback is enabled
   */
  public void transformChanged(int type, Transform3D transform) {
    callback.transformChanged(PickingCallback.ZOOM, currentTG);
    }

  /**
   * Register the class @param callback to be called each time the picked
   * object moves
   */
  public void setupCallback(PickingCallback callback) {
    this.callback = callback;
    if (callback == null)
      zoom.setupCallback(null);
    else
      zoom.setupCallback(this);
    }
    
  public void setFactor(double factor) {
    zoom.setFactor(factor);
    }

  }