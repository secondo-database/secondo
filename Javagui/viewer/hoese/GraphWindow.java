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

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Component;
import java.awt.Composite;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseAdapter;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import javax.swing.JPopupMenu;
import javax.swing.JOptionPane;
import javax.swing.JMenuItem;
import java.util.ListIterator;
import java.util.Vector;
import java.awt.geom.*;
import java.awt.BasicStroke;

import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JLayeredPane;
import javax.swing.JToggleButton;

import tools.Reporter;
import viewer.HoeseViewer;

import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;

import java.util.TreeSet;
import java.util.Iterator;


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

  /** The background object - start with a SimpleBackground */
  Background background = new SimpleBackground();

/** a flag for ignoring a repaint message.
    This can be used to make all changes without new drawing.
**/
   private boolean ignorePaint = false;


/** a additional object for  painting **/
   DsplGraph  additionalGraphObject;

/** a listener playing sound if the mouse goes over an object */
   SoundMouseListener soundListener;
/** flag determining whether sound should be played for
  * all objects under the cursor or only for the first object found.
  **/   
   boolean multisound = false;


   PreloadDialog  preloadDialog;


  /*
     The selected rectangle.
  */
  boolean selection = false;
  int sel_x = 0;
  int sel_y = 0;
  int sel_w = 0;
  int sel_h = 0;
  BasicStroke sel_stroke = new BasicStroke(2.0f);
  

  public synchronized void setSelection(int x, int y, int w, int h){
     if(selection){
        if(   sel_x == x && sel_y == y 
           && sel_w == w && sel_h == h){
         // selection already painted at this position,
         // ignore
         return;
        } 
     }
     if(selection){
        // remove old rectangle
        paintSelection();
     }
     sel_x = x;
     sel_y = y;
     sel_w = w;
     sel_h = h;
     // paint new rectangle
     paintSelection(); 
     selection = true;
  }

  /** paints or removes selection **/
  private synchronized void paintSelection(){
     Graphics2D g = (Graphics2D) getGraphics();
     g.setXORMode(Color.WHITE);    
     g.setStroke(sel_stroke);
     g.fillRect(sel_x, sel_y, sel_w, sel_h);
  }



  public void removeSelection(){
     if(selection) {
        paintSelection();
        selection = false;
     }
  }






  private class SoundMouseListener implements MouseMotionListener{
     public SoundMouseListener(){
       lastCats = new TreeSet<Category>();
     } 

     public void mouseDragged(MouseEvent evt){

     }
     public void mouseMoved(MouseEvent e){
         int x = e.getX();
         int y = e.getY();
         // collect the categories of objects under cursor
         GraphWindow gw = GraphWindow.this;
         boolean done = false;
         Point2D.Double p = new Point2D.Double();
         try {
           p = (Point2D.Double)CurrentState.transform.inverseTransform(e.getPoint(), p);
         } catch (Exception ex) {
            return; 
         }
         TreeSet<Category> currentCats = new TreeSet<Category>();
         double scalex = 1/Math.abs(CurrentState.transform.getScaleX());
         double scaley = 1/Math.abs(CurrentState.transform.getScaleY());

         for(int i=0;i<gw.getComponentCount()&& !done; i++){
            Layer layer = (Layer) gw.getComponent(i);
            TreeSet layerCats = layer.getCats(p,scalex,scaley, !gw.multisound);
            currentCats.addAll(layerCats);
            if(!gw.multisound && currentCats.size()>0){
               done = true;
            } 
         }
         Iterator<Category> it = lastCats.iterator();
         while(it.hasNext()){
            Category cat = it.next();
            if(!currentCats.contains(cat)){
               cat.stopSound();
            }
         }
         it = currentCats.iterator();
         while(it.hasNext()){
            Category cat = it.next();
            cat.startSound();
         }
         lastCats = currentCats;
     }    
     private TreeSet<Category> lastCats;
  }


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
    soundListener = new SoundMouseListener();

  }

  public void enableSound(boolean on){
     if(on){
       addMouseMotionListener(soundListener);
     } else {
       removeMouseMotionListener(soundListener);
     }
  }


  public void startPreload(){

     preloadDialog = new PreloadDialog(mw.getViewerControl().getMainFrame());
     if(!background.supportsPreload()){
        return;
     }

     // compute clipRect
     Rectangle2D.Double clipRect = getClipRect();
     int fileNumber = background.numberOfPreloadFiles(clipRect);


     if(fileNumber<1){
       return;
     }
     if(JOptionPane.showConfirmDialog(mw.getViewerControl().getMainFrame(), "This will download " + fileNumber + " files \n continue?")!=JOptionPane.YES_OPTION){
          return;
     }
     preloadDialog.setTarget(fileNumber);
     CancelListener cl = new CancelListener(){
         public void cancel(){
           background.cancelPreload();
           preloadDialog.finish(false);
         }
     };
     preloadDialog.setCancelListener(cl);
     background.preload(clipRect,preloadDialog);
     preloadDialog.setVisible(true);
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
  public JToggleButton addLayerObjects (QueryResult qr, boolean changeZoom) {

	Vector grob = qr.getGraphObjects();
	  
    handleCategory(grob);
    
    Layer lay = new Layer(grob, this);
    int Laynr = ++highest;
    qr.setResultLayer(lay);
    add(lay, Integer.valueOf(Laynr));
    mw.updateViewParameter(changeZoom);
    JToggleButton res =  lay.CreateLayerButton(LayerButtonListener, Laynr);
    return res; 
  }

  /**
   * Sets the Category by the provided selection mode
   * @param grob
   */
	private void handleCategory(Vector grob) {
		Category acat;
		switch(CurrentState.getCatSelectionMode()){
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
  public JToggleButton addLayer (Layer l, boolean changeZoom) {
    //    add (lay,new Integer(highestLayer()+1));
    Layer lay = new Layer(l.getGeoObjects(), this);
    int Laynr = highest++;
    add(lay, Integer.valueOf(Laynr));
    mw.updateViewParameter(changeZoom);
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


  /* scales an rectangle by scaleFactor from ProjectionManager */
  /*Rectangle2D.Double scale(Rectangle2D.Double r, boolean reverse){
    double sf = ProjectionManager.getScaleFactor();
    if(!reverse){
       sf = 1/sf;
    }
    r.setRect( r.x*sf, r.y*sf, r.width*sf, r.height*sf);
    return r;
  }
  */

  /**
   * Recalculates the boundingbox of all graph. objects
   * @see <a href="Categorysrc.html#updateBoundingBox">Source</a>
   */
  public void updateBoundingBox () {
    Rectangle2D.Double r;
    Background bgi = getBackgroundObject();
    if(bgi.useForBoundingBox()){
         r = (Rectangle2D.Double) bgi.getBBox().clone();

         Point2D.Double aPoint = new Point2D.Double();
         double x1 = r.getX();
         double y1 = r.getY();
         if(ProjectionManager.project(x1,y1,aPoint)){
             double x2 = x1 + r.getWidth();
             double y2 = y1 + r.getHeight();
             x1 = aPoint.getX();
             y1 = aPoint.getY();
             if(ProjectionManager.project(x2,y2,aPoint)){
                 r.setRect(x1,y1,aPoint.getX()-x1, aPoint.getY()-y1);
             }
         }
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
    if (r != null){
      mw.BBoxWC = r;
    } else {
      mw.BBoxWC=new Rectangle2D.Double(-170,-80,340,160);
    }
  }

  public void updateBackground(){
    Background bgi = getBackgroundObject();
    if(bgi==null){
      repaint();
      if(tools.Environment.DEBUG_MODE){
          Reporter.writeError("cannot find the background ");
      }
      return;
    }
    if(bgi.useForBoundingBox()) {
        ignorePaint=true;
        updateBoundingBox();
        ignorePaint=false;
    }
    mw.updateViewParameter(true);

    repaint();
  }

  /**
   * draws all layers
   * @param g
   * @see <a href="Categorysrc.html#paintChildren">Source</a>
   */

  /** computes the currently displayed rectangle in world coordinates **/

  private Rectangle2D.Double getClipRect(){
      AffineTransform at = CurrentState.transform;
      try{
         AffineTransform at1 = new AffineTransform(at);
         AffineTransform iat = at1.createInverse();
         Rectangle vpbounds1 =  mw.GeoScrollPane.getViewport().getViewRect();
         Rectangle2D.Double vpbounds = new Rectangle2D.Double(vpbounds1.x, vpbounds1.y,vpbounds1.width, vpbounds1.height);
         Rectangle2D cbbi1 = iat.createTransformedShape(vpbounds.getBounds2D()).getBounds2D();
         Rectangle2D.Double cbbi;
         if(cbbi1 instanceof Rectangle2D.Double){
            cbbi = (Rectangle2D.Double)cbbi1;
         } else {
            cbbi=new Rectangle2D.Double( cbbi1.getMinX(), cbbi1.getMinY(), cbbi1.getWidth(), cbbi1.getHeight());
         }
         return cbbi;
      } catch(Exception e){
         return null;
      }
  }


  public void paintChildren (Graphics g) {
  if (ignorePaint) {
      return;
  }

  AffineTransform at = CurrentState.transform;
  Graphics2D g2 = (Graphics2D) g;

  // paint the background
  // first transform the bounding box for the background
  // into screen coordinates
  try{

    Rectangle2D.Double clipRect = getClipRect();
    double antiScale = background.getAntiScale(CurrentState.transform,clipRect);
    if(Auxiliary.almostEqual(antiScale,1.0)){
       antiScale = 1.0;
    }


    if(antiScale!=1){
        mw.zoom(antiScale);
    }



    if(background != null) {
      background.paint(this,g2,CurrentState.transform, clipRect);
    }
  } catch(Exception e){
     Reporter.writeError("Problem while drawing background");
     Reporter.debug(e);
  }


  g2.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL,
        RenderingHints.VALUE_STROKE_PURE);
  super.paintChildren(g2);

    // paint the selected object again to be visible even if it is behind
    // other objects

      DsplGraph dg = mw.getSelGO();
    if ((dg != null) && (dg.getVisible())) {
      Composite C = g2.getComposite();
      g2.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,
          0.0f));
      Layer.draw(dg, g2, CurrentState.ActualTime, at);
      g2.setComposite(C);
    }
    // draw addinitional objects from objectvcreation
    if (additionalGraphObject != null) {
      Layer.draw(additionalGraphObject, g2, CurrentState.ActualTime, at);
    }
    if(selection){
      // an old selection is overwritten by repainting the scene
      // draw the selection 
      paintSelection();
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


  /**
   * returns the actual background object
   */
  public Background getBackgroundObject() {
    return background;
  }

  /**
   * set the background object
   */
  public void setBackgroundObject(Background b) {
    if(b != null){
      background = b;
    } else if (background == null){
      background = new SimpleBackground(Color.PINK);
    }
  }

}
