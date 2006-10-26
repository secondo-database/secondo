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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.geom.*;
import java.util.*;
import viewer.HoeseViewer;
import javax.swing.border.*;
import java.awt.image.BufferedImage;
import tools.Reporter;

/**
 * This class implements the layer-stack for the graphical objects. The super class is a 
 * JLayeredPane, which offers a lot of functionality needed here.
 */
public class GraphWindow extends JLayeredPane
    implements Cloneable {
/** The internal no. ob the highest layer */
  int highest = 0;
/** Listens for events send by the layer-buttons */
  ActionListener LayerButtonListener;
/** Main-application */
  HoeseViewer mw;

/** a scalable image as background */
 ScalableImage background=new ScalableImage();

/** a flag for ignoring a repaint message.
    This can be used to make all changes without new drawing. 
**/
   private boolean ignorePaint = false; 
    

/** a additional object for  painting **/
   DsplGraph  additionalGraphObject;


  /** Creates a Graphwindow without any layer
   * @see <a href="Categorysrc.html#GraphWindow">Source</a>
   */
  public GraphWindow (HoeseViewer main) {
    super();
    mw = main;
    setDoubleBuffered(true);
    LayerButtonListener = new java.awt.event.ActionListener() {

      /**
       * Shows /hides the layer associatd with the layer-button responsable for this event
       * @param evt
       * @see <a href="Categorysrc.html#actionPerformed">Source</a>
   */
      public void actionPerformed (java.awt.event.ActionEvent evt) {
        int laynr = Integer.parseInt(evt.getActionCommand());
        Component[] com = getComponentsInLayer(laynr);
        if(com.length>0)
           ((JComponent)com[0]).setVisible(((JToggleButton)evt.getSource()).isSelected());
      }
    };
    // add a switch for the background
    //JToggleButton jt = new JToggleButton();
    //jt.setSelected(true);
    //jt.setAlignmentX(Component.CENTER_ALIGNMENT);
    //jt.setPreferredSize(new Dimension(10, 10));
    //jt.addActionListener(LayerButtonListener);
    //jt.setActionCommand("-1");
    //mw.addSwitch(jt,0);
    add(background,new Integer(-1));
  }


  /**
   * Gets the applications transformation
   * @return The actual Transformation
   * @see <a href="Categorysrc.html#getProjection">Source</a>
   */
  public AffineTransform getProjection () {
    return  CurrentState.transform;
  }

 /** Set the flag for switching off repainting of the screen.
   **/
   public void setIgnorePaint(boolean enabled){
       ignorePaint=enabled;
   }

  /** sets a new shape for painting */
  public void paintAdditional(DsplGraph g){
      additionalGraphObject=g;
  }




  /**
   * This method creates a random category, used when automatic category is selected in the menu
   * A manual object-category association is not neccessaary.
   * @return A Category with name AutoXX, where XX is a running no.
   * @see <a href="Categorysrc.html#createAutoCat">Source</a>
   */
  public Category createAutoCat () {
    Category defCat = new Category();
    int r, g, b;
    /*        r=(int)(Math.random()*256);
     g=(int)(Math.random()*256);
     b=(int)(Math.random()*256);
     defCat.setLineColor(new Color(r,g,b));* @see <a href="Categorysrc.html#GraphWindow">Source</a>
   */
    defCat.setLineColor(Color.black);
    defCat.setName("Auto" + Integer.toString(CategoryEditor.CpCnt++));
    defCat.setLineWidth((float)Math.abs(Math.random()*5.0f + 0.5f));
    defCat.setLineStyle((int)(Math.random()*7));
    r = (int)(Math.random()*256);
    g = (int)(Math.random()*256);
    b = (int)(Math.random()*256);
    defCat.setFillStyle(new Color(r, g, b));
    defCat.setAlphaStyle(AlphaComposite.getInstance(AlphaComposite.SRC_OVER));
    defCat.setPointSize((int)(Math.random()*15 + 6));
    defCat.setPointasRect((Math.random() > 0.5));
    mw.Cats.add(defCat);
    return  defCat;
  }

  /**
   * Adds the graph. objects in Vector grob to a new layer on the top
   * @param grob
   * @return The layertoggle of this layer
   */
  public JToggleButton addLayerObjects (Vector grob) {
    Category acat;
    int catSelMode = CurrentState.getCatSelectionMode();

    switch(catSelMode){
       case CurrentState.CATEGORY_AUTO: {
                acat = createAutoCat();
                ListIterator li = grob.listIterator();
                while (li.hasNext()) {
                   DsplGraph dg = ((DsplGraph)li.next());
                   if(dg==null){
                        Reporter.writeError("viewer.hoese.GraphWindow .addLayerObjects"+
                                            " has received a null object");
                   } else {
                      dg.setCategory(acat);
                   }
                }
                break;
            }
      case CurrentState.CATEGORY_MANUAL: {
               newQueryRepresentation(grob);          
               break;
           }
      case CurrentState.CATEGORY_BY_NAME: {
              int size = grob.size();
              Vector remaining = new Vector(size); // objects which have no category with
                                                   // the same name
              for(int i=0;i<size;i++){
                 DsplGraph dg = (DsplGraph)grob.get(i);
                 if(dg==null){
                    Reporter.writeError("GraphWindow.addLayerObjects received a NULL object");
                 } else{
                    String attrName = dg.getAttrName();
                    Category cat = getCategory(attrName);
                    if(cat==null){
                       remaining.add(dg);
                    } else {
                        dg.setCategory(cat);
                    }
                 } 
              }
              // process remaining objects
              if(remaining.size()>0){
               newQueryRepresentation(remaining);
              }
              break;
           }
      default : {
                  Reporter.writeError("GraphWindow.addLayerObjects: unknown category selection mode");
                  newQueryRepresentation(grob);
                }
    }
    Layer lay = new Layer(grob, this);
    int Laynr = ++highest;
    add(lay, new Integer(Laynr));
    mw.updateViewParameter();
    return  lay.CreateLayerButton(LayerButtonListener, Laynr);
  }

  /** Computes the category with the given name.
    * If not such category is found, NULL is returned.
    */
  Category getCategory(String name){
     Vector cats = mw.Cats;
     int size = cats.size();
     for(int i=0;i<size;i++){
        Category Cat = (Category)cats.get(i);
        if(Cat.hasName(name))
            return Cat;
     }
     // nothing found
     return null;
  }  


  /**
   * Adds the Layer l to the top of layerstack
   * @param l
   * @return The layertoggle of this layer
   * @see <a href="Categorysrc.html#addLayer">Source</a> 
   */
  public JToggleButton addLayer (Layer l) {
    //    add (lay,new Integer(highestLayer()+1));
    Layer lay = new Layer(l.getGeoObjects(), this);
    int Laynr = highest++;
    add(lay, new Integer(Laynr));
    mw.updateViewParameter();
    return  lay.CreateLayerButton(LayerButtonListener, Laynr);
  }

  /**
   * This method makes it possible to add categories to each graph. attribute of a query
   * @param go A Vector with graphical objects of a query
   */
  public void newQueryRepresentation (Vector go) {
    //acat=Category.getDefaultCat();
    try{
       ListIterator li = go.listIterator();
       Vector v = new Vector(5, 1);
       while (li.hasNext()) {
         String aname = ((DsplGraph)li.next()).getAttrName();
         if (!v.contains(aname)) {
           v.add(aname);
           JComboBox cb = mw.TextDisplay.getQueryCombo();
           QueryResult Query = (QueryResult)cb.getSelectedItem();
           ViewConfig VCfg = Query.getViewConfigAt(Query.getViewConfigIndex(aname));
           if (VCfg==null){
               VCfg = new ViewConfig(mw,aname);
               Query.addViewConfig(VCfg);
           }
           VCfg.readPool();
           VCfg.setVisible(true);
         }
      }
    }
    catch(Exception e){
      Reporter.writeError("Exception in : GraphWindow.newQueryRep "+e );
      Reporter.debug(e);
    }
  }

  /**
   * Gets the internal no. of the highest layer
   * @return No. as int
   */
  public int getTop () {
    return  highest;
  }

  /**
   * Sets a new size for all layers
   * @param asize
   * @see <a href="Categorysrc.html#updateLayersSize">Source</a> 
   */
  public void updateLayersSize (Rectangle asize) {
    setPreferredSize(new Dimension((int)(asize.getX() + asize.getWidth()), 
        (int)(asize.getY() + asize.getHeight())));              //updateLayerSize();
    for (int i = 0; i < getComponentCount(); i++)
      //if (getLayer(getComponent(i))<=10000)
      getComponent(i).setBounds(asize);
    revalidate();
  }

  /**
   * Recalculates the boundingbox of all graph. objects
   * @see <a href="Categorysrc.html#updateBoundingBox">Source</a> 
   */
  public void updateBoundingBox () {
    Rectangle2D.Double r;
    BackGroundImage bgi = mw.getBackgroundImage();
    if(bgi.useForBoundingBox()){
         r = (Rectangle2D.Double) bgi.getBBox().clone();
    }else{
         r = null;
    }
    for (int i = 0; i < getComponentCount(); i++)
      if (getComponent(i) instanceof Layer) {
        Layer l = (Layer)getComponent(i);
        Rectangle2D.Double layer_box = l.getWorldCoordBounds();
        if (r == null)
          r = layer_box; 
        else{ 
          if(layer_box!=null)
              r = (Rectangle2D.Double)r.createUnion(layer_box);
        }
      }
    if (r != null)
      mw.BBoxWC = r;
  }

  public void updateBackground(){
    BackGroundImage bgi = mw.getBackgroundImage();
    if(bgi==null){
      remove(background);
      repaint();
      if(gui.Environment.DEBUG_MODE){
          Reporter.writeError("cannot find the background ");
      }
      return;
    }
    if(bgi.useForBoundingBox()) {
        ignorePaint=true;
        updateBoundingBox();
        ignorePaint=false;
    }
    mw.updateViewParameter();

    BufferedImage bgimg = bgi.getImage();
    if(bgimg==null){
       background.setImage(null);
       repaint();
       return;
    }
    // ok a background is given
    BufferedImage img2 = background.getImage();
    if(bgimg==img2){ // no changes of the picture
      return;
    }

    
   //if(img2==null){
   //    add(background,new Integer(-1));
   // }
    background.setImage(bgimg);
    repaint();
  }

  /**
   * draws all layers
   * @param g
   * @see <a href="Categorysrc.html#paintChildren">Source</a> 
   */
  public void paintChildren (Graphics g) {
    if(ignorePaint) return;

    // paint the background image
    // first transform the boundig box for the background 
    // to into screen coordinates
    AffineTransform at = CurrentState.transform;
    Rectangle2D R = at.createTransformedShape(mw.getBackgroundImage().getBBox()).getBounds();
    background.setBounds((int)R.getX(),(int)R.getY(),(int)R.getWidth(),(int)R.getHeight());
    Rectangle2D R2 = mw.GeoScrollPane.getBounds();
    Rectangle2D R3 = getBounds(); 
    R2.setRect(-R3.getX(),-R3.getY(),R2.getWidth(),R2.getHeight());
    background.setClipRect(R2);
    Graphics2D g2 = (Graphics2D)g;
    g2.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL, RenderingHints.VALUE_STROKE_PURE);
    super.paintChildren(g2);

    // paint  the selected object again to be visible even if it is behind othe objects

    DsplGraph dg = mw.getSelGO();
    if ((dg != null) && (dg.getVisible())){
	   		      Composite C = g2.getComposite();
		  			  g2.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.0f));
			  	    Layer.draw(dg,g2,CurrentState.ActualTime,at);
	            g2.setComposite(C);
     }
     // draw addinitional objects from objectvcreation
     if(additionalGraphObject!=null){
         Layer.draw(additionalGraphObject,g2,CurrentState.ActualTime,at); 
     }
}

  public void addMouseListener(MouseListener ML){
     MouseListener[] MLs = getMouseListeners();
     boolean found = false;
     for(int i=0;i<MLs.length && ! found ; i++)
         found = ML.equals(MLs[i]);
     if(!found)
       super.addMouseListener(ML);
  }



}





