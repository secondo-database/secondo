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
	
	/* new implementation of showIt */
	/* relative to the screen's resolution */
	
	if (elemList.isEmpty()) {
	    System.out.println("GFXout.showIt: No elements to show.");
	    return;
	}//if

	elemList = elemList.copy();

	//add cross for origin at (0,0)
	elemList.addFirst(new Segment(-10,0,10,0));
	elemList.addFirst(new Segment(0,-10,0,10));

	//find maxX,maxY values
	Rational maxX = ((Element)(elemList.getFirst())).rect().lr.x;
	Rational maxY = ((Element)(elemList.getFirst())).rect().ul.y;
	Rational minX = ((Element)(elemList.getFirst())).rect().ll.x;
	Rational minY = ((Element)(elemList.getFirst())).rect().ll.y;
	
	ListIterator lit = elemList.listIterator(0);
	while (lit.hasNext()) {
	    Element e = (Element)lit.next();
	    if (e.rect().lr.x.greater(maxX)) maxX = e.rect().lr.x.copy();
	    if (e.rect().ul.y.greater(maxY)) maxY = e.rect().ul.y.copy();
	    if (e.rect().ll.x.less(minX)) minX = e.rect().ll.x.copy();
	    if (e.rect().ll.y.less(minY)) minY = e.rect().ll.y.copy();
	}//while

	//must objects be moved?
	boolean move = false;
	Rational moveX = null;
	Rational moveY = null;
	Rational borderMove = RationalFactory.constRational(10); //10 pixels as a border
	if (minX.less(0)) {
	    move = true;
	    moveX = minX.copy().abs();
	}//if
	else moveX = RationalFactory.constRational(0);
	if (minY.less(0)) {
	    move = true;
	    moveY = minY.copy().abs();
	}//if
	else moveY = RationalFactory.constRational(0);
	moveX = moveX.plus(borderMove);
	moveY = moveY.plus(borderMove);
	
	System.out.println("move: "+move+", moveX: "+moveX+", moveY: "+moveY);

	System.out.println("maxX/maxY: "+maxX.toString()+"/"+maxY.toString());
	System.out.println("minX/minY: "+minX.toString()+"/"+minY.toString());
	System.out.println("elemList.size(): "+elemList.size());
	System.out.println(elemList.size()-2+" element(s) to show.");

	//compute transX, transY, trans
	
	//get resolution
	Toolkit tk = Toolkit.getDefaultToolkit();
	Dimension dim = tk.getScreenSize();
	Rational dimX = RationalFactory.constRational(dim.getWidth());
	Rational dimY = RationalFactory.constRational(dim.getHeight());
	
	Rational transX;
	Rational transY;
	Rational trans;
	Rational tenPercentX = dimX.times(10).dividedby(100);
	Rational tenPercentY = dimY.times(10).dividedby(100);
	Rational borderX = RationalFactory.constRational(dimX.minus(tenPercentX));
	Rational borderY = RationalFactory.constRational(dimY.minus(tenPercentY));

	System.out.println("borderX: "+borderX+", borderY: "+borderY);

	transX = borderX.dividedby(maxX.minus(minX));
	System.out.println("transX = "+borderX+" / ("+maxX+" - "+minX+") = "+transX);
	transY = borderY.dividedby(maxY.minus(minY));
	//transX = (dimX.minus(borderX)).dividedby(maxX.minus(minX));
	//transY = (dimY.minus(borderY)).dividedby(maxY.minus(minY));

	if (transX.comp(transY) == -1) trans = transX;
	else trans = transY;
	trans = trans.times(90).dividedby(100);
	trans.round(0);
	
	//trans = RationalFactory.constRational(1);

	System.out.println("transX: "+transX+", transY: "+transY+", trans: "+trans);

	//transpose the elements
	lit = elemList.listIterator(0);
	Element e = null;
	if (move) {
	    maxX = maxX.plus(moveX).plus(10);
	    maxY = maxY.plus(moveY).plus(10);
	}//if
	System.out.println("new maxX: "+maxX+" new maxY: "+maxY);
	//System.out.println("+++++ transposition +++++");
	while (lit.hasNext()) {
	    e = (Element)lit.next();
	    
	    if (e instanceof Point) {
		//System.out.println("Point(I): "); e.print();
		//move point
		if (move) ((Point)e).set(((Point)e).x.plus(moveX),((Point)e).y.plus(moveY));
		//System.out.println("Point(II): "); e.print();
		//invert y-value
		((Point)e).set(((Point)e).x,maxY.minus(((Point)e).y));
		//transpose point
		//System.out.println("Point(III): "); e.print();
		((Point)e).set(((Point)e).x.times(trans),((Point)e).y.times(trans));
		//System.out.println("Point(IV): "); e.print();		
	    }

	    if (e instanceof Segment) {
		//System.out.println("Segment(I): "); e.print();
		//move segment
		if (move) ((Segment)e).set(((Segment)e).startpoint.set(((Segment)e).startpoint.x.plus(moveX),
								       ((Segment)e).startpoint.y.plus(moveY)),
					   ((Segment)e).endpoint.set(((Segment)e).endpoint.x.plus(moveX),
								     ((Segment)e).endpoint.y.plus(moveY)));
		//System.out.println("Segment(II): "); e.print();
		//invert y-value
		((Segment)e).set(((Segment)e).startpoint.set(((Segment)e).startpoint.x,
							     maxY.minus(((Segment)e).startpoint.y)),
				 ((Segment)e).endpoint.set(((Segment)e).endpoint.x,
							   maxY.minus(((Segment)e).endpoint.y)));
		//System.out.println("Segment(III): "); e.print();
		//transpose segment
		((Segment)e).set(((Segment)e).startpoint.set(((Segment)e).startpoint.x.times(trans),
							     ((Segment)e).startpoint.y.times(trans)),
				 ((Segment)e).endpoint.set(((Segment)e).endpoint.x.times(trans),
							   ((Segment)e).endpoint.y.times(trans)));
		//System.out.println("Segment(IV): "); e.print();
	    }//if
	    
	    if (e instanceof Triangle) {
		//System.out.println("Triangle(I): "); e.print();
		//move triangle
		((Triangle)e).set(((Triangle)e).vertices[0].set(((Triangle)e).vertices[0].x.plus(moveX),
								((Triangle)e).vertices[0].y.plus(moveY)),
				  ((Triangle)e).vertices[1].set(((Triangle)e).vertices[1].x.plus(moveX),
								((Triangle)e).vertices[1].y.plus(moveY)),
				  ((Triangle)e).vertices[2].set(((Triangle)e).vertices[2].x.plus(moveX),
								((Triangle)e).vertices[2].y.plus(moveY)));
		//System.out.println("Triangle(II): "); e.print();
		//invert y-value
		((Triangle)e).set(((Triangle)e).vertices[0].set(((Triangle)e).vertices[0].x,
								maxY.minus(((Triangle)e).vertices[0].y)),
				  ((Triangle)e).vertices[1].set(((Triangle)e).vertices[1].x,
								maxY.minus(((Triangle)e).vertices[1].y)),
				  ((Triangle)e).vertices[2].set(((Triangle)e).vertices[2].x,
								maxY.minus(((Triangle)e).vertices[2].y)));
		//System.out.println("Triangle(III): "); e.print();
		//transpose triangle
		((Triangle)e).set(((Triangle)e).vertices[0].set(((Triangle)e).vertices[0].x.times(trans),
								((Triangle)e).vertices[0].y.times(trans)),
				  ((Triangle)e).vertices[1].set(((Triangle)e).vertices[1].x.times(trans),
								((Triangle)e).vertices[1].y.times(trans)),
				  ((Triangle)e).vertices[2].set(((Triangle)e).vertices[2].x.times(trans),
								((Triangle)e).vertices[2].y.times(trans)));
		//System.out.println("Triangle(IV): "); e.print();
	    }//if

	    if (e instanceof Polygons) {
		ListIterator lit2 = ((Polygons)e).triangles().listIterator(0);
		Triangle t;
		while (lit2.hasNext()) {
		    t = (Triangle)lit2.next();
		    //move polygons
		    t.set(t.vertices[0].set(t.vertices[0].x.plus(moveX),
					    t.vertices[0].y.plus(moveY)),
			  t.vertices[1].set(t.vertices[1].x.plus(moveX),
					    t.vertices[1].y.plus(moveY)),
			  t.vertices[2].set(t.vertices[2].x.plus(moveX),
					    t.vertices[2].y.plus(moveY)));
		    //invert y-value
		    t.set(t.vertices[0].set(t.vertices[0].x,
					    maxY.minus(t.vertices[0].y)),
			  t.vertices[1].set(t.vertices[1].x,
					    maxY.minus(t.vertices[1].y)),
			  t.vertices[2].set(t.vertices[2].x,
					    maxY.minus(t.vertices[2].y)));
		}//while
	    }//if
	}//while

		//find maxX,maxY values
	Rational newMaxX = ((Element)(elemList.getFirst())).rect().lr.x;
	Rational newMaxY = ((Element)(elemList.getFirst())).rect().ul.y;
	Rational newMinX = ((Element)(elemList.getFirst())).rect().ll.x;
	Rational newMinY = ((Element)(elemList.getFirst())).rect().ll.y;
	
	lit = elemList.listIterator(0);
	while (lit.hasNext()) {
	    e = (Element)lit.next();
	    if (e.rect().lr.x.greater(newMaxX)) newMaxX = e.rect().lr.x.copy();
	    if (e.rect().ul.y.greater(newMaxY)) newMaxY = e.rect().ul.y.copy();
	    if (e.rect().ll.x.less(newMinX)) newMinX = e.rect().ll.x.copy();
	    if (e.rect().ll.y.less(newMinY)) newMinY = e.rect().ll.y.copy();
	}//while
	    
	ShapeSeg shapeSeg = new ShapeSeg(elemList);
	shapeSeg.init();
	f.getContentPane().add(shapeSeg, BorderLayout.CENTER);
	//f.setSize(new Dimension(borderX.getInt(), borderY.getInt()));
	f.setSize(new Dimension(newMaxX.plus(newMaxX.times(10).dividedby(100)).getInt(),
				newMaxY.plus(newMaxY.times(10).dividedby(100)).getInt()));
	System.out.println("frameSize: "+borderX.getInt()+" * "+borderY.getInt());
	//System.out.println("\nelementlist:"); elemList.print();
	f.setVisible(true);
	
    }//end method showIt
    

    public static void kill(){
	f.dispose();
    }
    
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
