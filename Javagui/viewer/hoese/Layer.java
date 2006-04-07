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


package  viewer.hoese;

import  java.awt.*;
import  java.awt.event.*;
import  javax.swing.*;
import  java.awt.geom.*;
import  java.util.*;
import  javax.swing.border.*;
import viewer.HoeseViewer;
import java.awt.image.*;
import tools.Reporter;

/**
 * A Swing JComponent that represent a Layer in the layerstack
 */
public class Layer extends JComponent {
  /** All the graph. objects that are accociated with this Layer */
  private Vector GeoObjects;
  /** The object that mangages the layer */
  private GraphWindow owner;
  /** This butto controls the on/off status */
  public JToggleButton button;
  /** the world boundingbox of this  layer */
  Rectangle2D.Double boundsWC;
  /** True if the selected object is in this layer */
  boolean Selected = false;
  /** the internal no. of this layer */
  int LayerNo;
  Category LastCat = null;
  DsplGraph LastDisplayGraph = null;

  /**
   * Default Construktor
   * @see <a href="Layersrc.html#Layer1">Source</a>
   */
  public Layer () {
    GeoObjects = new Vector(20);
  }

  /**
   * Construktor for a layer, which should display the objects in obj within owner gw
   * @param   Vector obj
   * @param   GraphWindow gw
   * @see <a href="Layersrc.html#Layer2">Source</a>
   */
  public Layer (Vector obj, GraphWindow gw) {
    owner = gw;
    GeoObjects = (Vector)obj.clone();
    setDoubleBuffered(true);
    calcBounds();
  }

  /** Calculates world-bounding  box for this layer
   * @see <a href="Layersrc.html#calcBounds">Source</a>
   */
  public void calcBounds () {
    boundsWC=null;
    try{
      ListIterator li = GeoObjects.listIterator();
      while (li.hasNext()) {
        Rectangle2D.Double bds = null;            // null sp. entf
        DsplGraph dg = ((DsplGraph)li.next());
        dg.setLayer(this);
        bds = dg.getBounds();
        if(bds != null){ // not an empty object
           if (boundsWC == null)
             boundsWC = bds;
           else
             boundsWC = (Rectangle2D.Double)boundsWC.createUnion(bds);
        }
      }
    }
    catch(Exception e){
      Reporter.writeError("Exception in Layer.calcBounds "+e);
      Reporter.debug(e);
    }
  }

  /**
   * Removes a graph. object from this layer
   * @param dg A graph. object
   * @see <a href="Layersrc.html#removeGO">Source</a>
   */
  public void removeGO (DsplGraph dg) {
    if(dg!=null){
       GeoObjects.remove(dg);
       calcBounds();
    }
  }

  /**
   *Adds a graph. object to this layer at position index
   * @param index if ==-1 then dg is added at the end
   * @param dg The object to add
   * @see <a href="Layersrc.html#addGO">Source</a>
   */
  public void addGO (int index, DsplGraph dg) {
      if(dg!=null){
      if ((index < 0) || (index >= GeoObjects.size()))
        GeoObjects.add(dg);
      else
        GeoObjects.add(index, dg);
      calcBounds();
    }
  }

  /**
   * Sets whether the button should be set as selected or not.
   * @param b True when button should be selected
   * @see <a href="Layersrc.html#setSelectedButton">Source</a>
   */
  public void setSelectedButton (boolean b) {
    Selected = b;
    button.repaint();
  }

  /**
   * Calculates an unique index for an object in the layerlist, so that its height is comparable to
   * other objects height in the layer-stack
   * @param dg A graph. object
   * @return A double value
   * @see <a href="Layersrc.html#getObjIndex">Source</a>
   */
  public double getObjIndex (DsplGraph dg) {
    int index = GeoObjects.indexOf(dg);
    int size = GeoObjects.size();
    if (index < 0)
      index = size++;
    return  LayerNo + (double)index/(double)size;
  }

  /**
   * Gets the actual transformation, which is the parent transformation
   * @return Transformation
   * @see <a href="Layersrc.html#getProjection">Source</a>
   */
  public AffineTransform getProjection () {
    return  owner.getProjection();
  }

  /**
   *
   * @return the world-boundinbox of this layer.
   * @see <a href="Layersrc.html#getWorldCoordBounds">Source</a>
   */
  public Rectangle2D.Double getWorldCoordBounds () {
    return  boundsWC;
  }

  /**
   * Creates the Layerswichbutton for turning on/off
   * @param al The ActionListener that manages the layer-toggling
   * @param lnr The intern layer nr
   * @return A new Switchbutton
   * @see <a href="Layersrc.html#CreateLayerButton">Source</a>
   */
  public JToggleButton CreateLayerButton (ActionListener al, int lnr) {
    LayerNo = lnr;
    button = new LayerToggle();
    button.setSelected(true);
    button.setAlignmentX(Component.CENTER_ALIGNMENT);
    //button.setBorder(new LineBorder(Color.blue,1));
    button.setPreferredSize(new Dimension(10, 10));
    //button.setSize(10,10);
    button.addActionListener(al);
    button.setActionCommand(Integer.toString(lnr));
    return  button;
  }

  /**
   *
   * @return The list of geograph. objects in this layeer
   * @see <a href="Layersrc.html#getGeoObjects">Source</a>
   */
  public Vector getGeoObjects () {
    return  GeoObjects;
  }

  /**
   *
   * @return The application's actual time
   * @see <a href="Layersrc.html#getActualTime">Source</a>
   */
  public double getActualTime () {
    return  CurrentState.ActualTime;
  }


  /** set a new Category to paint the next object */
  private void setCategory(DsplGraph dg,Graphics2D g2){
    if(dg==null) return;
    if(g2==null) return;
    Category Cat = dg.getCategory();
    if(Cat==null) return;

    LastCat=Cat;
    LastDisplayGraph = dg;
    AffineTransform af2 = getProjection();
    Shape render = dg.getRenderObject(af2);
    if(render==null)
       return;
    Shape sh = af2.createTransformedShape(render);
    if(sh==null) return;


    if(!dg.isLineType()){
       g2.setComposite(Cat.getAlphaStyle());
    } else{
      g2.setComposite(HoeseViewer.emptyStyle);
    }
    g2.setStroke(Cat.getLineStroke(dg.getRenderAttribute(),CurrentState.ActualTime));
    Paint P = Cat.getFillStyle(dg.getRenderAttribute(),CurrentState.ActualTime);
    g2.setPaint(P);
    if(P==null) return;
    if (dg.isPointType()) {
      if (P instanceof TexturePaint){
        BufferedImage bi = ((TexturePaint)P).getImage();
        if(bi==null) return;
        g2.setPaint(new TexturePaint(bi,sh.getBounds2D()));
      }
      else if (P instanceof GradientPaint)
        g2.setPaint(new GradientPaint((float)sh.getBounds().getX(),(float)sh.getBounds().getY(),
          ((GradientPaint)P).getColor1(),(float)(sh.getBounds().getX()+sh.getBounds().getWidth()),
          (float)(sh.getBounds().getY()+sh.getBounds().getHeight()),((GradientPaint)P).getColor2(),false));
    }
    

  }


  /**
   * Paints the layer with its graph. object. The selected object is not drawn here.
   * @param g The graphic context
   * @see <a href="Layersrc.html#paintComponent">Source</a>
   */
  public void paintComponent (Graphics g) {
    //ListIterator li = GeoObjects.listIterator();
    Graphics2D g2 = (Graphics2D)g;
    //g2.transform(owner.getProjection());
    //g2.setFont (font.deriveFont((float)(12.0/owner.getProjection().getScaleX())));
    try{
      LastDisplayGraph = null;
      if (Selected)
        for(int i=0;i<GeoObjects.size();i++){
      DsplGraph dg = (DsplGraph)GeoObjects.get(i);
          if ((dg.getVisible()) && (!dg.getSelected())){
        setCategory(dg,g2);
            dg.draw(g2,CurrentState.ActualTime);
      }
        }
      else
        for(int i=0;i<GeoObjects.size();i++){
          DsplGraph dg = (DsplGraph)GeoObjects.get(i);
          if (dg.getVisible()){
            setCategory(dg,g2);
            dg.draw(g2,CurrentState.ActualTime);
          }
        }
     } catch(Exception e){
       Reporter.writeError("Exception "+e);
       Reporter.debug(e);
     }

  }
  /** A special JToggleButton, that draws a selection area
   * @see <a href="Layersrc.html#LayerToggle">Source</a>
   */
  class LayerToggle extends JToggleButton {

    public void paintComponent (Graphics g) {
      if (isSelected())
        g.setColor(Color.green);
      else
        g.setColor(Color.lightGray);
      Insets insets = getInsets();
      int currentWidth = getWidth();            //- insets.left - insets.right;
      int currentHeight = getHeight();          // - insets.top - insets.bottom;
      g.fillRect(0, 0, currentWidth, currentHeight);
      if (Selected) {
        g.setColor(Color.red);
        g.fillRect(3, 3, 3, 4);
      }

    }
  }
}



