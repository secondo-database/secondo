

package  viewer.hoese;

import  java.awt.*;
import  java.awt.event.*;
import  javax.swing.*;
import  java.awt.geom.*;
import  java.util.*;
import  viewer.HoeseViewer;


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
/** A scalable background-image used as lowest layer */
  ScaledLabel BackLabel;
/** Main-application */
  HoeseViewer mw;

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
        ((JComponent)com[0]).setVisible(((JToggleButton)evt.getSource()).isSelected());
      }
    };
  }

  /**
   * Craetes a copy of this object.
   * @return A cloned object
   * @exception CloneNotSupportedException
   * @see <a href="Categorysrc.html#clone">Source</a>
   */
  public Object clone () throws CloneNotSupportedException {
    return  super.clone();
  }

  /**
   * Gets the applications transformation
   * @return The actual Transformation
   * @see <a href="Categorysrc.html#getProjection">Source</a>
   */
  public AffineTransform getProjection () {
    return  mw.allProjection;
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
    /*		r=(int)(Math.random()*256);
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
   * Creates the back-image-layer out of an imagefile with offsets xo,yo
   * @param PathToImage A imagefilename String
   * @return The layertoggle of this layer
   * @see <a href="Categorysrc.html#createBackLayer">Source</a>
   */
  public JToggleButton createBackLayer (String PathToImage, double xo, double yo) {
    if (BackLabel != null)
      remove(BackLabel);
    if (PathToImage == null) {
      BackLabel = null;
      return  null;
    }
    BackLabel = new ScaledLabel(new ImageIcon(PathToImage), JLabel.LEFT);
    BackLabel.setVerticalAlignment(JLabel.TOP);
    BackLabel.setBounds((int)xo, (int)yo, (int)(-xo + mw.BBoxDC.getWidth()),
        (int)(-yo + mw.BBoxDC.getHeight()));
    add(BackLabel, new Integer(-1));
    JToggleButton jt = new JToggleButton();
    jt.setSelected(true);
    jt.setAlignmentX(Component.CENTER_ALIGNMENT);
    jt.setPreferredSize(new Dimension(10, 10));
    jt.addActionListener(LayerButtonListener);
    jt.setActionCommand("-1");
    return  jt;
  }

  /**
   * Adds the graph. objects in Vector grob to a new layer on the top
   * @param grob
   * @return The layertoggle of this layer
   */
  public JToggleButton addLayerObjects (Vector grob) {
    Category acat;
    if (mw.isAutoCatMI.isSelected()) {
      acat = createAutoCat();
      ListIterator li = grob.listIterator();
      while (li.hasNext()) {
        DsplGraph dg = ((DsplGraph)li.next());
	if(dg==null)
	   System.out.println("viewer.hoese.GraphWindow .addLayerObjects has received a null object");
	else
           dg.setCategory(acat);
        //dg.LabelText=dg.getAttrName();
      }
    }
    else
      newQueryRepresentation(grob);             // spaeter werden hier aus Viewconfig den GOs die Cats,Labels zugeordnet
    Layer lay = new Layer(grob, this);
    //	add (lay,new Integer(highestLayer()+1));
    int Laynr = ++highest;
    add(lay, new Integer(Laynr));
    mw.updateViewParameter();
    return  lay.CreateLayerButton(LayerButtonListener, Laynr);
  }

  /**
   * Adds the Layer l to the top of layerstack
   * @param l
   * @return The layertoggle of this layer
   * @see <a href="Categorysrc.html#addLayer">Source</a> 
   */
  public JToggleButton addLayer (Layer l) {
    //	add (lay,new Integer(highestLayer()+1));
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
      System.out.println("Exception in : GraphWindow.newQueryRep "+e );
      e.printStackTrace();
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
    Rectangle rDC = new Rectangle(0, 0, 0, 0);
    Rectangle2D.Double r = null;
    Interval in = null;
    for (int i = 0; i < getComponentCount(); i++)
      if (getComponent(i) instanceof Layer) {
        Layer l = (Layer)getComponent(i);
        if (r == null)
          r = l.getWorldCoordBounds(); 
        else 
          r = (Rectangle2D.Double)r.createUnion(l.getWorldCoordBounds());
        if (l.getTimeBounds() != null)
          if (in == null)
            in = l.getTimeBounds(); 
          else 
            in = in.union(l.getTimeBounds());
      }
    if (r != null)
      mw.BBoxWC = r;
    //mw.TimeBounds=in;
    mw.setActualTime(in);
    //			try{
  }

  /**
   * draws all layers
   * @param g
   * @see <a href="Categorysrc.html#paintChildren">Source</a> 
   */
  public void paintChildren (Graphics g) {
    Graphics2D g2 = (Graphics2D)g;
    g2.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL, RenderingHints.VALUE_STROKE_PURE);
    super.paintChildren(g2);
    DsplGraph dg = mw.getSelGO();

    if ((dg != null) && (dg.getVisible())){
      dg.draw(g2);              //the selected GraphObject must be drawn on top
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


/** A Scalable JLabel for the background image
  */
  class ScaledLabel extends JLabel {
    public ScaledLabel (Icon image, int h) {
      super(image, h);
    }

    public void paintComponent (Graphics g) {
     // if (actZF != mw.getZoomFactor()){
     // 	System.out.println(actZF);
     // 	Image im=((ImageIcon)getIcon()).getImage();
     // 	actZF=mw.getZoomFactor();
     // 	im=im.getScaledInstance((int)(iWidth*actZF),-1,Image.SCALE_SMOOTH);
     // 	setIcon(new ImageIcon(im));
     // 	System.out.println(actZF);
     // }
      ((Graphics2D)g).scale(mw.getZoomFactor(), mw.getZoomFactor());
      super.paintComponent(g);
    }
  }
}





