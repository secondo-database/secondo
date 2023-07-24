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


package viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.font.*;
import java.awt.image.*;
import java.lang.Math;
import sj.lang.ListExpr;
import java.util.*;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JFileChooser;

import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import gui.SecondoObject;
import gui.idmanager.ID;
import viewer.Viewer3D;
import viewer.viewer3d.graphic2d.Figure2D;
import viewer.viewer3d.graphic2d.IDPoint2D;
import viewer.viewer3d.graphic2d.Triangle2D;
import viewer.viewer3d.graphic3d.*;
import viewer.viewer3d.objects.*;
//import viewer.viewer3d.graphic2d.Point2D;

/**
 * A Displayclass for an graph from the histogram2d-algebra
 */
public class Dsplhistogram2d extends DsplGeneric implements ExternDisplay
{
  /** The internal datatype representation */
      static ExtWin extWin = new ExtWin("Histogram2D");
  static double[] rangesXVec = null;
  static double[] rangesZVec = null;
  static double[] binsVec = null;
  static double[] rangesXVecOld = null;
  static double[] rangesZVecOld = null;
  static double[] binsVecOld = null;
  boolean undef = false;
  boolean err;
 // Rectangle2D.Double bounds=null;

  static ID id = new ID();


  /**
      * Scans the numeric representation of a histogram2d datatype
      *
      * @param v
      *            the nestedlist representation of the histogram
      *            ((rangeX*)(rangeY*)(bin*))
  */

      protected void ScanValue(ListExpr v)
  {

    int i = 0;  //for rangesX
    int k = 0;  //for rangesY
    int j = 0;  //for bins
    int c = 0;  //size rangesXVec
    int d = 0;  //size rangesYVec
    if ( isUndefined(v) ){
      undef = true;
      return;
    }

    if(v.listLength()!=3){
      Reporter.writeError("ListLength of the graph type must be two.");
      err=true;
      return;
    }

//     ListExpr rangesX = v.first();
//     ListExpr rangesZ = v.second();

    ListExpr rangesZ = v.first();
    ListExpr rangesX = v.second();


    ListExpr bins = v.third();

    ListExpr rangesXRest = rangesX;
    ListExpr rangesZRest = rangesZ;
    ListExpr binsRest = bins;

    ListExpr rangesXCount = rangesX;
    ListExpr rangesZCount = rangesZ;
//     ListExpr binsCount = bins;


    if(((rangesX.listLength()-1) * (rangesZ.listLength()-1)) != ( bins.listLength()) ){
      Reporter.writeError("Incorrect data format!");
      err=true;
      return;
    }

    while (!rangesXCount.isEmpty())
    {
      c++;
      rangesXCount = rangesXCount.rest();
    }
    while (!rangesZCount.isEmpty())
    {
      d++;
      rangesZCount = rangesZCount.rest();
    }

    this.binsVecOld = binsVec; //copy of ranges and bins to hold old data for second panel
    this.rangesXVecOld = rangesXVec;
    this.rangesZVecOld = rangesZVec;
    rangesXVec = new double[c];
    rangesZVec = new double[d];
    int binCount = (c-1) * (d-1);
    binsVec = new double[binCount];

    while (!rangesXRest.isEmpty())
    {
      ListExpr rangeX = rangesXRest.first();
      if(rangesXRest.first().atomType()!=ListExpr.REAL_ATOM){
        Reporter.writeError("invalid representation of a range found");
        return;
      }
      double rangeXD = rangeX.realValue();
//       System.out.println(" rangeXD: " + rangeXD);
      rangesXVec[i] = rangeXD;
      rangesXRest = rangesXRest.rest();
      i++;
    }

    while (!rangesZRest.isEmpty())
    {
      ListExpr rangeY = rangesZRest.first();
      if(rangesZRest.first().atomType()!=ListExpr.REAL_ATOM){
        Reporter.writeError("invalid representation of a range found");
        return;
      }
      double rangeZD = rangeY.realValue();
  //     System.out.println(" rangeYD: " + rangeZD);
      rangesZVec[k] = rangeZD;
      rangesZRest = rangesZRest.rest();
      k++;
    }

    while (!binsRest.isEmpty())
    {
      ListExpr bin = binsRest.first();
      if( binsRest.first().atomType()!=ListExpr.REAL_ATOM ){
        Reporter.writeError("invalid representation of a bin found");
        err=true;
        return;
      }
      double binD = bin.realValue();
//       System.out.println(" binD: " + binD);
      binsVec[j] = binD;
      binsRest = binsRest.rest();
      j++;
    }
//     System.out.println("ich bin durch im ScanValue!");
  }


  /**
      * Init. the Dsplhistogram2d instance.
      *
      * @param type
      *            The symbol histogram2d
      * @param value
      *            the nestedlist representation of the histogram2d
      * @param qr
      *            queryresult to display output.
  */

      public void init(String name, int nameLength, int indent, ListExpr type,
                       ListExpr value, QueryResult qr){
                         AttrName = extendString(name, nameLength, indent);
                         ScanValue(value);
                         if(undef){
                           qr.addEntry(new String(AttrName + ": undefined"));
                           return;
                         }
                         if (err){
                           Reporter.writeError("Error in ListExpr :parsing aborted");
                           qr.addEntry(new String("(" + AttrName + ": GA(graph))"));
                           return;
                         } else{
                           qr.addEntry(this);
//      extWin.setVisible(true);   //!!!GLEICH AM ANFANG FENSTER OFFEN
                         }
                       }


  /**
      * shows this histogram in an external window
  */
      public void displayExtern(){
    extWin.setSource(this);
    extWin.setVisible(true);
      }

      public boolean isExternDisplayed(){
        return this== extWin.hist2d && extWin.isVisible();
      }





  // #######################     E X T E R N   W I N D O W    ##################################
  /**
      * to show the histograms in an external window
  */
      private static class ExtWin extends JFrame{
    private  MyWorld3D w3dPanel1;
    private  MyWorld3D w3dPanel2;
    public Dsplhistogram2d hist2d = null;


    /** creates a new external window **/
        public ExtWin(String title){
      super(title);
      JPanel showPanel = new JPanel( (new GridLayout(1,2)));
      w3dPanel1 = new MyWorld3D();
      showPanel.add(w3dPanel1);
      w3dPanel2 = new MyWorld3D();
      showPanel.add(w3dPanel2);
//      int panelWidth = 350;
//      int panelHeight = 550;
      //to conform panel size to histogram2d
      Toolkit toolkit = Toolkit.getDefaultToolkit();
      Dimension screenSize = toolkit.getScreenSize();
      int size = Math.min(screenSize.width/2, screenSize.height);
      w3dPanel1.setPreferredSize(new Dimension(size-40, size-40));
      w3dPanel2.setPreferredSize(new Dimension(size-40, size-40));

      MouseNavigator mouseNavi = new MouseNavigator();
      w3dPanel1.addMouseListener(mouseNavi);
      w3dPanel1.addMouseWheelListener(mouseNavi);
      w3dPanel1.addMouseMotionListener(mouseNavi);
      w3dPanel2.addMouseListener(mouseNavi);
      w3dPanel2.addMouseWheelListener(mouseNavi);
      w3dPanel2.addMouseMotionListener(mouseNavi);
      mouseNavi.addViewReceiver(w3dPanel1);
      mouseNavi.addViewReceiver(w3dPanel2);
      mouseNavi.updateViewReceivers();//starting position
//      w3dPanel1.setKeyListener();
//      w3dPanel2.setKeyListener();
      this.validate();

      getContentPane().setLayout(new BorderLayout());
      getContentPane().add(showPanel,BorderLayout.CENTER);

      JPanel p2 = new JPanel();
      JButton saveLeft = new JButton("save left");
      saveLeft.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
          JFileChooser fc = new JFileChooser(".");
          if(fc.showSaveDialog(null)==JFileChooser.APPROVE_OPTION){
                //System.out.println(" PSEXPORT >>> ");
            extern.psexport.PSCreator.export(w3dPanel1,fc.getSelectedFile());
                //System.out.println("PSEXPORT <<<<");
          }
        }
      });
      p2.add(saveLeft);
      getContentPane().add(p2,BorderLayout.SOUTH);

      this.pack();


        }

        public void setSource(Dsplhistogram2d hist2d){
          this.hist2d = hist2d;
          this.w3dPanel1.setHistogram(hist2d.rangesXVec, hist2d.rangesZVec, hist2d.binsVec);
          this.w3dPanel2.setHistogram(hist2d.rangesXVecOld,hist2d.rangesZVecOld, hist2d.binsVecOld);
        }
      }


    //##############################    M O U S E   N A V I G A T O R     ##########################

  /**
      * to scoll in the scene and to switch the view of the scene by the mouse
  */
      public static class MouseNavigator extends MouseAdapter implements MouseWheelListener, MouseMotionListener{
    private static final double zoomStep = 0.05;
    /**count of complete rotations for moving the mouse one pixel*/
        private static final double rotationSpeed = 0.001;


    private Vector w3ds;

    private int mousePressedBeginX = 0;
    private int mousePressedBeginY = 0;

    /**the angle in the x-z plane starting at z and rotation counterclockwise (to positive x)*/
        private double theta = 35.0/360.0*2*Math.PI;
    /**the angle starting at 0 = y-axis and leading to x-z plane*/
        private double phi = 75.0/360.0*2*Math.PI;
    private double zoomFactor = 1.0;

    private double thetaAtDragStart = theta;
    private double phiAtDragStart = phi;

    public MouseNavigator(){
      this.w3ds = new Vector();
    }

    /**
        * @param receiver all added receivers will have set their view (camera)
    */
        public void addViewReceiver(World3D receiver){
      this.w3ds.add(receiver);
        }

        public void mouseWheelMoved(MouseWheelEvent e) {
          if (e.getWheelRotation() > 0){
            zoomFactor *= (1.0+zoomStep);
          }
          else{
            zoomFactor /= (1.0+zoomStep);
          }
          this.updateViewReceivers();
        }

        public void mousePressed(MouseEvent e){
          mousePressedBeginX = e.getX();
          mousePressedBeginY = e.getY();
          Iterator it = this.w3ds.iterator();

          while (it.hasNext()){
            MyWorld3D w3d = (MyWorld3D)(it.next());
            w3d.grabFocus();
            w3d.repaint();
          }
        }

        public void mouseReleased(MouseEvent e){
          thetaAtDragStart = theta;
          phiAtDragStart = phi;
        }

        public void mouseDragged(MouseEvent e){

          int mouseX = e.getX() - this.mousePressedBeginX;//relative position
          int mouseY = e.getY() - this.mousePressedBeginY;

          this.theta = this.thetaAtDragStart - mouseX*2*Math.PI*rotationSpeed;
          this.phi = this.phiAtDragStart - mouseY*2*Math.PI*rotationSpeed;
      //clamp phi inside interval of 5 degree and 175 degree to avoid gimbal lock
          if (this.phi < 5.0/360.0*2*Math.PI){
            this.phi = 5.0/360.0*2*Math.PI;
          }
          else if (this.phi > 175.0/360.0*2*Math.PI){
            this.phi = 175.0/360.0*2*Math.PI;
          }
          this.updateViewReceivers();
        }

        public void mouseMoved(MouseEvent arg0) {}

        public void updateViewReceivers(){
//      System.out.println("im updateViewRecivers 1");
      //theta, phi, and zoomFactor are ready
          double  camera_X_OnUnitCircle = Math.sin(this.theta)*Math.sin(this.phi)*this.zoomFactor;
          double  camera_Y_OnUnitCircle = Math.cos(this.phi)*this.zoomFactor;
          double camera_Z_OnUnitCircle = Math.sin(phi)*Math.cos(this.theta)*this.zoomFactor;

          Iterator it = this.w3ds.iterator();
          while (it.hasNext()){
            MyWorld3D w3d = (MyWorld3D)(it.next());
            double cameraDistance = w3d.cameraDistance;
//        System.out.println("updateViewReceiver");
            w3d.setView(camera_X_OnUnitCircle*cameraDistance+w3d.cameraTargetX,
                        camera_Y_OnUnitCircle*cameraDistance+w3d.cameraTargetY,
                        camera_Z_OnUnitCircle*cameraDistance+w3d.cameraTargetZ,
                        w3d.cameraTargetX, w3d.cameraTargetY, w3d.cameraTargetZ, 0, 1000, 0);
            double windowSize = (cameraDistance*this.zoomFactor);
            w3d.setWindow(windowSize, windowSize);
            w3d.validate();
            w3d.update();
            w3d.repaint();
          }
//      System.out.println("im updateViewRecivers end");
        }
      }

  //####################################################################################################################

  /**
      * to form my own World3D with its camera position, its ranges and bins and its mouse navigator
      * to write the paint method for MyWorld3D
  */
      private static class MyWorld3D extends World3D{

    double[] rangesXVecPaint = null;
    double[] rangesZVecPaint = null;
    double[] binsVecPaint = null;
    double[] binsHeightPaint = null;
    private Color mainColor = Color.white;
    MouseNavigator mouseNavi = new MouseNavigator();
    private static Stroke stroke = new BasicStroke(0.5f);

    int selectedBin = -1;

    public double cameraDistance = 250;
    public double cameraTargetX = 0;//point where the camera looks at
    public double cameraTargetY = 0;
    public double cameraTargetZ = 0;

    //to paint the coordinates
    public void paint(Graphics g){
      ((Graphics2D)g).setStroke(stroke);
      //System.out.println("im paint 1");
      try{
        super.paint(g);
      } catch(Exception e ){
        Reporter.debug(e);;
      }
      // System.out.println("im paint 2");

      if (this.rangesXVecPaint == null || this.rangesZVecPaint == null || this.binsHeightPaint ==  null){
        // no information
        //System.err.println("im paint 3");
        //System.err.println("rangesXVecPaint = " + rangesXVecPaint);
        //System.err.println("rangesZVecPaint = " + rangesZVecPaint);
        // System.err.println("binsHeightPaint = " + binsHeightPaint);

        return;
      }
      else{
        // System.out.println("im paint 4");
        FM3DGraphic Fm3D = new FM3DGraphic();
        try {
          Fm3D.setView(this.getEyeX(), this.getEyeY(), this.getEyeZ(),
                       this.getVRPX(), this.getVRPY(), this.getVRPZ(),
                       this.getViewUpX(), this.getViewUpY(), this.getViewUpZ());
        } catch (Exception e) {
          e.printStackTrace();
        }
        Fm3D.setWindow(this.getWindowX(), this.getWindowY());
        int min = Math.min(this.getWidth(),this.getHeight());
        Fm3D.setViewport(10,10,min-20,min-20);
        //Fm3D ready to transform

//        System.out.println("im paint 5");

        double minX = rangesXVecPaint[0];
        double maxX = rangesXVecPaint[rangesXVecPaint.length-1];
        double minZ = rangesZVecPaint[0];
        double maxZ = rangesZVecPaint[rangesZVecPaint.length-1];
        double maxY = this.getMax(this.binsHeightPaint);
        double minY = this.getMin(this.binsHeightPaint);
        double minXsmaller0;
        double minYsmaller0;
        double minZsmaller0;

        if (minY == 0.0 && maxY == 0.0){
          maxY = 10.0;
        }

        if(minX > 0.0){
          minXsmaller0 = 0.0;
        }
        else {
          minXsmaller0 = minX;
        }
        if(minY > 0.0){
          minYsmaller0 = 0.0;
        }
        else {
          minYsmaller0 = minY;
        }
        if(minX > 0.0){
          minZsmaller0 = 0.0;
        }
        else {
          minZsmaller0 = minZ;
        }

//        System.out.println("im paint 7");
        double unitX = 100/(Math.abs(maxX)+ Math.abs(minXsmaller0)); //units to paint in the panel
        double unitY = 100/(Math.abs(maxY)+ Math.abs(minYsmaller0));
        double unitZ = 100/(Math.abs(maxZ)+ Math.abs(minZsmaller0));

        Vector stepsX = this.paintSteps(maxX, minX);
        Vector stepsZ = this.paintSteps(maxZ, minZ);
        Vector stepsHeight = this.paintSteps(maxY, minY);
        Iterator itx = stepsX.iterator();
        int i = 0;

        if(minX > 0.0){
          minX = 0.0;
        }
        if(minZ > 0.0){
          minZ = 0.0;
        }

        g.setFont((new Font("Helvetica", Font.PLAIN, 9)));

        while  (itx.hasNext() ){
          //x vary  *unitX
          //z = 0
          //y = 0
          double itxNext =  ((Double)(itx.next())).doubleValue();
          String itxNextStr = Double.toString(itxNext);
          String itxNextStrShort =  itxNextStr.substring(0, Math.min(itxNextStr.length(),itxNextStr.indexOf(".")+3));

          Point transformed = this.transformPoint((maxX-itxNext)*unitX, 0, (maxZ-minZ)*unitZ, Fm3D);
          if (transformed != null){
            g.drawString(itxNextStrShort, transformed.x-15, transformed.y+15);
          }
          i++;
        }
        Iterator itz = stepsZ.iterator();
        int j = 0;
//        System.out.println("im paint 8");
        while  (itz.hasNext() ){
          //x = 0
          //z vary *unitZ
          //y = 0
          double itzNext =  ((Double)(itz.next())).doubleValue();
          String itzNextStr = Double.toString(itzNext);
          String itzNextStrShort =  itzNextStr.substring(0, Math.min(itzNextStr.length(),itzNextStr.indexOf(".")+3));
          Point transformed = this.transformPoint((maxX-minX)*unitX,0,(maxZ-itzNext)*unitZ, Fm3D);
          if (transformed != null){
            g.drawString(itzNextStr, transformed.x+5, transformed.y+10);
          }
          j++;
        }
        //maximal z coordinate
        String maxZstr = Double.toString(maxZ);
        String maxZShort =  maxZstr.substring(0, Math.min(maxZstr.length(),maxZstr.indexOf(".")+3));
        Point transformedZ = this.transformPoint((maxX-minX)*unitX,0,(maxZ-maxZ)*unitZ, Fm3D);
        if (transformedZ != null){
          g.drawString(maxZShort, transformedZ.x+5, transformedZ.y+10);
        }

        Iterator itheight = stepsHeight.iterator();
        int k = 0;

        while  (itheight.hasNext() ){
          //x = 0
          //z = 0
          //y vary * unitY
          double itheightNext =  ((Double)(itheight.next())).doubleValue();
          String itheightNextStr = Double.toString(itheightNext);
          String itheightNextStrShort =  itheightNextStr.substring(0, Math.min(itheightNextStr.length(),
              itheightNextStr.indexOf(".")+3));
          Point transformed = this.transformPoint(0,(itheightNext)*unitY, (maxZ-minZ)*unitZ, Fm3D);
          if (transformed != null){
            g.drawString(itheightNextStrShort, transformed.x-28, transformed.y+5);
          }
          k++;
        }
        //maximal height
        String maxHeight = Double.toString(maxY);
        String maxHeightShort =  maxHeight.substring(0, Math.min(maxHeight.length(),maxHeight.indexOf(".")+3));
        Point transformed = this.transformPoint(0,(maxY)*unitY, (maxZ-minZ)*unitZ, Fm3D);
        if (transformed != null){
          g.drawString(maxHeightShort, transformed.x-28, transformed.y+5);
        }
      }
//      System.out.println("im paint end");
    }

    /**
        * Transforms a 3D-point in 2D-coordinates
        * @param x
        * @param y
        * @param z
        * @param Fm3D
        * @return A 2D point.
    */
        private Point transformPoint(double x, double y, double z, FM3DGraphic Fm3D){
//      System.out.println("im transformPoint 1");

      Point3D toTransform = new Point3D(x, y, z,255,255,255,id);

      Figure2D transformedFigure = toTransform.project(Fm3D);
      if(transformedFigure == null){
        return null;
      }
      if (transformedFigure instanceof IDPoint2D){
        IDPoint2D transformedPoint = (IDPoint2D) transformedFigure;
        return new Point((int)(transformedPoint.getX()), (int)(transformedPoint.getY()));
      }
      else{
//        System.out.println("im transformPoint exit weil null");
        return null;
      }
        }

    /**
        *
        * @param targetX
        * @param targetY
        * @param targetZ
    */
        public void setCameraTarget(double targetX, double targetY, double targetZ){
      this.cameraTargetX = targetX;
      this.cameraTargetY = targetY;
      this.cameraTargetZ = targetZ;
        }

        public void setCameraDistance(double cameraDistance){
          this.cameraDistance = cameraDistance;
        }

    /**
        * sets the color of a histogram2d
        *
        * @param mainColor
    */
        public void setMainColor(Color mainColor){
      this.mainColor = mainColor;
        }

    /**
        * computes the heights of the columns given by binAreas divided by the ranges
        * @param rangesX
        * @param rangesZ
        * @param binAreas
        * @return Array of heights to paint on y-axis.
    */
        private double[] getBinHeights(double[] rangesX, double[] rangesZ, double[] binAreas)
    {
//      System.out.println("in getBinHeights");
      double[] out = new double[binAreas.length];

      int binsCount = 0;
      for (int j = 0; j < rangesZ.length-1; j++){

        for (int i = 0; i < rangesX.length-1; i++){
//          System.out.println("rangesX["+i+"]: "+rangesX[i]);
//          System.out.println("binsCount: "+binsCount);
          out[binsCount] = binAreas[binsCount]/((rangesX[i+1] - rangesX[i]) * (rangesZ[j+1] - rangesZ[j]));
          binsCount++;
        }
      }
//      System.out.println("im getBinHeights 1");
      return out;
    }

    /**
        *
        * @param rangesXVecPaint
        * @param rangesZVecPaint
        * @param binsVecPaint
    */
        public void setHistogram(double[] rangesXVecPaint,double[] rangesZVecPaint, double[] binsVecPaint){
      //System.out.println("im setHistogram 1");
      this.removeAll();
      this.rangesXVecPaint = rangesXVecPaint;
      this.rangesZVecPaint = rangesZVecPaint;
      this.binsVecPaint = binsVecPaint;
      if (this.rangesXVecPaint == null || this.rangesZVecPaint == null || this.binsVecPaint == null){
        this.binsHeightPaint = null;
        this.removeID(id);
        return;
      }
      else{
        this.binsHeightPaint = this.getBinHeights(rangesXVecPaint, rangesZVecPaint, binsVecPaint);
//        System.out.println("im setHistogram 2");
      }
      // local Data array ready
      // set data 3D

      //***********************************************************

      double minX = rangesXVecPaint[0];
      double maxX = rangesXVecPaint[rangesXVecPaint.length-1];
      double minZ = rangesZVecPaint[0];
      double maxZ = rangesZVecPaint[rangesZVecPaint.length-1];
      double maxY = this.getMax(this.binsHeightPaint);
      double minY = this.getMin(this.binsHeightPaint);

      double minXsmaller0;
      double minYsmaller0;
      double minZsmaller0;

      if (minY == 0.0 && maxY == 0.0){
        maxY = 10.0;
      }

      if(minX > 0.0){
        minXsmaller0 = 0.0;
      }
      else {
        minXsmaller0 = minX;
      }
      if(minY > 0.0){
        minYsmaller0 = 0.0;
      }
      else {
        minYsmaller0 = minY;
      }
      if(minX > 0.0){
        minZsmaller0 = 0.0;
      }
      else {
        minZsmaller0 = minZ;
      }

      double unitX = 100/(Math.abs(maxX)+ Math.abs(minXsmaller0));
      double unitY = 100/(Math.abs(maxY)+ Math.abs(minYsmaller0));
      double unitZ = 100/(Math.abs(maxZ)+ Math.abs(minZsmaller0));

//      System.out.println("minX: "+minX);
//      System.out.println("maxX: "+maxX);
//      System.out.println("minZ: "+minZ);
//      System.out.println("maxZ: "+maxZ);
//      System.out.println("minY: "+minY);
//      System.out.println("maxY: "+maxY);
//      System.out.println("unitX: "+unitX);
//      System.out.println("unitZ: "+unitZ);
//      System.out.println("unitY: "+unitY);
//      System.out.println("unitX: "+unitX);

      this.paintCoordinateSystemXYZ(minX, maxX, minZ, maxZ, minY, maxY, unitX, unitZ, unitY);
      this.paintHistogram2d(this.rangesXVecPaint, this.rangesZVecPaint, this.binsHeightPaint, unitX, unitZ, unitY,
                            103, 239, 109);
//      System.out.println("im setHistogram end");
      this.update();
        }

    /**
        *
        * @param values
        * @return Maximum of an array of doubles.
    */
        private double getMax(double[] values){
//      System.out.println("im getMax 1");
      double out = values[0];
      if (values.length == 1){
        return out;
      }
      for (int i = 1; i < values.length; i++){
        if (values[i] > out){
          out = values[i];
        }
      }
//      System.out.println("im getMax end");
      return out;
        }

    /**
        *
        * @param values
        * @return Minimum of an array of doubles.
    */
        private double getMin(double[] values){
//      System.out.println("im getMin 1");
      double out = values[0];
      if (values.length == 1){
        return out;
      }
      for (int i = 1; i < values.length; i++){
        if (values[i] < out){
          out = values[i];
        }
      }
//      System.out.println("im getMin end");
      return out;
        }


    /** split the given rectangle into two triangles and inserts them into tv.
    **/
        private void insertRectangleSimple(Triangle3DVector tv,
                                           Point3DSimple p1, Point3DSimple p2, Point3DSimple p3, Point3DSimple p4,
                                           boolean l1_2, boolean l2_3, boolean l3_4, boolean  l4_1,
                                           Color borderColor, ID aID){

                                             Triangle3D t1 = new Triangle3D(p1,p2,p3, l1_2, false,l2_3, borderColor,aID);
                                             Triangle3D t2 = new Triangle3D(p1,p3,p4, false,l4_1,l3_4,borderColor,aID);
                                             tv.append(t1);
                                             tv.append(t2);
                                           }


    /** adds a rectangle to the trianglevector. The rectangle is splitted
        * into nearly squares. So, the painting algorithm works better.
    **/
        private void insertRectangle(Triangle3DVector tv,
                                     Point3DSimple p1, Point3DSimple p2, Point3DSimple p3, Point3DSimple p4,
                                     Color color,
                                     Color borderColor, ID aID){
                                       p1.setColor(color);
                                       p2.setColor(color);
                                       p3.setColor(color);
                                       p4.setColor(color);
                                       double len1_2 = p1.distance(p2);
                                       double len1_4 = p1.distance(p4);

                                       if(len1_2==0 || len1_4==0){
                                         insertRectangleSimple(tv,p1,p2,p3,p4, true,true,true,true,borderColor,aID);
                                       } else {
                                         double ratio = len1_2 > len1_4 ? len1_2/len1_4 : len1_4/len1_2;
                                         if((int)ratio < 2){ // split not needed ???
                                           insertRectangleSimple(tv,p1,p2,p3,p4, true,true,true,true,borderColor,aID);
                                         } else {
                                           double x1 = p1.getX();
                                           double y1 = p1.getY();
                                           double z1 = p1.getZ();

                                           double x2 = p2.getX();
                                           double y2 = p2.getY();
                                           double z2 = p2.getZ();

                                           double x3 = p3.getX();
                                           double y3 = p3.getY();
                                           double z3 = p3.getZ();

                                           double x4 = p4.getX();
                                           double y4 = p4.getY();
                                           double z4 = p4.getZ();

                                           if(len1_2>len1_4){
                                             int steps = (int) (len1_2 / len1_4);
                                             double dx1 = (x2-x1)/ steps;
                                             double dx2 = (x3-x4)/steps;
                                             double dy1 = (y2-y1)/steps;
                                             double dy2 = (y3-y4)/steps;
                                             double dz1 = (z2-z1)/steps;
                                             double dz2 = (z3-z4)/steps;

                                             Point3DSimple P1 = p1.duplicate();
                                             Point3DSimple P4 = p4.duplicate();


                                             for(int i=1;i<=steps;i++){
                                               Point3DSimple P2 = new Point3DSimple(x1 + i*dx1, y1+i*dy1, z1+i*dz1,color);
                                               Point3DSimple P3 = new Point3DSimple(x4 + i*dx2, y4+i*dy2, z4+i*dz2,color);
                                               if(i==1){ // first rectangle
                                                 insertRectangleSimple(tv,P1,P2,P3,P4,true,false,true,true,borderColor,aID);
                                               } else if(i==steps){ // last triangle
                                                 insertRectangleSimple(tv,P1,P2,P3,P4,true,true,true,false,borderColor,aID);
                                               } else {
                                                 insertRectangleSimple(tv,P1,P2,P3,P4,true,false,true,false,borderColor,aID);
                                               }
                                               P1 = P2;
                                               P4 = P3;
                                             }

                                           } else {
                                             int steps = (int) (len1_4 / len1_2);
                                             double dx1 = (x3-x2)/ steps;
                                             double dx2 = (x4-x1)/steps;
                                             double dy1 = (y3-y2)/steps;
                                             double dy2 = (y4-y1)/steps;
                                             double dz1 = (z3-z2)/steps;
                                             double dz2 = (z4-z1)/steps;
                                             Point3DSimple P1 = p2.duplicate();
                                             Point3DSimple P4 = p1.duplicate();
                                             for(int i=1;i<=steps;i++){
                                               Point3DSimple P2 = new Point3DSimple( x2+i*dx1, y2+i*dy1, z2+i*dz1, color);
                                               Point3DSimple P3 = new Point3DSimple( x1+i*dx2, y1+i*dy2 , z1+i*dz2, color);
                                               if(i==1){ // first rectangle
                                                 insertRectangleSimple(tv,P1,P2,P3,P4,true,false,true,true,borderColor,aID);
                                               } else if(i==steps){ // last triangle
                                                 insertRectangleSimple(tv,P1,P2,P3,P4,true,true,true,false,borderColor,aID);
                                               } else {
                                                 insertRectangleSimple(tv,P1,P2,P3,P4,true,false,true,false,borderColor,aID);
                                               }
                                               P1 = P2;
                                               P4 = P3;
                                             }
                                           }

                                         }
                                       }

                                     }



      /**
          * adds the 3D geometry for one column of a histogram2d
          *
          * @param height
          * @param rangeSmallX
          * @param rangeBigX
          * @param rangeSmallZ
          * @param rangeBigZ
          * @param cRed
          * @param cGreen
          * @param cBlue
      */
          private void paintColumnColor(double height, double rangeSmallX, double rangeBigX, double rangeSmallZ, double rangeBigZ,
                                        int cRed, int cGreen, int cBlue){

//        System.out.println("im paintColumnColor 1");
//           Point3DVector pointVecC = new Point3DVector();
//           Line3DVector lineVecC = new Line3DVector();

                                          if (height == 0.0){
                                            return;
                                          }


                                          if(cRed <100){
                                            cRed= 100;
                                          }
                                          if(cGreen <100){
                                            cGreen= 100;
                                          }
                                          if(cBlue <100){
                                            cBlue= 100;
                                          }
                                          if(cRed >205){
                                            cRed= 205;
                                          }
                                          if(cGreen >205){
                                            cGreen= 205;
                                          }
                                          if(cBlue >205){
                                            cBlue= 205;
                                          }


                                          Color color = new Color(cRed,cGreen,cBlue);
                                          Color borderColor = Color.BLACK;

           //triangles
                                          Point3DSimple pBackRightDownForTria = new Point3DSimple(rangeSmallX, 0, rangeBigZ,cRed,cGreen,cBlue);
                                          Point3DSimple pBackLeftDownForTria = new Point3DSimple(rangeBigX, 0,rangeBigZ,cRed, cGreen, cBlue);

                                          Point3DSimple pFrontRightDownForTria = new Point3DSimple(rangeSmallX, 0, rangeSmallZ,cRed, cGreen, cBlue);
                                          Point3DSimple pFrontLeftDownForTria = new Point3DSimple(rangeBigX, 0, rangeSmallZ,cRed, cGreen, cBlue);


                                          Point3DSimple pBackRightUpForTria = new Point3DSimple(rangeSmallX, height, rangeBigZ,cRed, cGreen, cBlue);
                                          Point3DSimple pBackLeftUpForTria = new Point3DSimple(rangeBigX, height, rangeBigZ,cRed, cGreen, cBlue);


                                          Point3DSimple pFrontRightUpForTria = new Point3DSimple(rangeSmallX, height, rangeSmallZ,cRed, cGreen, cBlue);
                                          Point3DSimple pFrontLeftUpForTria = new Point3DSimple(rangeBigX, height, rangeSmallZ,cRed, cGreen, cBlue);


                                          Triangle3DVector triaVecC = new Triangle3DVector();

           // front
                                          insertRectangle( triaVecC, pFrontLeftDownForTria, pFrontLeftUpForTria,
                                              pFrontRightUpForTria, pFrontRightDownForTria, color, borderColor,id);
           // left side
                                          insertRectangle( triaVecC, pBackLeftDownForTria, pBackLeftUpForTria,
                                              pFrontLeftUpForTria, pFrontLeftDownForTria, color,borderColor, id);
           // back
                                          insertRectangle( triaVecC, pBackLeftDownForTria, pBackLeftUpForTria,
                                              pBackRightUpForTria, pBackRightDownForTria, color,borderColor, id);
           // right side
                                          insertRectangle( triaVecC, pFrontRightDownForTria, pFrontRightUpForTria,
                                              pBackRightUpForTria, pBackRightDownForTria, color,borderColor,id);
           // bottom
                                          insertRectangle( triaVecC, pFrontLeftDownForTria, pBackLeftDownForTria,
                                              pBackRightDownForTria, pFrontRightDownForTria, color,borderColor,id);
           // top
                                          insertRectangle( triaVecC, pFrontLeftUpForTria, pBackLeftUpForTria,
                                              pBackRightUpForTria, pFrontRightUpForTria, color,borderColor,id);

                                          this.add(triaVecC);
                                        }



      /**
          * sets data of the histogram2d to paint
          * @param _rangesX
          * @param _rangesZ
          * @param _bins
          * @param unitX
          * @param unitZ
          * @param unitY
          * @param cRed
          * @param cGreen
          * @param cBlue
      */
          private void paintHistogram2d(double[] _rangesX, double[] _rangesZ, double[] _bins,double unitX,
                                        double unitZ, double unitY, int cRed, int cGreen, int cBlue){
                                          int binsCount = 0;
                                          double paintX = 0.0;
                                          double paintZ = 0.0;
                                          double paintXplus = 0.0;
                                          double paintZplus = 0.0;


                                          for (int j = 0; j < _rangesZ.length-1; j++){
//          System.out.println("rangesZ["+j+"]: "+_rangesZ[j]);
                                            for (int i = 0; i < _rangesX.length-1; i++){
//            System.out.println("rangesX["+i+"]: "+_rangesX[i]);
//            System.out.println("binsCount: "+binsCount);
                                              paintX = (_rangesX[_rangesX.length-1] - _rangesX[i])*unitX;
                                              paintXplus = (_rangesX[_rangesX.length-1] - _rangesX[i+1])*unitX;
                                              paintZ = (_rangesZ[_rangesZ.length-1] - _rangesZ[j])*unitZ;
                                              paintZplus = (_rangesZ[_rangesZ.length-1] - _rangesZ[j+1])*unitZ;

//            System.out.println("binsCount: "+binsCount);
//            System.out.println("selectedBin: "+this.selectedBin);
                                              //
//            if (binsCount == this.selectedBin){
//              paintColumnColor(_bins[binsCount]*unitY,
//                  paintX,
//                  paintXplus,
//                  paintZ,
//                  paintZplus,
//                  255, 0, 0 );
//            }
//            else{

//            System.out.println("*********************");
//            System.out.println("bins["+binsCount+"]"  +_bins[binsCount]*unitX);
//            System.out.println("rangesX["+i+"]: "+_rangesX[i]);
//            System.out.println("rangesZ["+j+"]: "+_rangesZ[j]);

                                              paintColumnColor( _bins[binsCount]*unitY,
                                                  paintX,
                                                  paintXplus,
                                                  paintZ,
                                                  paintZplus,
                                                  cRed, cGreen, cBlue );
//            }
                                              binsCount++;
                                            }
                                          }
                                        }




      /**
          * paints the coordinate system for the histogram2d
          * @param minX
          * @param maxX
          * @param minZ
          * @param maxZ
          * @param minY
          * @param maxY
          * @param unitX
          * @param unitZ
          * @param unitY
      */
          public void paintCoordinateSystemXYZ(double minX, double maxX, double minZ, double maxZ, double minY, double maxY, double unitX, double unitZ,
                                               double unitY){
//        System.out.println("im paintCoordingatesystemXYZ 1");

        // Point3DVector pointVec = new Point3DVector();
                                                 Line3DVector   lineVec = new Line3DVector();
        //Triangle3DVector triaVec = new Triangle3DVector();

                                                 if (minX > 0.0){
                                                   minX = 0.0;
                                                 }
                                                 if (minY > 0.0){
                                                   minY = 0.0;
                                                 }
                                                 if (minZ > 0.0){
                                                   minZ = 0.0;
                                                 }

//        System.out.println("im paintCoordingatesystemXYZ 2");
                                                 Point3D pNull = new Point3D(0, 0.0,0,0,0,0, id);
                                                 Point3D pXpositiv = new Point3D(maxX*unitX,0, 0, 0, 0, 0, id);
                                                 Point3D pXnegativ = new Point3D((maxX-minX)*unitX,0, 0, 0, 0, 0, id);
        //y-axis for binheights
                                                 Point3D pYpositiv = new Point3D(0,maxY*unitY, 0, 0, 0, 0,id);
                                                 Point3D pYnegativ = new Point3D(0,minY*unitY, 0, 0, 0, 0,id);
                                                 Point3D pZpositiv = new Point3D(0,0, maxZ*unitZ, 0, 0, 0,id);
                                                 Point3D pZnegativ = new Point3D(0,0, (maxZ-minZ)*unitZ, 0, 0, 0,id);

                                                 Point3D pFront = new Point3D((maxX-minX)*unitX, 0, (maxZ-minZ)*unitZ, 0,0,0);
                                                 Point3D pZYUp = new Point3D(0, maxY*unitY, (maxZ-minZ)*unitZ, 0,0,0);
                                                 Point3D pXYUp = new Point3D((maxX-minX)*unitX, maxY*unitY, 0, 0,0,0);
                                                 Point3D pZYDown = new Point3D(0, minY*unitY, (maxZ-minZ)*unitZ, 0,0,0);
                                                 Point3D pXYDown = new Point3D((maxX-minX)*unitX, minY*unitY, 0, 0,0,0);

//        System.out.println("im paintCoordingatesystemXYZ 3");

        /*pointVec.append(pNull);
                                                 pointVec.append(pXpositiv);
                                                 pointVec.append(pXnegativ);
                                                 pointVec.append(pYpositiv);
                                                 pointVec.append(pYnegativ);
                                                 pointVec.append(pZpositiv);
                                                 pointVec.append(pZnegativ);
                                                 pointVec.append(pFront);
                                                 pointVec.append(pZYDown);
                                                 pointVec.append(pXYDown);
        */

                                                 Line3D Xpositiv = new Line3D(pNull, pXpositiv);
                                                 Line3D Xnegativ = new Line3D(pXpositiv, pXnegativ);
                                                 Line3D Ypositiv = new Line3D(pNull, pYpositiv);
                                                 Line3D Ynegativ = new Line3D(pNull, pYnegativ);
                                                 Line3D Zpositiv = new Line3D(pNull, pZpositiv);
                                                 Line3D Znegativ = new Line3D(pZpositiv, pZnegativ);
                                                 Line3D xToFront = new Line3D(pFront, pXnegativ);
                                                 Line3D zToFront = new Line3D(pFront, pZpositiv);
                                                 Line3D zmaxTomaxYZ = new Line3D(pZnegativ, pZYUp);
                                                 Line3D xmaxTomaxXY = new Line3D(pXnegativ, pXYUp);
                                                 Line3D upLineXY = new Line3D(pYpositiv, pZYUp);
                                                 Line3D upLineYZ = new Line3D(pYpositiv, pXYUp);
                                                 Line3D bottomLineZ = new Line3D(pZYDown, pYnegativ);
                                                 Line3D bottomLineX = new Line3D(pXYDown, pYnegativ);
                                                 Line3D XnegativeToYnegative = new Line3D(pXnegativ, pXYDown);

                                                 lineVec.append(bottomLineZ);
                                                 lineVec.append(bottomLineX);
                                                 lineVec.append(XnegativeToYnegative);


                                                 lineVec.append(Xpositiv);
                                                 lineVec.append(Xnegativ);
                                                 lineVec.append(Ypositiv);
                                                 lineVec.append(Ynegativ);
                                                 lineVec.append(Zpositiv);
                                                 lineVec.append(Znegativ);
                                                 lineVec.append(xToFront);
                                                 lineVec.append(zToFront);
                                                 lineVec.append(zmaxTomaxYZ);
                                                 lineVec.append(xmaxTomaxXY);
                                                 lineVec.append(upLineXY);
                                                 lineVec.append(upLineYZ);


        //to paint the steps of the coordinatesystem XY columns and XZ columns
                                                 Vector rangeXIntervals = this.paintSteps(maxX, minX);
                                                 Iterator itx = rangeXIntervals.iterator();
                                                 Point3D[] stepsX = new Point3D[12];
                                                 Point3D[] stepsX_Ymax = new Point3D[12]; //above
                                                 Point3D[] stepsX_Ymin = new Point3D[12]; //above
                                                 Point3D[] stepsX_Zmax = new Point3D[12]; // bottom
                                                 Point3D[] stepsX_Zmin = new Point3D[12]; // bottom
                                                 Line3D[] lineXYColumn = new Line3D[12];
                                                 Line3D[] lineXYColumnMin = new Line3D[12];
                                                 Line3D[] lineXZColumn = new Line3D[12];
                                                 Line3D[] lineXZColumnMin = new Line3D[12];
                                                 int i = 0;


                                                 while  (itx.hasNext() ){

                                                   double itxNext =  ((Double)(itx.next())).doubleValue();
//          System.out.println("itxNext: "+itxNext);
//          System.out.println("i: "+i);
//          System.out.println("itxNext*unitX: "+itxNext*unitX);
                                                   stepsX[i]= new Point3D((maxX-itxNext)*unitX, 0,0, 0,0,0);
                                                   stepsX_Ymax[i]= new Point3D((maxX-itxNext)*unitX, maxY*unitY,0, 0,0,0);
                                                   stepsX_Ymin[i]= new Point3D((maxX-itxNext)*unitX, minY*unitY,0, 0,0,0);
                                                   lineXYColumn[i] = new Line3D(stepsX[i], stepsX_Ymax[i]);
                                                   lineXYColumnMin[i] = new Line3D(stepsX[i], stepsX_Ymin[i]);
                                                   lineVec.append(lineXYColumn[i]);
                                                   lineVec.append(lineXYColumnMin[i]);
                                                   stepsX_Zmax[i] = new Point3D((maxX-itxNext)*unitX, 0,maxZ*unitZ, 0,0,0);
                                                   stepsX_Zmin[i] = new Point3D((maxX-itxNext)*unitX, 0,(maxZ-minZ)*unitZ, 0,0,0);
                                                   lineXZColumn[i] = new Line3D(stepsX_Zmax[i], stepsX[i]);
                                                   lineXZColumnMin[i] = new Line3D(stepsX_Zmin[i], stepsX[i]);
                                                   lineVec.append(lineXZColumn[i]);
                                                   lineVec.append(lineXZColumnMin[i]);

                                                   i++;
                                                 }

        //to paint the steps of the coordinatesystem YZ columns and ZX rows
                                                 Vector rangeZIntervals = this.paintSteps(maxZ, minZ);
                                                 Iterator itz = rangeZIntervals.iterator();
                                                 Point3D[] stepsZ = new Point3D[12];
                                                 Point3D[] stepsZ_Ymax = new Point3D[12]; //above
                                                 Point3D[] stepsZ_Ymin = new Point3D[12]; //above
                                                 Point3D[] stepsZ_Xmax = new Point3D[12]; // bottom, row
                                                 Point3D[] stepsZ_Xmin = new Point3D[12]; // bottom, row
                                                 Line3D[] lineYZColumn = new Line3D[12];
                                                 Line3D[] lineYZColumnMin = new Line3D[12];
                                                 Line3D[] lineZXRow = new Line3D[12];
                                                 Line3D[] lineZXRowMin = new Line3D[12];
                                                 int j = 0;


                                                 while  (itz.hasNext() ){
                                                   double itzNext =  ((Double)(itz.next())).doubleValue();
//          System.out.println("itzNext: "+itzNext);
                                                   stepsZ[j]= new Point3D(0, 0,(maxZ-itzNext)*unitZ, 0,0,0);
                                                   stepsZ_Ymax[j]= new Point3D(0, maxY*unitY,(maxZ-itzNext)*unitZ, 0,0,0);
                                                   stepsZ_Ymin[j]= new Point3D(0, minY*unitY,(maxZ-itzNext)*unitZ, 0,0,0);
                                                   lineYZColumn[j] = new Line3D(stepsZ[j], stepsZ_Ymax[j]);
                                                   lineYZColumnMin[j] = new Line3D(stepsZ[j], stepsZ_Ymin[j]);
                                                   lineVec.append(lineYZColumn[j]);
                                                   lineVec.append(lineYZColumnMin[j]);
                                                   stepsZ_Xmax[j] = new Point3D(maxX*unitX, 0,(maxZ-itzNext)*unitZ, 0,0,0);
                                                   stepsZ_Xmin[j] = new Point3D((maxX-minX)*unitX, 0,(maxZ-itzNext)*unitZ, 0,0,0);
                                                   lineZXRow[j] = new Line3D(stepsZ_Xmax[j], stepsZ[j]);
                                                   lineZXRowMin[j] = new Line3D(stepsZ_Xmin[j], stepsZ[j]);
                                                   lineVec.append(lineZXRow[j]);
                                                   lineVec.append(lineZXRowMin[j]);
                                                   j++;
                                                 }

        //to paint the steps of the coordinatesystem YZ rows and YX rows
                                                 Vector rangeYIntervals = this.paintSteps(maxY, minY);
                                                 Iterator ity = rangeYIntervals.iterator();
                                                 Point3D[] stepsY = new Point3D[12];
                                                 Point3D[] stepsY_Xmax = new Point3D[12]; //above
                                                 Point3D[] stepsY_Xmin = new Point3D[12]; //above
                                                 Point3D[] stepsY_Zmax = new Point3D[12]; // bottom, row
                                                 Point3D[] stepsY_Zmin = new Point3D[12]; // bottom, row
                                                 Line3D[] lineYZRow = new Line3D[12];
                                                 Line3D[] lineYZRowMin = new Line3D[12];
                                                 Line3D[] lineYXRow = new Line3D[12];
                                                 Line3D[] lineYXRowMin = new Line3D[12];
                                                 int k = 0;


                                                 while  (ity.hasNext() ){
                                                   double ityNext =  ((Double)(ity.next())).doubleValue();
                                                   stepsY[k]= new Point3D(0, ityNext*unitY,0, 0,0,0);
                                                   stepsY_Xmax[k]= new Point3D(maxX*unitX, ityNext*unitY,0, 0,0,0);
                                                   stepsY_Xmin[k]= new Point3D((maxX-minX)*unitX, ityNext*unitY,0, 0,0,0);
                                                   lineYXRow[k] = new Line3D(stepsY[k], stepsY_Xmax[k]);
                                                   lineYXRowMin[k] = new Line3D(stepsY[k], stepsY_Xmin[k]);
                                                   lineVec.append(lineYXRow[k]);
                                                   lineVec.append(lineYXRowMin[k]);
                                                   stepsY_Zmax[k] = new Point3D(0, ityNext*unitY,maxZ*unitZ, 0,0,0);
                                                   stepsY_Zmin[k] = new Point3D(0, ityNext*unitY,(maxZ-minZ)*unitZ, 0,0,0);
                                                   lineYZRow[k] = new Line3D(stepsY_Zmax[k], stepsY[k]);
                                                   lineYZRowMin[k] = new Line3D(stepsY_Zmin[k], stepsY[k]);
                                                   lineVec.append(lineYZRow[k]);
                                                   lineVec.append(lineYZRowMin[k]);

                                                   k++;
                                                 }

//        System.out.println("im paintCoordingatesystemXYZ end");

  //      this.add(pointVec); //to test
                                                 this.add(lineVec);
  //      this.add(triaVec);
                                               }



      /**
          *
          * @return vector of steps to be painted
      */
          private Vector paintSteps(double max, double min){
        Vector numbers = new Vector(20);
        double distance = 0.0;
        if (min < 0.0)
        {
          distance = max-min;
        }
        else
        {
          distance = max;
        }
        double step = this.getDisplayStep(distance, 5);

        if(max<=0.0){
          max = 0.0;
        }
        int countPositiv = (int)(max/step); //numbers of steps in positiv direction
        if (min>=0.0){
          min=0.0;
        }
        int countNegativ = (int)(Math.abs(min)/step);

        for (int i = 0; i <= countPositiv; i++){
          numbers.add(Double.valueOf(i*step));
        }
        for (int i = 1; i <= countNegativ; i++){
          numbers.add(Double.valueOf(-i*step));
        }
        return numbers;
          }

      /**
          *
          * @param interval
          * @param minSteps
          * @return step distance to paint
      */
          private static double getDisplayStep(double interval, double minSteps){
        interval = Math.abs(interval);
        if (interval == 0.0){
          interval = 10;
        }
        double step;
        double stepMin = interval/minSteps;


        double magnitude = 1.0;
        while(stepMin > 10){
          stepMin/=10;
          magnitude*=10;
        }
        //stepMin is between 0 and 100
        while(stepMin < 1){
          stepMin*=10;
          magnitude/=10;
        }
        //stepMin between 1 and 10
        if(stepMin < 2.0){
          step = 2*magnitude;
        }
        else{
          step = 5*magnitude;
        }
        return step;
          }
      }

      public String toString() {
        return "Histogram2d";
      }
}
