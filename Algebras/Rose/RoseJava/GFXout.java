import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

public class GFXout extends JApplet {
  //provides methods for drawing points, segments, triangles and polygons
  
    static JFrame f;
    static ElemList elemList;
    //static SegList segmentList;
    //static SegList triangleList;
    //static SegList polygonList;
    //static PointList pointList;

  public GFXout() {
      elemList = new ElemList();
      //initList();
      //mtList();
  }

  public static void initWindow(){
    //initializes the main window
    f = new JFrame("AlgebraViewer");
    
    f.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
	System.exit(0);
      }//end event windowClosing
    });

    f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
  }//end method init
  
  public static void showIt(){
    //shows the main window and draws all segments

      if (elemList.isEmpty()) {
	  System.out.println("GFXout.showIt: No elements to show.");
	  System.exit(0);
      }//if

    //find max x/y-values
    
      Rational maxX = new Rational(0);
      Rational maxY = new Rational(0);
      Rational minX = new Rational(0);
      Rational minY = new Rational(0);
      
      maxX = ((Element)(elemList.getFirst())).rect().lr.x;
      maxY = ((Element)(elemList).getFirst()).rect().ul.y;
      minX = ((Element)(elemList).getFirst()).rect().ll.x;
      minY = ((Element)(elemList).getFirst()).rect().ll.y;
      for (int i = 0; i < (elemList).size(); i++) {
	  Element e = (Element)(elemList).get(i);
	  if (e.rect().lr.x.greater(maxX)) { maxX = e.rect().lr.x.copy(); }
	  if (e.rect().ul.y.greater(maxY)) { maxY = e.rect().ul.y.copy(); }
	  if (e.rect().ll.x.less(minX)) { minX = e.rect().ll.x.copy(); }
	  if (e.rect().ll.x.less(minY)) { minY = e.rect().ll.x.copy(); }
      }//for i
      Point trans = new Point((new Rational(100)).plus(minX.abs()),(new Rational(-100)).plus(minY.abs()));
      //System.out.println("trans:"); trans.print();
      //trans.set(trans.x.plus(trans.x.times(new Rational(0.25))),trans.y.plus(trans.y.times(new Rational(0.25))));

      System.out.println("maxX/maxY: "+maxX.toString()+"/"+maxY.toString());
      System.out.println("minX/minY: "+minX.toString()+"/"+minY.toString());
      System.out.println("elemList has "+((LinkedList)elemList).size()+" elements");
    //put the drawings in the middle
	  
    //maxX = maxX.plus(maxX.times(new Rational(0.25)));
    //maxY = maxY.plus(maxY.times(new Rational(0.25)));
    //minX = minX.minus(minX.times(new Rational(0.25)));
    //minY = minY.minus(minY.times(new Rational(0.25)));
      //Rational mod = new Rational(100);
      //maxX = maxX.plus(mod);
      //maxY = maxY.plus(mod);
      //minX = minX.minus(mod);
      //minY = minY.minus(mod);
      //System.out.println("new maxX/maxY: "+maxX.toString()+"/"+maxY.toString());
      //System.out.println("new minX/minY: "+minX.toString()+"/"+minY.toString());
      //System.out.println("maxY: "+maxY);
      maxY = maxY.abs();
      //System.out.println("maxY(abs): "+maxY);
      maxY = maxY.plus(trans.y); //System.out.println("maxY(trans): "+maxY);
      maxY = maxX.plus(trans.x); //System.out.println("maxX(trans): "+maxX);
    //convert segmentlist
      for (int i = 0; i < elemList.size(); i++) {
	  Element e = (Element)elemList.get(i);
	  if (e instanceof Point) {
	      elemList.set(i,Mathset.sum((Point)e,trans));
	      ((Point)(elemList).get(i)).y =
		  maxY.minus(((Point)(elemList).get(i)).y);
	      //elemList.set(i,Mathset.sum((Point)e,trans));
	  }//if
	  if (e instanceof Segment) {
	      //((Segment)(elemList).get(i)).print();
	      elemList.set(i,((Segment)e).set(Mathset.sum(((Segment)e).startpoint,trans),Mathset.sum(((Segment)e).endpoint,trans)));
	      //((Segment)(elemList).get(i)).print();
	      //System.out.println("maxY: "+maxY);
	      ((Segment)(elemList).get(i)).startpoint.y =
		  maxY.minus(((Segment)(elemList).get(i)).startpoint.y);
	      ((Segment)(elemList).get(i)).endpoint.y =
		  maxY.minus(((Segment)(elemList).get(i)).endpoint.y);
	      //((Segment)elemList.get(i)).print();
	      //elemList.set(i,((Segment)e).set(Mathset.sum(((Segment)e).startpoint,trans),Mathset.sum(((Segment)e).endpoint,trans)));
	      //((Segment)elemList.get(i)).print();
	  }//if
	  if (e instanceof Triangle) {
	      Triangle t = (Triangle)(elemList).get(i);
	      elemList.set(i,((Triangle)e).set(Mathset.sum(((Triangle)e).vertices[0],trans),
					       Mathset.sum(((Triangle)e).vertices[1],trans),
					       Mathset.sum(((Triangle)e).vertices[2],trans)));
	      Triangle nt = new Triangle(new Point(t.vertices()[0].x,maxY.minus(t.vertices()[0].y)),
					 new Point(t.vertices()[1].x,maxY.minus(t.vertices()[1].y)),
					 new Point(t.vertices()[2].x,maxY.minus(t.vertices()[2].y)));
	      ((LinkedList)elemList).set(i,nt);
	      //elemList.set(i,((Triangle)e).set(Mathset.sum(((Triangle)e).vertices[0],trans),
	      //		     Mathset.sum(((Triangle)e).vertices[1],trans),
	      //		     Mathset.sum(((Triangle)e).vertices[2],trans)));
	  }//if
	  if (e instanceof Polygons) {
	      Polygons p = (Polygons)(elemList).get(i);
	      TriList tl = p.triangles();
	      for (int j = 0; j < tl.size(); j++) {
		  Triangle t = (Triangle)tl.get(j);
		  t.set(Mathset.sum(t.vertices[0],trans),Mathset.sum(t.vertices[1],trans),Mathset.sum(t.vertices[2],trans));
		  Triangle tn = new Triangle(new Point(t.vertices()[0].x,maxY.minus(t.vertices()[0].y)),
					     new Point(t.vertices()[1].x,maxY.minus(t.vertices()[1].y)),
					     new Point(t.vertices()[2].x,maxY.minus(t.vertices()[2].y)));
		  tl.set(j,tn);
	      }//for j
	      Polygons np = new Polygons(tl);
	      ((LinkedList)elemList).set(i,np);
	  }//if
      }//for i
    
    ShapeSeg shapeSeg = new ShapeSeg(elemList);
    shapeSeg.init();
    f.getContentPane().add(shapeSeg, BorderLayout.CENTER);
    maxX = maxX.plus(trans.x).plus(new Rational(200));
    maxY = maxY.plus(trans.y).plus(new Rational(400));
    f.setSize(new Dimension(maxX.getInt(), maxY.getInt()));
    f.setVisible(true);
  }//end method show

  private static void initList(){
    //initiates the segment list
      LinkedList elemList = new ElemList();
  }//end method initList


  public static void add(Element e){
    //accepts Element as input and decides which method to call
      ((LinkedList)elemList).add(e);
  }//end method add

  public static void addList(ElemList el){
    //accepts an ElList and calls add for every element
      for (int i = 0; i < ((LinkedList)el).size(); i++) {
	  add((Element)((LinkedList)el).get(i));
      }//for i
  }//end method addList

  private static void addPoint(Point p){
    //adds point p
      ((LinkedList)elemList).add(p);
  }//end method addPoint

  private static void addSegment(Segment s){
    //adds segment s
    ((LinkedList)elemList).add((Segment)s.copy());
  }//end method addSegment

  private static void addTriangle(Triangle t){
    //adds triangle t
    ((LinkedList)elemList).add(new Segment(t.vertices[0].x, t.vertices[0].y,
			     t.vertices[1].x, t.vertices[1].y));
    ((LinkedList)elemList).add(new Segment(t.vertices[1].x, t.vertices[1].y,
			     t.vertices[2].x, t.vertices[2].y));
    ((LinkedList)elemList).add(new Segment(t.vertices[2].x, t.vertices[2].y,
			     t.vertices[0].x, t.vertices[0].y));    
  }//end method addTriangle
  
  private static void addPolygon(Polygons p){
    //adds polygon p
    SegList sl = p.border();
    for (int i = 0; i < sl.size(); i++) {
	((LinkedList)elemList).add(sl.get(i));
    }//for i
  }//end method addPolygon

  private static void addPointList(PointList p){
    //adds all points in p
      for (int i = 0; i < p.size(); i++) {
	  addPoint((Point)p.get(i));
      }//for i
  }//end method addPointList

  private static void addSegmentList(SegList s){
    //adds all segments in s
    Segment el = new Segment();
    for (int i = 0; i < s.size(); i++) {
      el = (Segment)s.get(i);
      addSegment(el);
    }//for
  }//end method addSegmentList

  private static void addTriangleList(TriList t){
    //adds all triangles in t
    Triangle el = new Triangle();
    for (int i = 0; i < t.size(); i++) {
      el = (Triangle)t.get(i);
      addTriangle(el);
    }//for
  }//end method addTriangleList

  private static void addPolygonList(PolList p){
    //add all polygons in p
    Polygons el = new Polygons();
    for (int i = 0; i < p.size(); i++) {
      el = (Polygons)p.get(i);
      addPolygon(el);
    }//for
  }//end method addPolygonList
  
}

class ShapeSeg extends JPanel {
  final Color bg = Color.white;
  Color fg = Color.black;
  LinkedList elemList;

  public ShapeSeg (LinkedList e) {
    //setBackground(gb);
    //setForeground(fg);
    this.elemList = e;
    //setBorder(BorderFactory.createCompoundBorder(BorderFactory.createRaisedBevelBorder(),
    //						 BorderFactory.createLoweredBevelBorder()));
  }//end constructor ShapeSeg
  
  public void init(){
    setBackground(bg);
    setForeground(fg);
    setBorder(BorderFactory.createCompoundBorder(BorderFactory.createRaisedBevelBorder(),
						 BorderFactory.createLoweredBevelBorder()));
  }//end method init

  public void paintComponent(Graphics g) {
    Segment t = new Segment();
    super.paintComponent(g);
    for (int i = 0; i < ((LinkedList)elemList).size(); i++) {
	Element e = (Element)((LinkedList)elemList).get(i);
	if (e instanceof Point) {
	    Point p = (Point)e;
	    g.setColor(Color.black);
	    g.fillOval((p.x.minus(1)).getInt(), (p.y.minus(1)).getInt(), 3,3);
	}//if
	if (e instanceof Segment) {
	    Segment s = (Segment)e;  
	    g.setColor(Color.green);
	    g.drawLine(s.startpoint.x.getInt(), s.startpoint.y.getInt(),
		       s.endpoint.x.getInt(), s.endpoint.y.getInt());
	}//if
	if (e instanceof Triangle) {
	    Triangle tr = (Triangle)e;
	    g.setColor(Color.red);
	    g.drawLine(tr.vertices()[0].x.getInt(), tr.vertices()[0].y.getInt(),
		       tr.vertices()[1].x.getInt(), tr.vertices()[1].y.getInt());
	    g.drawLine(tr.vertices()[1].x.getInt(), tr.vertices()[1].y.getInt(),
		       tr.vertices()[2].x.getInt(), tr.vertices()[2].y.getInt());
	    g.drawLine(tr.vertices()[2].x.getInt(), tr.vertices()[2].y.getInt(),
		       tr.vertices()[0].x.getInt(), tr.vertices()[0].y.getInt());
	}//if
	if (e instanceof Polygons) {
	    Polygons p = (Polygons)e;
	    SegList sl = p.border();
	    for (int j = 0; j < sl.size(); j++) {
		Segment s = (Segment)sl.get(j);
		g.setColor(Color.blue);
		g.drawLine(s.startpoint.x.getInt(), s.startpoint.y.getInt(),
			   s.endpoint.x.getInt(), s.endpoint.y.getInt());
	    }//for j
	}//if 
	    //t = (Segment)segmentList.get(i);
	    //g.setColor(Color.green);
	    /*if (t.name == "green")
	      { g.setColor(Color.green); }
	      else if (t.name == "red")
	      { g.setColor(Color.red); }
	      else if (t.name == "cyan")
	      { g.setColor(Color.cyan); }
	      else if (t.name == "magenta")
	      { g.setColor(Color.magenta); }
	      else if (t.name == "orange")
	      { g.setColor(Color.pink); }
	      else if (t.name == "pink")
	      { g.setColor(Color.pink); }
	      else if (t.name == "yellow")
	      { g.setColor(Color.yellow); }      */
	    //g.drawLine(t.startpoint.x.getInt(), t.startpoint.y.getInt(),
	// t.endpoint.x.getInt(), t.endpoint.y.getInt());
    }//for
  }//end method pointComponent
}//end class ShapeSeg


class ShapesPanel extends JPanel {
  final int maxCharHeight = 15;
  final Color bg = Color.lightGray;
  final Color fg = Color.black;
  
  public ShapesPanel() {
    //Initialize drawing colors, border, opacity.
    setBackground(bg);
    setForeground(fg);
    setBorder(BorderFactory.createCompoundBorder(
						 BorderFactory.createRaisedBevelBorder(),
						 BorderFactory.createLoweredBevelBorder()));
  }
  
  public void clear(Graphics g) {
    //clears the background
    super.paintComponent(g);
  }//end method clear

  public void paintComponent(Graphics g) {
    super.paintComponent(g);      //clears the background
    
    Insets insets = getInsets();
    int currentWidth = getWidth() - insets.left - insets.right;
    int currentHeight = getHeight() - insets.top - insets.bottom;
    int gridWidth = currentWidth / 7;
    int gridHeight = currentHeight / 2;
    
    Color fg3D = Color.lightGray;
    int firstX = insets.left + 3;
    int firstY = insets.top + 3;
    int x = firstX;
    int y = firstY;
    int stringY = gridHeight - 7;
    int rectWidth = gridWidth - 2*x;
    int rectHeight = stringY - maxCharHeight - y;
    
    // drawLine(x1, y1, x2, y2) 
    g.drawLine(x, y+rectHeight-1, x + rectWidth, y);
    g.drawString("drawLine", x, stringY);
    x += gridWidth;
    
    /*
      
      // drawRect(x, y, w, h) 
      g.drawRect(x, y, rectWidth, rectHeight);
      g.drawString("drawRect", x, stringY);
      x += gridWidth;
      
      // draw3DRect(x, y, w, h, raised) 
      g.setColor(fg3D);
      g.draw3DRect(x, y, rectWidth, rectHeight, true);
      g.setColor(fg);
      g.drawString("draw3DRect", x, stringY);
      x += gridWidth;
      
      // drawRoundRect(x, y, w, h, arcw, arch) 
      g.drawRoundRect(x, y, rectWidth, rectHeight, 10, 10);
      g.drawString("drawRoundRect", x, stringY);
      x += gridWidth;
      
      // drawOval(x, y, w, h) 
      g.drawOval(x, y, rectWidth, rectHeight);
      g.drawString("drawOval", x, stringY);
      x += gridWidth;
      
      // drawArc(x, y, w, h) 
      g.drawArc(x, y, rectWidth, rectHeight, 90, 135);
      g.drawString("drawArc", x, stringY);
      x += gridWidth;
      
    */
    
    // drawPolygon(xPoints, yPoints, numPoints) 
    int x1Points[] = {x, x+rectWidth, x, x+rectWidth};
    int y1Points[] = {y, y+rectHeight, y+rectHeight, y};
    g.drawPolygon(x1Points, y1Points, x1Points.length); 
    g.drawString("drawPolygon", x, stringY);
    
    /*
      
      // NEW ROW
      x = firstX;
      y += gridHeight;
      stringY += gridHeight;
      
    */
    
    // drawPolyline(xPoints, yPoints, numPoints) 
    // Note: drawPolygon would close the polygon.
    int x2Points[] = {x, x+rectWidth, x, x+rectWidth};
    int y2Points[] = {y, y+rectHeight, y+rectHeight, y};
    g.drawPolyline(x2Points, y2Points, x2Points.length); 
    g.drawString("drawPolyline", x, stringY);
    x += gridWidth;
    
    /*
      // fillRect(x, y, w, h)
      g.fillRect(x, y, rectWidth, rectHeight);
      g.drawString("fillRect", x, stringY);
      x += gridWidth;
      
      // fill3DRect(x, y, w, h, raised) 
      g.setColor(fg3D);
      g.fill3DRect(x, y, rectWidth, rectHeight, true);
      g.setColor(fg);
      g.drawString("fill3DRect", x, stringY);
      x += gridWidth;
      
      // fillRoundRect(x, y, w, h, arcw, arch)
      g.fillRoundRect(x, y, rectWidth, rectHeight, 10, 10);
      g.drawString("fillRoundRect", x, stringY);
      x += gridWidth;
      
      // fillOval(x, y, w, h)
      g.fillOval(x, y, rectWidth, rectHeight);
      g.drawString("fillOval", x, stringY);
      x += gridWidth;
      
      // fillArc(x, y, w, h)
      g.fillArc(x, y, rectWidth, rectHeight, 90, 135);
      g.drawString("fillArc", x, stringY);
      x += gridWidth;
      
    */
    
    // fillPolygon(xPoints, yPoints, numPoints) 
    int x3Points[] = {x, x+rectWidth, x, x+rectWidth};
    int y3Points[] = {y, y+rectHeight, y+rectHeight, y};
    g.fillPolygon(x3Points, y3Points, x3Points.length); 
    g.drawString("fillPolygon", x, stringY);
  }
}
