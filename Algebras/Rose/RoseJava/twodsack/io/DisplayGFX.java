/*
 * DisplayGFX.java 2004-11-10
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.io;

import twodsack.set.*;
import twodsack.setelement.*;
import twodsack.setelement.datatype.basicdatatype.*;
import twodsack.setelement.datatype.compositetype.*;
import twodsack.util.collectiontype.*;
import twodsack.util.number.*;
import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.geom.AffineTransform;
import java.awt.geom.GeneralPath;
import java.awt.geom.*;
import java.util.Iterator;
import java.util.LinkedList;
import javax.swing.BorderFactory;
import javax.swing.JFrame;
import javax.swing.JPanel;

/**
 * The DisplayGFX class can be used to display the geometrical objects of the 2DSACK algebra.
 * When correctly used, a window opens that shows the objects put into the instance of 
 * DisplayGFX.
 * Use DisplayGFX as follows:<p><ol>
 * <li>define a new instance of DisplayGFX
 * <li>call {@link #initWindow} for the instance
 * <li>add all elements you want to be shown to DisplayGFX using {@link #addSet(ElemMultiSet)}
 * <li>call {@link #showIt} to finally open the graphics window
 * <li>use {@link #kill} to destroy the window
 * </ol><p>
 * There are different colors used for every set added to DisplayGFX. These colors are defined
 * in a color table inside of the class and cannot be changed. The objects that are shown, will be 
 * enlarged to a certain size depending on the screen's resolution and the actual window size.
 * The size for points put in the DisplayGFX instance is fixed. This can be annoying, if the
 * coordinates of the other data have very small values. In this case, use the <tt>zoom()</tt> function
 * implemented for every Element type before adding the objects to the DisplayGFX instance.<p>
 * You can only have <u>one</u> window opened at a time.
 */
public class DisplayGFX {
    /*
     * fields
     */
    static JFrame f;
    static LinkedList emsList;


    /*
     * constructors
     */
    
    /**
     * The 'empty' constructor for DisplayGFX. Use this to construct a new instance.
     * The internal list of objects is reset when calling this constructor.
     */
    public DisplayGFX() {
	emsList = new LinkedList();
    }

    /*
     * methods
     */

    /**
     * Use this method to make some additional initializations for the graphics window.
     */
    public static void initWindow() {
	f = new JFrame("AlgebraViewer");
	
	f.addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
		    //System.exit(0);
		    f.dispose(); } } );
	//f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    }//end method initWindow
    

    /**
     * This method destroys the actual graphics window.
     * When kill() has finished, the exection of the Java program continues.
     */
    public static void kill() {
	f.dispose();
    }//end method kill

    /**
     * Sets of elements can be added by this method.
     * Supported by DisplayGFX are objects of the type ElemMultiSet. Currently, display methods
     * are implemented for {@link twodsack.set.PointMultiSet}, {@link twodsack.set.SegMultiSet} and {@link twodsack.set.TriMultiSet}.
     * If you want to display
     * composite types, e.g. an instance of type {@link twodsack.setelement.datatype.compositetype.Polygons}, you have to pass the polygons border
     * as a {@link twodsack.set.SegMultiSet} or its interior passed as a {@link twodsack.set.TriMultiSet}.
     *
     * @param ems the set of elements that shall be added to DisplayGFX
     */
    public static void addSet (ElemMultiSet ems) {
	if (!ems.isEmpty())
	    emsList.addLast(ems);
    }//end method addSet

    /**
     * Opens the window and draws all of the objects stored in DisplayGFX.
     * Initial size of the window is 640*480.
     *
     * @param setCross draws a cross in the origin of the coordinate system (currently not implemented, use <tt>false</tt>)
     */
    public static void showIt (boolean setCross) {
	if (emsList.isEmpty()) {
	    System.out.println("DisplayGFX.showIt(): No elements to show.");
	    return;
	}//if

	ShapeBuilder mySB = new ShapeBuilder(emsList,f);
	mySB.init();
	f.getContentPane().add(mySB, BorderLayout.CENTER);
	//System.out.println("Showing elements in JAVA window.");
	f.setSize(new Dimension(640,480));
	
	f.setVisible(true);
    }//end method showIt

}//end class DisplayGFX

/**
 * An internal class that actually draws the objects and transforms them appropriate to the window size
 * and screen resolution. Here, the color matrix is defined. This class is used <u>only</u> by 
 * class DisplayGFX.
 */
class ShapeBuilder extends JPanel {
    /*
     * fields
     */
    //final double pointSize = 0.00005; //size for points in point sets
    final double pointSize = 2; //size for points in point sets
    final Color bg = Color.white;
    final Color fg = Color.black;
    LinkedList emsList;
    Dimension screen;
    double objectsSpaceX;
    double objectsSpaceY;
    Rect objectsBbox;
    AffineTransform myAT;
    JFrame actFrame;
    Color[] colorMatrix = new Color[40];
    int emsCounter;
    
    //standardColorMatrix defines the base colors for the objects. The colors are made 
    //translucent later when they are used.
    Color[] standardColorMatrix = { Color.BLACK, Color.BLUE, Color.RED, Color.GREEN,
				    Color.GRAY, Color.MAGENTA, Color.ORANGE, Color.PINK,
				    Color.CYAN, Color.YELLOW };

    /*
     * constructors
     */
    /**
     * Constructs a new ShapeBuilder instance.
     * @param ems the set of objects that shall be drawn.
     * @param myJF the JFrame that holds the drawing area.
     */
    public ShapeBuilder (LinkedList ems, JFrame myJF) {
	this.emsList = ems;
	this.actFrame = myJF;

	//find lowest and highest x,y-coordinates in ems
	Element firstEL = (Element)((ElemMultiSet)ems.getFirst()).first();
	Rect firstRect = firstEL.rect();
	double ulx = firstRect.ulx.getDouble();
	double uly = firstRect.uly.getDouble();
	double lrx = firstRect.lrx.getDouble();
	double lry = firstRect.lry.getDouble();
	
	Iterator it = ems.iterator();
	Iterator it2;
	ElemMultiSet actEMS;
	Rect actRect;
	while (it.hasNext()) {
	    actEMS = (ElemMultiSet)it.next();
	    it2 = actEMS.iterator();
	    while (it2.hasNext()) {
		actRect = ((Element)((MultiSetEntry)it2.next()).value).rect();
		if (ulx > actRect.ulx.getDouble()) ulx = actRect.ulx.getDouble();
		if (uly < actRect.uly.getDouble()) uly = actRect.uly.getDouble();
		if (lrx < actRect.lrx.getDouble()) lrx = actRect.lrx.getDouble();
		if (lry > actRect.lry.getDouble()) lry = actRect.lry.getDouble();
	    }//while it2

	    this.objectsBbox = new Rect(RationalFactory.constRational(ulx),
					RationalFactory.constRational(uly),
					RationalFactory.constRational(lrx),
					RationalFactory.constRational(lry));
	    this.objectsSpaceX = Math.abs((this.objectsBbox.lrx.minus(this.objectsBbox.ulx)).getDouble());
	    this.objectsSpaceY = Math.abs((this.objectsBbox.uly.minus(this.objectsBbox.lry)).getDouble());
	}//while it
	
	//fill colorMatrix
	int baseR = 0;
	int baseG = 30;
	int baseB = 0;
	for (int i = 0; i < colorMatrix.length; i++) {
	    colorMatrix[i] = new Color(baseR,baseG,baseB);
	    baseG = baseG + (255-30)/40;
	}//for i
    }//end constructor ShapeBuilder

    /*
     * methods
     */
    /**
     * Used to initialize the JFrame. Colors, borders and such are defined here.
     */
    public void init() {
	setBackground(bg);
	setForeground(fg);
	setBorder(BorderFactory.createCompoundBorder(BorderFactory.createRaisedBevelBorder(),
						     BorderFactory.createLoweredBevelBorder()));
	Toolkit myTK = Toolkit.getDefaultToolkit();
	this.screen = myTK.getScreenSize();
	this.myAT = new AffineTransform();
    }//end method init


    /**
     * Clears the background and draws the objects passed in the constructor in the drawing
     * area. Transformations are used to draw the objects in a proper size according to their
     * own coordinates, the screen resolution and the drawing area size.
     * This method is automatically called everytime when the JFrame window has to be redrawn.
     *
     * @param g1 the Graphics object
     */
    public void paint (Graphics g1) {
	this.emsCounter = 0;
	Graphics2D g = (Graphics2D)g1;
	
	//clear the background
	super.paintComponent(g);
	
	this.myAT.setToIdentity();

	double frameH = actFrame.getSize().getHeight();
	double frameW = actFrame.getSize().getWidth();
	Insets actInsets = actFrame.getInsets();
	double paintHeight = (frameH-10-actInsets.top-actInsets.bottom);
	double min = Math.min((frameW-10-actInsets.right-actInsets.left) / objectsSpaceX,
			      paintHeight / objectsSpaceY);
	myAT.setTransform(min,0,0,-min,-objectsBbox.llx.getDouble()*min+5,objectsBbox.lly.getDouble()*min+paintHeight+5);
	g.setTransform(myAT);
	
	//set stroke
	g.setStroke(new BasicStroke((float)(1.0f / min),BasicStroke.CAP_BUTT,BasicStroke.JOIN_MITER));

	Iterator it = this.emsList.iterator();
	Object actO;
	
	//Now, the object list is traversed. It may only contain instances of the following types:
	// - PointMultiSet
	// - SegMultiSet
	// - TriMultiSet
	//Depending on their type they are drawn.
	while (it.hasNext()) {
	    actO = it.next();
	    
	    if (actO instanceof PointMultiSet) {
		//The object is of type PointMultiSet!
		PointMultiSet pms = (PointMultiSet)actO;
		Iterator pit = pms.iterator();
		Point actPoint;
		g.setColor(standardColorMatrix[emsCounter % 10]);
		//Every point is drawn as a small rectangle of side length 4. This may be
		//inappropriate for some object coordinates. In that case, change the 
		//value of sideLength below.
		double sideLength = pointSize;
		while (pit.hasNext()) {
		    actPoint = (Point)((MultiSetEntry)pit.next()).value;
		    GeneralPath gp = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
		    gp.moveTo((float)(actPoint.x.getDouble() - sideLength/2),
			      (float)(actPoint.y.getDouble()));
		    gp.lineTo((float)(actPoint.x.getDouble()),
			      (float)(actPoint.y.getDouble() + sideLength/2));
		    gp.lineTo((float)(actPoint.x.getDouble() + sideLength/2),
			      (float)(actPoint.y.getDouble()));
		    gp.lineTo((float)(actPoint.x.getDouble()),
			      (float)(actPoint.y.getDouble() - sideLength/2));
		    gp.closePath();
		    g.fill(gp);
		    g.setColor(standardColorMatrix[emsCounter % 10]);
		    g.draw(gp);
		}//while pit
	    }//if PointMultiSet
	    
	    else if (actO instanceof SegMultiSet) {
		//The object is of type SegMultiSet!
		SegMultiSet sms = (SegMultiSet)actO;
		Iterator sit = sms.iterator();
		Segment actSeg;
		g.setColor(standardColorMatrix[emsCounter % 10]);
		while (sit.hasNext()) {
		    actSeg = (Segment)((MultiSetEntry)sit.next()).value;
		    g.draw(new Line2D.Double(actSeg.getStartpoint().x.getDouble(),
					     actSeg.getStartpoint().y.getDouble(),
					     actSeg.getEndpoint().x.getDouble(),
					     actSeg.getEndpoint().y.getDouble()));
		}//while sit
	    }//if SegMultiSet
	    
	    else if (actO instanceof TriMultiSet) {
		//The object is of type TriMultiSet
		TriMultiSet tms = (TriMultiSet)actO;
		Iterator tit = tms.iterator();
		Triangle actTri;
		Point[] vertices;
		
		/* The code below can be used to assing a color appropriate to the triangle's
		 * size. Pass the value computed to the setcolor() method.
		 */
		/*
		//find maximal and mininmal area values
		double minArea = ((Triangle)((MultiSetEntry)tit.next()).value).area();
		double maxArea = minArea;
		double actArea = 0;
		while (tit.hasNext()) {
		actTri = (Triangle)((MultiSetEntry)tit.next()).value;
		actArea = actTri.area();
		if (actArea < minArea) minArea = actArea;
		if (actArea > maxArea) maxArea = actArea;
		}//while
		double quot = (maxArea-minArea)/40;
		
		//System.out.println("maxArea: "+maxArea+", minArea: "+minArea+" quot: "+quot);
		*/
		tit = tms.iterator();
		
		int actColor = emsCounter % 10;
		g.setColor(standardColorMatrix[actColor]);

		//construct translucent color to fill triangles
		Color transCol = standardColorMatrix[actColor].brighter();
		transCol = new Color(transCol.getRed(),transCol.getGreen(),transCol.getBlue(),64);


		while (tit.hasNext()) {
		    actTri = (Triangle)((MultiSetEntry)tit.next()).value;
		    //actArea = actTri.area();
		    vertices = actTri.vertices();

		    GeneralPath gp = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
		    
		    gp.moveTo((float)actTri.vertices()[0].x.getDouble(),
			      (float)actTri.vertices()[0].y.getDouble());
		    gp.lineTo((float)actTri.vertices()[1].x.getDouble(),
			      (float)actTri.vertices()[1].y.getDouble());
		    gp.lineTo((float)actTri.vertices()[2].x.getDouble(),
			      (float)actTri.vertices()[2].y.getDouble());
		    gp.closePath();		     
		    g.setPaint(transCol);
		    g.fill(gp);
		    
		    g.setColor(standardColorMatrix[actColor]);
		   
		    g.draw(gp);
		}//while tit
	    }//if TriMultiSet
	    else if (actO instanceof Polygons) {
		//The object's type is Polygons!
		System.out.println("DisplayGFX: Display method for Polygons is currently not implemented.");
	    }//else Polygons
	    else {
		//Object is of unknown type. Cannot be drawn.
		System.out.println("ShapeBuilder.paintComponent(): Object is of unknown type. Cannot be displayed.");
	    }//else
	    emsCounter++;
	}//while it
    }//end method paintComponent

}//end class ShapeBuilder
