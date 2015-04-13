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

import tools.Reporter;
import java.awt.Color;
import java.awt.Component;
import java.awt.AWTEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;

import javax.media.j3d.BoundingSphere;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Group;
import javax.media.j3d.Light;
import javax.media.j3d.PointLight;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.WakeupCriterion;
import javax.media.j3d.WakeupOnAWTEvent;
import javax.media.j3d.WakeupOnBehaviorPost;

import javax.vecmath.Color3f;
import javax.vecmath.Point3d;
import javax.vecmath.Vector3d;

import com.sun.j3d.utils.behaviors.mouse.MouseBehavior;
import com.sun.j3d.utils.behaviors.mouse.MouseBehaviorCallback;

import java.util.Enumeration;


/**
 * A class for lighting for Spatial3dViewer
 * with brightness regulator
 *
 */

public class LightBranchGroup extends BranchGroup {
	 
  public static final int  MAX_LEVEL = 6; // range 0-5
  
  
  /**
   * a private internal class of mouse behavior modified from 
   * 
   *  MouseTranslation.java, MouseZoom.java, MouseWheelZoom.java of Sun Microsystems 
   *  
   *  Copyright (c) 2007 Sun Microsystems, Inc. All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions
   * are met:
   *
   * - Redistribution of source code must retain the above copyright
   *   notice, this list of conditions and the following disclaimer.
   *
   * - Redistribution in binary form must reproduce the above copyright
   *   notice, this list of conditions and the following disclaimer in
   *   the documentation and/or other materials provided with the
   *   distribution.
   *
   * Neither the name of Sun Microsystems, Inc. or the names of
   * contributors may be used to endorse or promote products derived
   * from this software without specific prior written permission.
   *
   * This software is provided "AS IS," without a warranty of any
   * kind. ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND
   * WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,
   * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY
   * EXCLUDED. SUN MICROSYSTEMS, INC. ("SUN") AND ITS LICENSORS SHALL
   * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF
   * USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
   * DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR
   * ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL,
   * CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND
   * REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF OR
   * INABILITY TO USE THIS SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
   * POSSIBILITY OF SUCH DAMAGES.
   *
   * You acknowledge that this software is not designed, licensed or
   * intended for use in the design, construction, operation or
   * maintenance of any nuclear facility.
   */
  private class LightMouseBehavior extends MouseBehavior {
	  
    private LightPanel panel;
    private double x_factor = 1.0;
    private double y_factor = 1.0;
    private double z_factor = 1.0;
    
    private MouseBehaviorCallback callback = null;
    
    public LightMouseBehavior(TransformGroup transformGroup, LightPanel _panel) {
      super(transformGroup);
      panel = _panel;
    }
    
    public LightMouseBehavior(){
      super(0);
      panel = null;
    }
    
    /*
    public LightMouseBehavior(int flags) {
      super(flags);
    }
    
    public LightMouseBehavior(Component c) {
      super(c, 0);
    } 
    
    public LightMouseBehavior(Component c, TransformGroup transformGroup) {
      super(c, transformGroup);
    }
    
    public LightMouseBehavior(Component c, int flags) {
      super(c, flags);
    }
    */
    
    @Override
    public void initialize() {
      super.initialize();
      if ((flags & INVERT_INPUT) == INVERT_INPUT) {
      
        x_factor *= -1;
        y_factor *= -1;
        z_factor *= -1;
        invert = true;
      }
    }
    
    @Override
    public void processStimulus (Enumeration criteria) {
      WakeupCriterion wakeup;
      AWTEvent[] events;
      MouseEvent evt;
      
      while (criteria.hasMoreElements()) {
        wakeup = (WakeupCriterion) criteria.nextElement();
        if (wakeup instanceof WakeupOnAWTEvent) {
          events = ((WakeupOnAWTEvent)wakeup).getAWTEvent();
          if (events.length > 0) {
            evt = (MouseEvent) events[events.length-1];
            doProcess(evt);
          }
        }  
        else if (wakeup instanceof WakeupOnBehaviorPost) {
          while (true) {
            synchronized (mouseq) {
              if (mouseq.isEmpty()) break;
              evt = (MouseEvent)mouseq.remove(0);
              while ((evt.getID() == MouseEvent.MOUSE_DRAGGED) && !mouseq.isEmpty() && (((MouseEvent)mouseq.get(0)).getID() == MouseEvent.MOUSE_DRAGGED)) {
                evt = (MouseEvent)mouseq.remove(0);
              }
            }
            doProcess(evt);
          }
        }
      }
      wakeupOn(mouseCriterion);
    }
    
    void doProcess(MouseEvent evt) {
      int id;
      int dx, dy;
      int units = 0;
      Vector3d translation = new Vector3d();

      processMouseEvent(evt);
      
      if (((buttonPress) && ((flags & MANUAL_WAKEUP) == 0)) || ((wakeUp)&&((flags & MANUAL_WAKEUP) != 0))){
        id = evt.getID();
        
        // x and y direction
        if (evt.isControlDown() && (id == MouseEvent.MOUSE_DRAGGED) && !evt.isAltDown() && !evt.isMetaDown()) {      
          x = evt.getX();
          y = evt.getY();
          dx = x - x_last;
          dy = y - y_last;
          if ((!reset)/* && ((Math.abs(dy) < 50) && (Math.abs(dx) < 50))*/) {
            // Reporter.writeInfo("Mousetranslate dx: " + dx + " dy: " + dy);
            transformGroup.getTransform(currXform);
            translation.x =  dx * x_factor*panel.getXChangeFactor(); 
            translation.y = -dy * y_factor*panel.getYChangeFactor();
            transformX.set(translation);
            if (invert) {
              currXform.mul(currXform, transformX);
            } else {
              currXform.mul(transformX, currXform);
            }
            transformGroup.setTransform(currXform);
            transformChanged( currXform );
            if (callback!=null)  callback.transformChanged( MouseBehaviorCallback.TRANSLATE, currXform );          
            
            Vector3d trans = new Vector3d();
            currXform.get(trans);
            if(panel != null){
              panel.setX(trans.x);
              panel.setY(trans.y);
            }
            
          }else {
            reset = false;
          }
          x_last = x;
          y_last = y;
        }
        
        // z direction
        else if (evt.isControlDown() && (id == MouseEvent.MOUSE_DRAGGED) && !evt.isMetaDown() && evt.isAltDown()){
          x = evt.getX();
          y = evt.getY();
          dx = x - x_last;
          dy = y - y_last;
          if (!reset){
            
            // Reporter.writeInfo("Mousezoom dx: " + dx + " dy: " + dy);
            
            transformGroup.getTransform(currXform);
            translation.z  = dy*z_factor*panel.getZChangeFactor();
            transformX.set(translation);
            if (invert) {
              currXform.mul(currXform, transformX);
            } else {
              currXform.mul(transformX, currXform);
            }
            transformGroup.setTransform(currXform);
            transformChanged( currXform );
            if (callback!=null)callback.transformChanged( MouseBehaviorCallback.ZOOM,currXform );
            
            Vector3d trans = new Vector3d();
            currXform.get(trans);
            if(panel !=null)panel.setZ(trans.z);
            
          }else {
            reset = false;
          }
          x_last = x;
          y_last = y;
        
        // mouse wheel movement -> z direction
        }else if (evt.isControlDown() && (id == MouseEvent.MOUSE_WHEEL) && !evt.isMetaDown() && evt.isControlDown()) {
          MouseWheelEvent wheelEvent = (MouseWheelEvent)evt;
          if (wheelEvent.getScrollType() == wheelEvent.WHEEL_UNIT_SCROLL ) {
            units = wheelEvent.getUnitsToScroll();
          }
          if (!reset) {
            
            // Reporter.writeInfo("MousewheelZoom");
            transformGroup.getTransform(currXform);
            translation.z  = units*z_factor*panel.getZChangeFactor();
            transformX.set(translation);
            if (invert) {
              currXform.mul(currXform, transformX);
            } else {
              currXform.mul(transformX, currXform);
            }
            transformGroup.setTransform(currXform);
            transformChanged( currXform );
            if (callback!=null)  callback.transformChanged( MouseBehaviorCallback.ZOOM,currXform );
            
            Vector3d trans = new Vector3d();
            currXform.get(trans);
            if(panel !=null)panel.setZ(trans.z);
            
          }else {
            reset = false;
          }
        }else if (evt.isControlDown() && id == MouseEvent.MOUSE_PRESSED) {
          // Reporter.writeInfo("Mouse pressed");
          
          x_last = evt.getX();
          y_last = evt.getY();
        }
      }
    }
    public void transformChanged( Transform3D transform ) {

    }
    
    public void setupCallback( MouseBehaviorCallback callback ) {
      this.callback = callback;
    }
    

  }
  
  private BoundingSphere area;
  private Color3f color;
  private TransformGroup tg;
  private LightMouseBehavior behavior;
  
  private int brightness = 0;
  
  // constructor
  public LightBranchGroup(LightPanel panel){
    
    super();
    color = new Color3f(1.0f, 1.0f, 1.0f);
    area = new BoundingSphere(new Point3d(0.0,0.0,0.0),panel.STARTING_RADIUS);
    tg = new TransformGroup();
    tg.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
    
    // create all lightis here
    for(int i=0; i<MAX_LEVEL ; i++){
      PointLight light = new PointLight();
      light.setCapability(Light.ALLOW_INFLUENCING_BOUNDS_READ);
      light.setCapability(Light.ALLOW_INFLUENCING_BOUNDS_WRITE);
      light.setCapability(Light.ALLOW_COLOR_READ);
      light.setCapability(Light.ALLOW_COLOR_WRITE);
      light.setCapability(Light.ALLOW_STATE_READ);
      light.setCapability(Light.ALLOW_STATE_WRITE);
      light.setInfluencingBounds(area);
      light.setEnable(false);
      tg.addChild(light);
    }
    setBrightness(MAX_LEVEL/2);
    
    // add behavior
    behavior = new LightMouseBehavior(tg, panel);
    behavior.setSchedulingBounds(area);
    behavior.setEnable(false);
    this.addChild(behavior);
    this.addChild(tg);
    
    setCapability(Group.ALLOW_CHILDREN_READ);
    setCapability(Group.ALLOW_CHILDREN_WRITE);
    setCapability(BranchGroup.ALLOW_DETACH);
    compile();
  }
  
  /**
   * increase brightness
   */
  public void increase(){
    if(brightness<MAX_LEVEL-1){
      ((PointLight)tg.getChild(brightness)).setEnable(true);
      brightness++;
    }
  }
  
  /**
   * decrease brightness
   */
  public void decrease(){
    if(brightness>0){
      ((PointLight)tg.getChild(brightness)).setEnable(false);
      brightness--;
    }
  }
  
  /**
   * getter brightness
   * @return brighness in int
   */
  public int getBrightness(){
    return brightness;
  }
  
  /**
   * setter brightness
   * @param brighness in int
   */
  public void setBrightness(int _brightness){
    if(_brightness > MAX_LEVEL-1|| _brightness < 0) return;
    
    for(int i = 0; i < _brightness ; i++){
      ((PointLight)tg.getChild(i)).setEnable(true);      
    }
    for(int i = _brightness; i < MAX_LEVEL ; i++){
      ((PointLight)tg.getChild(i)).setEnable(false);      
    }
    brightness = _brightness;
  }
  
  /**
   * set wheather the position can be changable
   * @param enable as boolean
   */
  public void setRemovable(boolean enable){
    behavior.setEnable(enable);
  }
  
  /**
   * set Light area
   * @param radius as double
   */
  public void setArea(double _radius){
    area.setRadius(_radius);
    behavior.setSchedulingBounds(area);
    for(int i = 0; i < MAX_LEVEL; i++){
      ((PointLight)tg.getChild(i)).setInfluencingBounds(area);
    }
  }
  
  /**
   * 
   * @return radius of light reachable area in double
   */
  public double getArea(){
    return area.getRadius();
  }
  
  /**
   * switch on/off function
   * @param on
   */
  public void setEnable(boolean on){    
    for(int i=0; i<=brightness; i++){
      ((PointLight)tg.getChild(i)).setEnable(on);
    }
  }
  
  /**
   * check if the light is on
   * @return return true if at least one of the PointLight is enabled, else false 
   */
  public boolean getEnable(){
    for(int i=0; i<MAX_LEVEL ; i++){
      if (((PointLight)tg.getChild(i)).getEnable()) return true;
    }
    return false; 
  }
  
  /**
   * @return color of the light
   */
  public Color getColor(){
    return color.get();
  }
  
  /**
   * @param color to be set
   */
  public void setColor(Color c){
    Color3f newColor = new Color3f(c);
    color = newColor;
    for(int i=0; i<MAX_LEVEL; i++){
      ((PointLight)tg.getChild(i)).setColor(color);
    }
  }
  
  /**
   * @return current position in Point3d
   */
  public Point3d getPosition(){
    Vector3d dif = new Vector3d();
    Transform3D trans = new Transform3D();
    tg.getTransform(trans);    
    trans.get(dif);
    Point3d ret = new Point3d((float)(dif.x),(float)(dif.y),(float)(dif.z));
    return ret;
  }
  
  /**
   * @param to be set position in Point3d
   */
  public void setPosition(Point3d newPos){
    
    Point3d currentPos = getPosition();
    Point3d dif = new Point3d();
    dif.sub(newPos,currentPos);
    /* Reporter.writeInfo("Current Position: x: " + currentPos.x + " y: " + currentPos.y + " z: " + currentPos.z);
    Reporter.writeInfo("new Position: x: " + newPos.x + " y: " + newPos.y + " z: " + newPos.z);
    Reporter.writeInfo("difference: x: " + dif.x + " y: " + dif.y + " z: " + dif.z);
    */
    Transform3D trans = new Transform3D();
    tg.getTransform(trans);
    trans.set(new Vector3d(newPos));
    tg.setTransform(trans);
  }
  
  public void setX(double value){
      Point3d pos = getPosition();
      pos.x = value;
      setPosition(pos);
  }
  public void setY(double value){
      Point3d pos = getPosition();
      pos.y = value;
      setPosition(pos);
  }
  public void setZ(double value){
      Point3d pos = getPosition();
      pos.z = value;
      setPosition(pos);
  }
  public void setMouseMoveEnable(boolean enable){
    behavior.setEnable(enable);
  }
  public boolean getMouseMoveEnable(){
    return behavior.getEnable();
  }
  
}
