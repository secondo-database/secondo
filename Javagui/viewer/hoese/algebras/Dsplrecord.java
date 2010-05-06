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

package  viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import java.awt.geom.*;
import gui.SecondoObject;
import tools.Reporter;


/**
 * A displayclass for the record-type.
 * The displayclass for records creates one single query result.
 * The showing of the single record elements is delegated to the corresponding
 * display classes of the element types.<p>
 *
 * <p>Implements <p>
 *            ExternDisplay:   interface to enable displaying the record single elements
 *                             in a separate frame<br>
 *            Timed:           interface for displaying temporal elements<br>
 *            DsplGraph:       interface for displaying graphical elements<br>
 *            RenderAttribute: interface for setting the render attribute<br>
 *            LabelAttribute:  interface for setting the label attribute<br>
 *            DisplayComplex:  interface for displaying complex elements<br>
 *
 * @author Sabrina Straub
 * @version 1.0 for FaPra WS 2009/2010
 */
public class Dsplrecord extends DsplGeneric implements ExternDisplay, Timed, 
				DsplGraph, RenderAttribute, LabelAttribute, 
                                DisplayComplex {

  /** Frame to show record elements */
  private RecordFrame display=null;

  /** The Value ListExpr of the given secondo Object */
  private ListExpr recVal;

  /** The Types ListExpr of the given secondo Object */
  private ListExpr recTypes;

  /** A structur to store the instances of the record elements display classes */
  private Vector graphVector = new Vector();

  /** A structure to store the amount of shapes per object */
  private Vector shapeVector = new Vector();

  /** The category of this object. 
   * init with value defaultcategory */
  private Category cat = Category.getDefaultCat();

  /** The layer in which this object is drawn */
  private Layer refLayer;

  /** The subtype of the record */
  private String subtype;

  /** Dsplrecord Constructor.
   *  The constructor initializes a new frame for external display function
   *  when a new record object needs to be displayed.
   */
  public Dsplrecord() {
    if(this.display==null){
      this.display = new RecordFrame();
    }
  }

  /*
  * Implemented methods to represent the query result
  */

  /** Dsplrecord init.
   * This method is used to analyse the types and values of this record type.
   *
   * @param type datatype record with its attribute-types 
   * @param value A listexpr of the attribute-values
   * @param qr The queryresultlist to add alphanumeric representation
   */
  public void init(String name, int nameWidth, int indent, ListExpr type, 
                   ListExpr value, QueryResult qr) {
    // create a new entry in query result
    if(type.isAtom()){
      this.subtype=getTypeName(type).symbolValue();
    }
    else{
      this.subtype=getTypeName(type).toString();
    }

    // add entry
    qr.addEntry(this);
    this.recVal = value;
    this.recTypes = type;
    // initialize the record elements
    initRecordElements(value, type);

  }


  /** Dsplrecord isGraph.
   * A method to determine wether a record elements type is an instance of DsplGraph 
   * @param typename
   * @return boolean result of the determination
   * @throws ClassNotFoundException
   * @throws InstantiationException
   * @throws IllegalAccessException
   */
  public boolean isGraph(String typename){
    try{
      Class c=Class.forName("viewer.hoese.algebras."+"Dspl" + typename);
      Object o = c.newInstance();
      if(o instanceof DsplGraph){
        return true;
      }
      else{
        return false;
      }
    }catch(ClassNotFoundException e){
      Reporter.showError("No Displayclass found for record element type: " + typename);
    }catch(InstantiationException e){
      Reporter.showError("Displayclass for record element type " + typename + " could not be initialized!");
    }catch(IllegalAccessException e){
      Reporter.showError("Error while accessing DisplayClass for record element type :" + typename);
    }
    return false;
  }

  /** Dsplrecord toString. 
   * A method to return the textual represantation of a record
   * as query result entry
   * @return the textual Represantation as a string
   */
  public String toString() {
    return this.subtype;
  }

  /** Dsplrecord getAttrName. 
   * A method of the DsplBase-Interface.
   * @return The name of the Attribute
   */
  public String getAttrName() {
    return this.subtype;
  }

  /** Dsplrecord getTypeName. 
   * A method to return the typename of the given type
   * @param type
   * @return the subtype as an ListExpr object
   */
  public ListExpr getTypeName(ListExpr type){
    ListExpr typename=null;
    typename = type.first();
    return typename;
  }

  /** Dsplrecord initRecordElements.
   * A method that iterates over all record elements and initializes them.
   * All graphical elements are stored in the graphVector, to ensure that all
   * elements can be displayed.
   * @param value
   * @param type
   * @throws ClassNotFoundException
   * @throws InstantiationException
   * @throws IllegalAccessException
   */
  private void initRecordElements(ListExpr value, ListExpr type){
    ListExpr val;
    ListExpr rest;
    String typename = "";
    try{
        if(!type.isEmpty()){
          rest = type.rest();
          val = value;
          if(rest.first().second().isAtom() == true){
            // the first element is atom
            if(isGraph(rest.first().second().symbolValue())){
              // element is graph
              typename = rest.first().second().symbolValue(); 
              Class to = Class.forName("viewer.hoese.algebras."+"Dspl" + typename);
              DsplGraph elementClassO = (DsplGraph)to.newInstance();
              // add element to graphVector
              graphVector.addElement(elementClassO);
              // initialize element
              if(val.first() == null){
                elementClassO.init(typename,0,0,rest.first().second(),val,new QueryResult(new SecondoObject("recordElem",val),true));
              }
              else{
              elementClassO.init(typename,0,0,rest.first().second(),val.first(),new QueryResult(new SecondoObject("recordElem",val.first()),true));
              }
            }
          }
          if(rest.first().second().isAtom() == false){
            // the first element is not atom
            if(isGraph(rest.first().second().first().symbolValue())){
              // element is graph
              typename = rest.first().second().first().symbolValue();
              Class to = Class.forName("viewer.hoese.algebras."+"Dspl" + typename);
              DsplGraph elementClassO = (DsplGraph)to.newInstance();
              // add element to graphVector
              graphVector.addElement(elementClassO);
              // initialize element - use complete type (including subelements)
              elementClassO.init(rest.first().second().toString(),0,0,rest.first().second(),val.first(),new QueryResult(new SecondoObject("recordElem",val.first()),true));
            }
          }
          while(!rest.rest().isEmpty()){
            // iterate the other elements of the record
            rest = rest.rest();
            if(val!=null){
              val = val.rest();
            }
            if(rest.first().second().isAtom() == true){
              // element is atom
              if(isGraph(rest.first().second().symbolValue())){
                typename = rest.first().second().symbolValue();
                // element is graph
                Class to = Class.forName("viewer.hoese.algebras."+"Dspl" + typename);
                DsplGraph elementClassO = (DsplGraph)to.newInstance();
                // add element to graphVector
                graphVector.addElement(elementClassO);
                // initialize element
                if(val == null){
                  elementClassO.init(typename,0,0,rest.first().second(), new ListExpr(),new QueryResult(new SecondoObject("recordElem",val),true));
                }
                else{
                  elementClassO.init(typename,0,0,rest.first().second(),val.first(),new QueryResult(new SecondoObject("recordElem",val.first()),true));
                }
              }
            }
            if(rest.first().second().isAtom() == false){
              // element is not atom
              if(isGraph(rest.first().second().first().symbolValue())){
                // element is graph
                typename = rest.first().second().first().symbolValue();
                Class to = Class.forName("viewer.hoese.algebras."+"Dspl" + typename);
                DsplGraph elementClassO = (DsplGraph)to.newInstance();
                // add element to graphVector
                graphVector.addElement(elementClassO);
                // initialize element - use complete type (including subelements)
                elementClassO.init(rest.first().second().toString(),0,0,rest.first().second(),val.first(),new QueryResult(new SecondoObject("recordElem",val.first()),true));

              }
            }
          }
        }
      }
      catch(ClassNotFoundException e){
        Reporter.showError("No Displayclass found for record element type: " + typename);
      }catch(InstantiationException e){
        Reporter.showError("Displayclass for record element type " + typename + " could not be initialized!");
      }catch(IllegalAccessException e){
        Reporter.showError("Error while accessing DisplayClass for record element type: " + typename);
      }
  }

  /** Dsplrecord findShape.
   * A method to find a specific shape in the shapeVector
   * @return the position in the shapeVector
   * @param num
   */
  public int findShape(int num){
    for(int i=0;i<this.shapeVector.size();i++){
      if(((ShapeSet)this.shapeVector.get(i)).hasNumber(num))return i;
    }
    return 0;
  }

  /*
  * Implemented methods from ExternDisplay
  */

  /** Origin ExternDisplay interface.
   * Returns true, if the extra frame is displayed
   * @return the boolean result
   */
  public boolean isExternDisplayed() {
    return this.display.isVisible();
  }

  /** Origin ExternDisplay interface.
   *  Displays big textual information in a extra frame
   */
  public void displayExtern() {
    this.display.setSource(this);
    this.display.setVisible(true);
  }

  /*
  * Implemented methods from Timed
  */

  /** Origin Timed interface.
   * Gets the over all time boundaries
   * @return Interval
   */
  public Interval getBoundingInterval() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      Interval intV=null;
        for(int j=0;j<this.graphVector.size();j++){
          if(this.graphVector.get(j) instanceof Timed){
            if(intV==null){
	      intV=((Timed)this.graphVector.get(j)).getBoundingInterval();
	    }else{
	      intV = intV.union(((Timed)this.graphVector.get(j)).getBoundingInterval());
	    }
          }
        }
        return intV;
    }
    return null;
  }
  
  /** Origin Timed interface.
   * Gets the list of intervals this object is defined at
   * @return Vector of intervals
   */
  public Vector getIntervals() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      Vector intervalVector=new Vector();
        for(int j=0;j<this.graphVector.size();j++){
          if(this.graphVector.get(j) instanceof Timed){
            Vector v=((Timed)this.graphVector.get(j)).getIntervals();
	    for(int i=0;i<v.size();i++){
	      intervalVector.add(v.get(i));
	    } 
          }
        }
      return intervalVector;
    }
    return null;
  }

  /** Origin Timed interface.
   * In the TimePanel component a temporal datatype can be represented 
   * individually.
   * This method defines a specific output as JPanel
   * @param PixelTime How much timeunits a pixel has
   * @return JPanel
   */
  public JPanel getTimeRenderer(double pixelTime) {
    JPanel pan = new JPanel();
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof Timed){
          return ((Timed)this.graphVector.get(j)).getTimeRenderer(pixelTime);
        }
      }
    }
    return pan;
  }

  /*
  * Implemented methods from DsplGraph
  */

  /** Origin DsplGraph interface.
   * Returns the number of contained shapes
   * @return int
   */
  public int numberOfShapes() {
    int sum=0;
    int temp;
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int i=0;i<this.graphVector.size();i++){
        if(this.graphVector.get(i) instanceof DsplGraph){
          temp=((DsplGraph)this.graphVector.get(i)).numberOfShapes();
          this.shapeVector.addElement(new ShapeSet(sum,temp));
          sum+=temp;
        }
      }
    }
    return sum;
  }

  /** Origin DsplGraph interface.
   * Determines whether the type is displayed as point.
   * @param num
   * @return true if is pointtype
   */
  public boolean isPointType(int num) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int i=0;i<this.graphVector.size();i++){
        if(this.graphVector.get(i) instanceof DsplGraph){
          return (((DsplGraph)this.graphVector.get(i)).isPointType(num));
        }
      }
    }
    return false;
  }

  /** Origin DsplGraph interface.
   * Determines whether the type is a line (no interior)
   * @param num
   * @return true if is linetype
   */
  public boolean isLineType(int num) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int i=0;i<this.graphVector.size();i++){
        if(this.graphVector.get(i) instanceof DsplGraph){
          return (((DsplGraph)this.graphVector.get(i)).isLineType(num));
        }
      }
    }
    return false;
  }

  /**  Origin DsplGraph interface.
   * Text of the associated Label
   * @param time.
   * @return Labeltext
   */
  public String getLabelText(double time) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int i=0;i<this.graphVector.size();i++){
        if(this.graphVector.get(i) instanceof DsplGraph){
          return (((DsplGraph)this.graphVector.get(i)).getLabelText(time));
        }
      }
    }
    return null;
  }

  /** Origin DsplGraph interface.
   * Returns the attribute controlling the creation of the label.
   * @return LabelAttribute
   */
  public LabelAttribute getLabelAttribute() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int i=0;i<this.graphVector.size();i++){
        if(this.graphVector.get(i) instanceof DsplGraph){
          return (((DsplGraph)this.graphVector.get(i)).getLabelAttribute());
        }
      }
    }
    return null;
  }

  /** Origin DsplGraph interface.
   * Sets the labeltext for an object
   * @param label Text of label
   */
  public void setLabelAttribute(LabelAttribute label) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          ((DsplGraph)this.graphVector.get(j)).setLabelAttribute(label);
        }
      }
    }
  }

  /** Origin DsplGraph interface.
   * Gets the offset of the labelposition from center of object in pixel.
   * @return relative offset as point
   */
  public Point getLabPosOffset() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          return ((DsplGraph)this.graphVector.get(j)).getLabPosOffset();
        }
      }
    }
    return new Point(0,0);
  }

  /** Origin DsplGraph interface.
   * Sets the offset of the labelposition from center of object in pixel.
   * @param pt relative offset
   */
  public void setLabPosOffset(Point pt) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          ((DsplGraph)this.graphVector.get(j)).setLabPosOffset(pt);
        }
      }
    }
  }

  /** Origin DsplGraph interface.
   * The boundingbox of the record elements in Worldcoordinates
   * @return Boundingbox in double precision
   */
  public Rectangle2D.Double getBounds() {
    Rectangle2D.Double r = null;
    if(this.graphVector!=null && this.graphVector.size()>0){
      // there are some graphical objects to display
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof Timed){
          if(r==null){
	    r=((DsplGraph)this.graphVector.get(j)).getBounds();
          }
          else{
	    r=(Rectangle2D.Double)r.createUnion(((DsplGraph)this.graphVector.get(j)).getBounds());
          }
        }
      }
    }
    int num = numberOfShapes();
    for(int i=0;i<num;i++){
      Shape shp = getRenderObject(i,new AffineTransform());
      if(shp!=null){
        Rectangle2D b = shp.getBounds2D();
        if(r==null){
          r = new Rectangle2D.Double(b.getX(),b.getY(),b.getWidth(),b.getHeight());
        } 
        else {
          r.add(b);
        }
      }
    }
    return r;
  }

  /** Origin DsplGraph interface.
   * Returns one of the current Shapes of this object. 
   * @param num: the number of the requested shape
   * @param at
   * @return Shape
   */
  public Shape getRenderObject(int num, AffineTransform at) {
    Shape shape=null;
    int objectNo=findShape(num);
    ShapeSet shS=(ShapeSet)this.shapeVector.get(objectNo);
    shape=((DsplGraph)this.graphVector.get(objectNo)).getRenderObject(shS.getPosition(num),at);
    return shape;
  }

  /** Origin DsplGraph interface.
   * Sets the category for drawing the record elements.
   * @param acat The category to set
   */
  public void setCategory(Category acat) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          ((DsplGraph)graphVector.get(j)).setCategory(acat);
        }
      }
    }
    this.cat = acat;
  }

  /** Origin DsplGraph interface.
   * Gets the category for drawing the record elements.
   * @return The category.
   */
  public Category getCategory() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          return (((DsplGraph)this.graphVector.get(j)).getCategory());
        }
      }
    }
    return  this.cat;
  }

  /** Origin DsplGraph interface.
   * Sets the renderattribute of this object 
   * @param renderAttribute
   */
  public void setRenderAttribute(RenderAttribute renderAttribute) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          ((DsplGraph)graphVector.get(j)).setRenderAttribute(renderAttribute);
        }
      }
    }
  }

  /** Origin DsplGraph interface.
   * Returns the render attribute assigned to this object 
   * @return RenderAttribute
   */
  public RenderAttribute getRenderAttribute(){
    if(this.graphVector!=null && this.graphVector.size()>0){
     for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          return (((DsplGraph)this.graphVector.get(j)).getRenderAttribute());
        }
      }
    }
    return null;
  }

  /** Origin DsplGraph interface.
   * Specifies the layer to which this object belongs.
   * @param alayer A Layer-object
   */
  public void setLayer(Layer alayer) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          ((DsplGraph)this.graphVector.get(j)).setLayer(alayer);
        }
      }
    }
    this.refLayer = alayer;
  }

  /** Origin DsplGraph interface.
   * Gets the layer to which this object belongs.
   * @return object layer
   */
  public Layer getLayer() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          return (((DsplGraph)this.graphVector.get(j)).getLayer());
        }
      }
    }
    return this.refLayer;
  }

  /** Origin DsplGraph interface.
   * Tests if a world position is inside this object, or near by under a 
   * certain scale, which is necessary to translate pixel distances 
   * to world-distance e.g. line
   * @param xpos x -coordinate of the position
   * @param ypos y -coordinate of the position
   * @param scalex x-scale
   * @param scaley y-scale
   * @return boolean
   */
  public boolean contains(double xpos, double ypos,double scalex, double scaley) {
    boolean result=false;
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DsplGraph){
          result=result || (((DsplGraph)graphVector.get(j)).contains(xpos,ypos,scalex,scaley));
        }
      }
    }
    return result;
  }

  /*
  * Implemented methods from RenderAttribute
  */
  
  /** Origin RenderAttribute interface.
   * Returns the defined state at the given time 
   * @param time
   * @return boolean
   */
  public boolean isDefined(double time) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof RenderAttribute){
          return (((RenderAttribute)this.graphVector.get(j)).isDefined(time));
        }
      }
    }
    return false;
  }
   
  /** Origin RenderAttribute interface.
   * Returns the minimum value of this attribute
   * @param time
   * @return double
   */
  public double getRenderValue(double time) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof RenderAttribute){
          return (((RenderAttribute)this.graphVector.get(j)).getRenderValue(time));
        }
      }
    }
    return 1.0;
  }

  /** Origin RenderAttribute interface.
   * Return whether this objects is defined at any time
   * @return boolean
   */
  public boolean mayBeDefined() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof RenderAttribute){
          return  (((RenderAttribute)this.graphVector.get(j)).mayBeDefined());
        }
      }
    }
    return false;
  }
   
  /** Origin RenderAttribute interface.
   * Returns the maximum value of this attribute
   * @return double
   */
  public double getMinRenderValue() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof RenderAttribute){
          return (((RenderAttribute)this.graphVector.get(j)).getMinRenderValue());
        }
      }
    }
    return 1.0;
  }
   
  /** Origin RenderAttribute interface.
   * Returns the value of this attribute for the given time
   * @return double
   */
  public double getMaxRenderValue() {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof RenderAttribute){
          return (((RenderAttribute)this.graphVector.get(j)).getMaxRenderValue());
        }
      }
    }
    return 1.0;
  }

  /*
  * Implemented methods from LabelAttribute
  */

  /** Origin LabelAttribute interface.
   * @param time
   * @return The label of the Attribute
   */
  public String getLabel(double time) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof LabelAttribute){
            return (((LabelAttribute)graphVector.get(j)).getLabel(time));
        }
      }
    }
    return null;
  }

  /*
  * Implemented methods from DisplayComplex
  */

  /** Origin DisplayComplex interface.
   * Draw this object to g.
   * The time parameter can be used to create a special representation of 
   * a time dependent object or for asking the renderattribute for some
   * values. The currently used transformation matrix can be used for example
   * if an object should have the same size in each zoom level. 
   * @param g:    the used graphics context
   * @param time: the current time in the animation. 
   * @param at:   the currently used transformation matrix.
   */
  public void draw(Graphics g, double time, AffineTransform at) {
    if(this.graphVector!=null && this.graphVector.size()>0){
      for(int j=0;j<this.graphVector.size();j++){
        if(this.graphVector.get(j) instanceof DisplayComplex){
          ((DisplayComplex)graphVector.get(j)).draw(g,time,at);
        }
      }
    }
  }

  /** Embedded class RecordFrame.
   * Extern displayed frame. Used to display the single record elements.
   * Accessible by double click.
   */
  protected class RecordFrame extends JFrame{

    /** a reference to the Dsplrecord Object */
    private Dsplrecord record;

    /** a QueryResult to display the record elements */
    private QueryResult qrNEW;

    /** a ScrollPane to store the QueryResult */
    private JScrollPane textDspl = new JScrollPane();

    /** 
     * Creates a new Frame displaying the record entries .
     */
    public RecordFrame(){
      getContentPane().setLayout(new BorderLayout());
      setSize(640,480);
    }

    /**
     * Sets the record from which the elements data comes. 
     */
    public void setSource(Dsplrecord record){
      qrNEW= new QueryResult(new SecondoObject("Elements",recVal),true);
      getContentPane().add(textDspl,BorderLayout.CENTER);
      textDspl.setViewportView(qrNEW);
      this.record = record;
      initElements();
    }

    /** 
     * Initializes the QueryResult.
     * The QueryResult is shown as list -> name(type):value
     */
    private void initElements(){
      ListExpr typeCopy = this.record.recTypes;
      ListExpr valCopy = this.record.recVal;
      ListExpr rest = null;
      ListExpr val = null;
      String typename = "";
      try{
        if(!typeCopy.isEmpty()){
          rest = typeCopy.rest();
          val = valCopy;
          // get first element
          if(rest.first().second().isAtom()){
            // element is atom
            typename = rest.first().second().symbolValue();
            Class t = Class.forName("viewer.hoese.algebras.Dspl" + typename);
            DsplGeneric elementClass = (DsplGeneric)t.newInstance();
            // initialize element
            if(val.first()!=null){
              elementClass.init(rest.first().first().toString() + " (" + typename.trim() + ")",0,0,rest.first().second(),val.first(),qrNEW);
            }
            else{
              elementClass.init(rest.first().first().toString() + " (" + typename.trim() + ")",0,0,rest.first().second(),val,qrNEW);
            }
          }
          if(!rest.first().second().isAtom()){
            // element is not atom 
            typename = rest.first().second().first().symbolValue();
            Class t = Class.forName("viewer.hoese.algebras.Dspl" + typename);
            DsplGeneric elementClass = (DsplGeneric)t.newInstance();
            // initialize element
            elementClass.init(rest.first().first().toString() + " (" + rest.first().second().toString().trim() + ")",0,0,rest.first().second(),val.first(),qrNEW);
          }
          // iterate the rest of the record elements
          while(!rest.rest().isEmpty()){
            // get next element
            rest = rest.rest();
            if(val!=null){
              val = val.rest();
            }
            if(rest.first().second().isAtom()){
              // element is atom 
              typename = rest.first().second().symbolValue();
              Class tN = Class.forName("viewer.hoese.algebras.Dspl" + typename);
              DsplGeneric elementClassN = (DsplGeneric)tN.newInstance();
              // initialize element
              if(val!=null){
                elementClassN.init(rest.first().first().toString() + " (" + typename.trim() + ")",0,0,rest.first().second(),val.first(),qrNEW);
              }
              else{
                elementClassN.init(rest.first().first().toString() + " (" + typename.trim() + ")",0,0,rest.first().second(),new ListExpr(),qrNEW);
              }
            }
            if(!rest.first().second().isAtom()){
              // element is not atom
              typename = rest.first().second().first().symbolValue();
              Class tN = Class.forName("viewer.hoese.algebras.Dspl" + typename);
              DsplGeneric elementClassN = (DsplGeneric)tN.newInstance();
              // initialize element
              elementClassN.init(rest.first().first().toString() + " (" + rest.first().second().toString().trim() + ")",0,0,rest.first().second(),val.first(),qrNEW);
            }
          }
        }
      }
      catch(ClassNotFoundException e){
        Reporter.showError("No Displayclass found for record element type: " + typename);
      }catch(InstantiationException e){
        Reporter.showError("Displayclass for record element type " + typename + " could not be initialized!");
      }catch(IllegalAccessException e){
        Reporter.showError("Error while accessing DisplayClass for record element type: " + typename);
      }

    }

    /** 
     * Returns the source of the set data. 
     */
    public Dsplrecord getSource(){
      return record;
    }

  }

  /** Embedded class ShapeSet.
   * A class to handle the ShapeVector
   */
  protected class ShapeSet{

    private int[] set;
    private int lastSearch;
    private int lastPosition;

    /** Constructor.
     * @param position
     * @param count
     */
    public ShapeSet(int position,int count){
      set=new int[count];  
      for(int i=0;i<count;i++){
        set[i]=position+i;
      }
    }

    /**
     * A method to determine wether the shape is from a record element.
     * @param num
     * @return boolean
     */
    public boolean hasNumber(int num){
      lastSearch=num;		
      for(int i=0;i<set.length;i++){
        if(set[i]==num){
	  lastPosition=i;
	  return true;
        }
      }
      return false;
    }

    /**
     * A method to get the position of the given shape in the shapeVector 
     * @param num the number of the shape
     * @return the postion in the shapeVector
     */
    public int getPosition(int num){
      if(num==lastSearch) return lastPosition;
      else{
        for(int i=0;i<set.length;i++){
          if(set[i]==num){
	    return i;
          }
        }
      }
      return 0;
    }

  }

}