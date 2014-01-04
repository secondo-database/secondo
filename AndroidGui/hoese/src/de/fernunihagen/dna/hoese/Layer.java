package de.fernunihagen.dna.hoese;

import javamini.awt.geom.Rectangle2D;

import javamini.awt.Dimension;
import javamini.awt.Shape;
import javamini.awt.geom.Point2D;
import java.util.ListIterator;
import java.util.Vector;

import de.fernunihagen.dna.hoese.project.VoidProjection;


import tools.Reporter;
import javamini.awt.geom.AffineTransform;

public class Layer {
	  /** All the graph. objects that are accociated with this Layer */
	  private Vector GeoObjects;
	  /** the world boundingbox of this  layer */
	  Rectangle2D.Double boundsWC;
	  /** True if the selected object is in this layer */
	  boolean Selected = false;
	  /** the internal no. of this layer */
	  int LayerNo;


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
	  public Layer (Vector obj, Object gw) {
	    GeoObjects = (Vector)obj.clone();
//	    setDoubleBuffered(true);
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
// TODO:
		  return null; // owner.getProjection();
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
}
