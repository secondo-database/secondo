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

package viewer;

import com.sun.j3d.utils.image.TextureLoader;
import com.sun.j3d.utils.picking.behaviors.*;
import com.sun.j3d.utils.picking.PickCanvas;
import com.sun.j3d.utils.picking.PickResult;
import com.sun.j3d.utils.universe.SimpleUniverse;
import gui.*;
import gui.idmanager.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.BufferedImage;
import java.sql.Timestamp;
import java.util.*;
import javax.media.j3d.*;
import javax.swing.*;
import javax.swing.border.LineBorder;
import javax.swing.event.*;
import javax.vecmath.Color3f;
import javax.vecmath.Point3d;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3d;
import project.OSMMercator;
import sj.lang.ListExpr;
import viewer.spacetimecube.*;


/**
 * Class representing the Secondo SpaceTimeCube viewer.
 * It enables to view the data object moving point
 * in a 3-dimensional way.
 * @author Franz Fahrmeier
 *
 */
public class SpaceTimeCubeViewer extends SecondoViewer implements 
			ActionListener, ChangeListener, MouseListener, ItemListener {
	
	private MenuVector MenuExtension;
	private JMenuItem miShowSettings;
	private JMenu STCMenu;
	private Vector<SecondoObject> objectList; // stores all queried SecondoObjects
	private SpaceTimeCube STC;
	private Canvas3D canvas;
	private PickCanvas pickCanvas;
	private SimpleUniverse universe;
	private Transform3D transform, transformMap;
	private TransformGroup tg, tgMap; // tgMap only includes the map and is a part of tg
	private Vector<Shape3D> mPoints; // stores all currently maintained moving points as java3d Shape3D objects
	private BranchGroup lastBg;
	private BufferedImage map;
	private Shape3D mapShape, topShape; // topShape is the top shape of the cube
	private Shape3D lineXaxis, lineYaxis, lineZaxis, lineBottomBack, lineBottomRight,
				lineLeftBack, lineRightFront, lineRightBack; // lines to construct the cube
	private Point3d bottomFrontLeft, bottomFrontRight, bottomBackLeft, bottomBackRight,
				topFrontLeft, topFrontRight, topBackLeft, topBackRight; // points helping to construct the cube
	private JSlider zMinSlide, zMaxSlide, mapSlide; // sliders for timeframe filter and map movement
	private JSpinner rotXspinner, rotZspinner; // spinners for rotation about X and Z axis
	private JLabel zMinLabel, zMaxLabel;
	private JLabel labelDateMap, labelTimeMap;
			// labels for timestamp connected to map slider
	private JButton butSubmit, butRetry, butReset, butBack;
	private JFrame frameLoad, frameMPoint;	// frameLoad = dialog when loading of the map takes time
								// frameMPoint = frame showing additional information when right-clicking a mpoint  
	private View2DSTC panelMapOverview; // panel on the top right showing the 2D map
	private OSMMercator osmm;
	private int rotXlastVal, rotZlastVal;
			// last rotation value needs to be stored so that only deltas can be rotated
	private double minXlimitLast, maxXlimitLast, minYlimitLast, maxYlimitLast; // last filter values
	private long minZlimitLast, maxZlimitLast;
	private SettingsDialog frameSettings; // settings dialog to be opened from Secondo menu
	private Vector<LastLimits> lastLimits;
			// stores all filter values to make "back"-button work multiple times
	private JComboBox cbView;
	private Vector<double[]> views; // stores values for "lookat"-method
	//< Settings (from settings dialog)
	private Hashtable<ID, float[]> colorSO; // color information for each SecondoObject in the STC
	private boolean drawVertLines = false; // switch for drawing vertical lines
	private Color3f colorCanvas = new Color3f(0,0,0); // background color
	private float lineWidth = 1.0f; // weight of MPoint lines
	private float transpMPoints = 0.0f; // transparency value of the MPoints
	// >
	
	/**
	 * Constructor doing several attribute initializations
	 * for common attributes and setting initial layout.
	 */
	public SpaceTimeCubeViewer(){
		
		MenuExtension = new MenuVector();
		STCMenu = new JMenu("STC-Viewer");
		MenuExtension.addMenu(STCMenu);
	    miShowSettings = STCMenu.add("Settings");
	   
	    miShowSettings.addActionListener(this);
		
		objectList = new Vector<SecondoObject>();
		mPoints = new Vector<Shape3D>();
		STC = new SpaceTimeCube(this);
		osmm = new OSMMercator();
		lastLimits = new Vector<LastLimits>();
		colorSO = new Hashtable<ID, float[]>();
		
		views = new Vector<double[]>();
		double[] viewOverview = {0.5, -1.0, 0.65, 0, 0, 1}; // overview view
		double[] viewTop = {0, 0, 0.75, 0, 1, 0}; // top view
		double[] viewFront = {0, -0.75, 0, 0, 0, 1}; // front view
		// all defined views are added to the views vector
		views.add(viewOverview);
		views.add(viewTop);
		views.add(viewFront);
		
		// combobox holding all view possibilities
		cbView = new JComboBox();
		cbView.addItemListener(this);
		cbView.addItem("---------");
		cbView.addItem("Overview");
		cbView.addItem("Top");
		cbView.addItem("Front");
		
		// buttons in this viewer
		butSubmit = new JButton("OK");
		butBack = new JButton("Back");
		butReset = new JButton("Reset");
		butRetry = new JButton("Retry");
		
		// spinners in this viewer
		rotXspinner = new JSpinner();
		rotXspinner.setPreferredSize(new Dimension(45, rotXspinner.getPreferredSize().height));
		rotZspinner = new JSpinner();
		rotZspinner.setPreferredSize(new Dimension(45, rotZspinner.getPreferredSize().height));
		
		// sliders in this viewer
		zMinSlide = new JSlider();
		zMaxSlide = new JSlider();
		mapSlide = new JSlider();
		mapSlide.setOrientation(SwingConstants.VERTICAL);
		
		setLayout(new BorderLayout());
		
		//< Definition of settings panel on the right of the viewer
		JPanel panelSettings = new JPanel();
		GridBagLayout gblSettings = new GridBagLayout();
		GridBagConstraints gbcSettings = new GridBagConstraints();
		gbcSettings.fill=GridBagConstraints.HORIZONTAL;
		gbcSettings.insets = new Insets(2,2,2,2);
		panelSettings.setLayout(gblSettings);
		
		JSeparator sepHori1 = new JSeparator();
		JSeparator sepHori2 = new JSeparator();
		JSeparator sepVert = new JSeparator(SwingConstants.VERTICAL);
		
		zMinLabel = new JLabel("Timestamp");
		zMinLabel.setFont(new Font("Arial", Font.BOLD, 10));
		zMaxLabel = new JLabel("Timestamp");
		zMaxLabel.setFont(new Font("Arial", Font.BOLD, 10));
		JLabel labelAreaSelection = new JLabel("Area selection:");
		labelAreaSelection.setFont(new Font("Arial", Font.BOLD, 12));
		JLabel labelTimeframe = new JLabel("Timeframe:");
		labelTimeframe.setFont(new Font("Arial", Font.BOLD, 12));
		JLabel labelMap = new JLabel("Map:");
		labelMap.setFont(new Font("Arial", Font.BOLD, 12));
		JLabel labelVon = new JLabel("From:");
		labelVon.setFont(new Font("Arial", Font.BOLD, 10));
		JLabel labelBis = new JLabel("Till:");
		labelBis.setFont(new Font("Arial", Font.BOLD, 10));
		JLabel labelViewOp = new JLabel("View operations:");
		labelViewOp.setFont(new Font("Arial", Font.BOLD, 12));
		JLabel labelView = new JLabel("View:");
		labelView.setFont(new Font("Arial", Font.BOLD, 10));
		JLabel labelRotX = new JLabel("Rot. X:");
		labelRotX.setFont(new Font("Arial", Font.BOLD, 10));
		JLabel labelRotZ = new JLabel("Rot. Z:");
		labelRotZ.setFont(new Font("Arial", Font.BOLD, 10));	
		labelDateMap = new JLabel("Date");
		labelDateMap.setFont(new Font("Arial", Font.BOLD, 10));
		labelTimeMap = new JLabel("Time");
		labelTimeMap.setFont(new Font("Arial", Font.BOLD, 10));
		JLabel placeholder1 = new JLabel("");
		JLabel placeholder2 = new JLabel("");
		JLabel placeholder3 = new JLabel("");
		JPanel panelBut = new JPanel(new FlowLayout());
		panelBut.add(butSubmit);
		panelBut.add(butBack);
		panelBut.add(butReset);
		panelMapOverview = new View2DSTC(); // this is the 2D map on the top right
		panelMapOverview.setPreferredSize(new Dimension(250,250)); // size of the 2D map !
		panelMapOverview.setBorder(new LineBorder(Color.BLUE, 1));
		JPanel panelRotation = new JPanel(new FlowLayout(FlowLayout.LEFT)); // panel for rotation operations
		panelRotation.add(labelRotX);
		panelRotation.add(rotXspinner);
		panelRotation.add(labelRotZ);
		panelRotation.add(rotZspinner);
		
		gbcSettings.gridx=2;
		gbcSettings.gridy=0;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(labelAreaSelection, gbcSettings);
		panelSettings.add(labelAreaSelection);
		
		gbcSettings.gridx+=0;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(panelMapOverview, gbcSettings);
		panelSettings.add(panelMapOverview);
		
		gbcSettings.gridx+=0;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(sepHori1, gbcSettings);
		panelSettings.add(sepHori1);
		
		gbcSettings.gridx+=0;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(labelTimeframe, gbcSettings);
		panelSettings.add(labelTimeframe);
		
		gbcSettings.gridx+=0;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(labelVon, gbcSettings);
		panelSettings.add(labelVon);
		
		gbcSettings.gridx+=1;
		gbcSettings.gridy+=0;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gbcSettings.weightx=1;
		gblSettings.setConstraints(zMinSlide, gbcSettings);
		panelSettings.add(zMinSlide);
		gbcSettings.weightx=0;
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(placeholder1, gbcSettings);
		panelSettings.add(placeholder1);
		
		gbcSettings.gridx+=1;
		gbcSettings.gridy+=0;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gbcSettings.weightx=1;
		gblSettings.setConstraints(zMinLabel, gbcSettings);
		panelSettings.add(zMinLabel);
		gbcSettings.weightx=0;
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(labelBis, gbcSettings);
		panelSettings.add(labelBis);
		
		gbcSettings.gridx+=1;
		gbcSettings.gridy+=0;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gbcSettings.weightx=1;
		gblSettings.setConstraints(zMaxSlide, gbcSettings);
		panelSettings.add(zMaxSlide);
		gbcSettings.weightx=0;
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(placeholder2, gbcSettings);
		panelSettings.add(placeholder2);
		
		gbcSettings.gridx+=1;
		gbcSettings.gridy+=0;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gbcSettings.weightx=1;
		gblSettings.setConstraints(zMaxLabel, gbcSettings);
		panelSettings.add(zMaxLabel);
		gbcSettings.weightx=0;
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(panelBut, gbcSettings);
		panelSettings.add(panelBut);
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(sepHori2, gbcSettings);
		panelSettings.add(sepHori2);
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(labelViewOp, gbcSettings);
		panelSettings.add(labelViewOp);
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(labelView, gbcSettings);
		panelSettings.add(labelView);
		
		gbcSettings.gridx+=1;
		gbcSettings.gridy+=0;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(cbView, gbcSettings);
		panelSettings.add(cbView);
		
		gbcSettings.gridx=2;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=2;
		gblSettings.setConstraints(panelRotation, gbcSettings);
		panelSettings.add(panelRotation);
		
		gbcSettings.gridx=0;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=4;
		gbcSettings.weighty=1;
		gblSettings.setConstraints(placeholder3, gbcSettings);
		panelSettings.add(placeholder3);
		gbcSettings.weighty=0;

		Dimension d = sepVert.getPreferredSize();   
        d.height = panelSettings.getPreferredSize().height; 
        sepVert.setPreferredSize(d);
		
		gbcSettings.gridx=0;
		gbcSettings.gridy=0;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(labelMap, gbcSettings);
		panelSettings.add(labelMap);
		
        Dimension d2 = mapSlide.getPreferredSize();
        d2.height = 410;
        mapSlide.setPreferredSize(d2);

		gbcSettings.gridx+=0;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=10;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(mapSlide, gbcSettings);
		panelSettings.add(mapSlide);
		
		gbcSettings.gridx+=0;
		gbcSettings.gridy=11;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(labelDateMap, gbcSettings);
		panelSettings.add(labelDateMap);
		
		gbcSettings.gridx+=0;
		gbcSettings.gridy+=1;
		gbcSettings.gridheight=1;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(labelTimeMap, gbcSettings);
		panelSettings.add(labelTimeMap);
		
		gbcSettings.gridx+=1;
		gbcSettings.gridy=0;
		gbcSettings.gridheight=13;
		gbcSettings.gridwidth=1;
		gblSettings.setConstraints(sepVert, gbcSettings);
		panelSettings.add(sepVert);
		//>
        
		add(panelSettings, BorderLayout.EAST);
		
		transform = new Transform3D(); // base transform for the whole cube
		transformMap = new Transform3D();
		
		// setting the initial view (overview)
		Point3d eye = new Point3d(views.get(0)[0],views.get(0)[1],views.get(0)[2]);
		Point3d center = new Point3d(0.0,0.0,0.0);
		Vector3d up = new Vector3d(0,0,1);
		transform.lookAt(eye, center, up);
		
		canvas = new Canvas3D(SimpleUniverse.getPreferredConfiguration());
		add(canvas,BorderLayout.CENTER);
		 
	    universe = new SimpleUniverse(canvas);
		universe.getViewingPlatform().setNominalViewingTransform();
		
		// points helping to construct the cube
		bottomFrontLeft = new Point3d(-0.5+0,-0.5+0,-0.5+0);
		bottomFrontRight = new Point3d(-0.5+1,-0.5+0,-0.5+0);
		bottomBackLeft = new Point3d(-0.5+0,-0.5+1,-0.5+0);
		bottomBackRight = new Point3d(-0.5+1,-0.5+1,-0.5+0);
		topFrontLeft = new Point3d(-0.5+0,-0.5+0,-0.5+1);
		topFrontRight = new Point3d(-0.5+1,-0.5+0,-0.5+1);
		topBackLeft = new Point3d(-0.5+0,-0.5+1,-0.5+1);
		topBackRight = new Point3d(-0.5+1,-0.5+1,-0.5+1);
		
		zMinSlide.addChangeListener(this);
		zMaxSlide.addChangeListener(this);
		rotXspinner.addChangeListener(this);
		rotZspinner.addChangeListener(this);
		butSubmit.addActionListener(this);
		butReset.addActionListener(this);
		butBack.addActionListener(this);
		butBack.setEnabled(false);
		butRetry.addActionListener(this);
		
		// definition of dialog when loading of the map takes time (frameLoad)
		GridBagConstraints gbcLoad = new GridBagConstraints();
		GridBagLayout gblLoad = new GridBagLayout();
		gbcLoad.fill=GridBagConstraints.HORIZONTAL;
		gbcLoad.insets = new Insets(2,2,2,2);
		frameLoad = new JFrame();
		frameLoad.setLayout(gblLoad);
		frameLoad.setTitle("loading...");
		JLabel labelLoadingMap = new JLabel("Loading map...");
		JLabel labelInternetRequired = new JLabel("(Internet connection required)");
		gbcLoad.gridx = 0;
		gbcLoad.gridy = 0;
		gbcLoad.gridwidth = 2;
		gblLoad.setConstraints(labelLoadingMap, gbcLoad);
		frameLoad.add(labelLoadingMap);
		gbcLoad.gridx = 0;
		gbcLoad.gridy += 1;
		gbcLoad.gridwidth = 2;
		gblLoad.setConstraints(labelInternetRequired, gbcLoad);
		frameLoad.add(labelInternetRequired);
		gbcLoad.gridx = 0;
		gbcLoad.gridy += 1;
		gbcLoad.gridwidth = 1;
		gbcLoad.insets = new Insets(6,2,2,2);
		gblLoad.setConstraints(butRetry, gbcLoad);
		frameLoad.add(butRetry);
		gbcLoad.gridx += 1;
		gbcLoad.gridy += 0;
		gbcLoad.gridwidth = 1;
		gbcLoad.weightx = 1;
		gbcLoad.insets = new Insets(2,2,2,2);
		gblLoad.setConstraints(new JLabel(), gbcLoad);
		frameLoad.setSize(230,130);
	}
	
	 /**
	  * Checks if a recently queried SecondoObject is already displayed.
	  * @return
	  * 	true if SecondoObject is already displayed in viewer.
	  */
	 public boolean isDisplayed(SecondoObject o){
		 for (int i=0;i<objectList.size();i++) {
			 SecondoObject tempSO = objectList.get(i);
			 if (tempSO.getID().equals(o.getID())) return true;			 
		 }
		 return false;
	 } 
	 
	 /**
	  * Removes all objects from the viewer.
	  */
	 public void removeAll(){
		 objectList.clear();
		 STC.clear();
		 recompute();
	 }
	 
	 /**
	  * Checks if the recently queried SecondoObject can be displayed in this viewer.
	  * @return
	  * 	true if the queried SecondoObject can be displayed in this viewer.
	  */
	 public boolean canDisplay(SecondoObject o){
		 ListExpr LE = o.toListExpr();
			ListExpr type;
			  if(LE==null)
			     return false;
			  else{
			     if(LE.listLength()!=2){ // check for type/value-pair
			        return false;
			     }
			     type = LE.first();
			     while(type.atomType()==ListExpr.NO_ATOM){
			       if(type.isEmpty()){
			           return false;
			       }
			       type = type.first();
			     }
			     if(type.atomType()!=ListExpr.SYMBOL_ATOM){
			         return false;
			     }
			     String typeName = type.symbolValue();
			     // if a relation was queried instead of a single MPoint
			     if (typeName.equals("rel")) {
					   ListExpr value=LE.first(); // (rel...
		 			   value = value.second(); // (tuple...
		 			   value = value.second(); // (...
		 			   
		 			   ListExpr tempLE;
		 			   tempLE = value;
		 			  for (int i=0;i<value.listLength();i++) {
						   String s = tempLE.first().second().toString();
						   if (s.length()>=6) {
			 				  if (s.substring(s.length()-6, s.length()).equals("mpoint")) {
			 					 return true;	 				  
			 				  }
							   
						   }
						  tempLE = tempLE.rest();
					   }
		 			  return false;
			     }
			     // if a single MPoint was queried
			     else if (typeName.equals("mpoint")) {
			    	 return true;
			     }
			     else {
			    	 return false;
			     }
			  }
	 }
	 
	 /**
	  * Removes a given SecondoObject from the viewer.
	  */
	 public void removeObject(SecondoObject o){
		 if(isDisplayed(o)) {
			 objectList.remove(o);

			 STC.removeMPoints(o.getID());
			 recompute();
		 }
	 }
	 
	 /**
	  * Not relevant for this viewer.
	  */
	 public  boolean selectObject(SecondoObject o){
			return true;
	 }
	 
	 /**
	  * Gives the name of the viewer.
	  */
	 public String getName(){
	     return "SpaceTimeCube-Viewer";
	 }
	 
	 /**
	  * @return
	  * 	List of all SecondoObjects that are currently maintained in this viewer.
	  */
	 public Vector<SecondoObject> getSecondoObjectList() { return objectList; }
	 
	 /**
	  * Adds a given SecondoObject to the viewer.
	  * @return
	  * 	true if adding the object was successful.
	  */
	 public boolean addObject(SecondoObject o){
		 
		 Calendar rightNow = Calendar.getInstance();
		 long ts1 = rightNow.getTimeInMillis();
		 
		 if(isDisplayed(o)) { return true; }
		 
		 MPointQuery mpQuery = new MPointQuery();
		 		 
		 if (mpQuery.readFromSecondoObject(o)) {
			 STC.addMPointsVector(mpQuery.getMPointsVector());	 
		 }
		 drawMap();

		 float[] tmpCol = {1, 0, 0};
		 colorSO.put(o.getID(), tmpCol);
		 
		 recompute();

		 objectList.add(o);
		 
		 rightNow = Calendar.getInstance();
		 long ts2 = rightNow.getTimeInMillis();
		 double timeToAdd = ((double)(ts2 - ts1)/1000); // in seconds
		 System.out.println("Time to add SecondoObject to STC-Viewer: "+timeToAdd+" seconds");
		 
		 return true;
	}
	 
	 /**
	  * Called when AWT action performed.
	  */
	 public void actionPerformed(ActionEvent e) {
		 
		 // submit button clicked
		 if (e.getSource() == butSubmit) {
			 if (STC.getLimitSet()) {				 
				 minXlimitLast = STC.getMinXlimit();
				 maxXlimitLast = STC.getMaxXlimit();
				 minYlimitLast = STC.getMinYlimit();
				 maxYlimitLast = STC.getMaxYlimit();
				 minZlimitLast = STC.getMinZlimit();
				 maxZlimitLast = STC.getMaxZlimit();
			 }
			 else {
				 minXlimitLast = STC.getMinXnoLimit();
				 maxXlimitLast = STC.getMaxXnoLimit();
				 minYlimitLast = STC.getMinYnoLimit();
				 maxYlimitLast = STC.getMaxYnoLimit();
				 minZlimitLast = STC.getMinZnoLimit();
				 maxZlimitLast = STC.getMaxZnoLimit();
			 }
			 LastLimits tmpLL = new LastLimits(minXlimitLast, maxXlimitLast,
					 minYlimitLast, maxYlimitLast, minZlimitLast, maxZlimitLast);
			 lastLimits.add(tmpLL); // store actual filter/limit so that multiple back actions will work 
			 
			 STC.setMinZlimit((long)zMinSlide.getValue()*1000);
			 STC.setMaxZlimit((long)zMaxSlide.getValue()*1000); // *1000 to get milliseconds
			 
			 setAreaFilter();
			 
			 STC.setLimitSet(true); // tell the STC that a limit is set now
			 STC.recompute();
			 recompute();
		 }
		 // back button clicked
		 else if (e.getSource() == butBack) {
			 if (STC.getLimitSet()) {
				 STC.setMinXlimit(lastLimits.get(lastLimits.size()-1).getMinXlimit());
				 STC.setMaxXlimit(lastLimits.get(lastLimits.size()-1).getMaxXlimit());
				 STC.setMinYlimit(lastLimits.get(lastLimits.size()-1).getMinYlimit());
				 STC.setMaxYlimit(lastLimits.get(lastLimits.size()-1).getMaxYlimit());
				 STC.setMinZlimit(lastLimits.get(lastLimits.size()-1).getMinZlimit());
				 STC.setMaxZlimit(lastLimits.get(lastLimits.size()-1).getMaxZlimit());
			 }
			 
			 STC.setLimitSet(true);
			 STC.recompute();
			 lastLimits.remove(lastLimits.size()-1); // last filter/limit needs to be removed from lastLimits vector
			 recompute();
		 }
		 // reset button clicked
		 else if (e.getSource() == butReset) {
			 STC.setLimitSet(false); // like that a recalculation will be done without any filter
			 STC.recompute();
			 lastLimits.clear();
			 recompute();
		 }
		 // retry button (in loading dialog) clicked
		 else if (e.getSource() == butRetry) {
			 recompute();
		 }
		 // menu item for showing settings clicked
		 else if (e.getSource() == miShowSettings) {
	    	 frameSettings = new SettingsDialog(this); /* always new object necessary because
	    	 	the frame depends on the actual viewer state */
	    	 // locate the frame in the middle of the viewer
	    	 frameSettings.setLocation(getLocationOnScreen().x+(getWidth()/2)-frameSettings.getWidth()/2,
						 getLocationOnScreen().y+(getHeight()/2)-frameSettings.getHeight()/2);
	    	 frameSettings.setVisible(true);
		 }
	 }
	 
	 /**
	  * Called when AWT state changed.
	  */
	 public void stateChanged(ChangeEvent e) {
		 // slider min. Z value changed
		 if (e.getSource() == zMinSlide) {
			 zMinLabel.setText(getTimestampForMilliseconds((long)zMinSlide.getValue()*1000));
		 }
		// slider max. Z value changed
		 else if (e.getSource() == zMaxSlide) {
			 zMaxLabel.setText(getTimestampForMilliseconds((long)zMaxSlide.getValue()*1000));
		 }
		// slider for map movement changed
		 else if (e.getSource() == mapSlide) {
			 if (tgMap != null) {
				 int range = mapSlide.getMaximum() - mapSlide.getMinimum();
				 int realVal = mapSlide.getValue() - mapSlide.getMinimum();
				 double value = -0.5 + ((double)realVal/(double)range);
			 			// milliseconds need to be translated to a value between 0 and 1
				 transformMap.setTranslation(new Vector3d(-0.5+0,-0.5+0,value));
				 tgMap.setTransform(transformMap);
				 
				 String tmp[] = getTimestampForMilliseconds((long)mapSlide.getValue()*1000).split(" ");
				 labelDateMap.setText(tmp[0]);
				 labelTimeMap.setText(tmp[1]);
			 }
		 }
		 // spinner rot. X changed
		 else if (e.getSource() == rotXspinner) {
			 // the actual value is always compared to the last value so that a delta rotation is possible
			 double tmpAngle = ((Integer)rotXspinner.getValue()-rotXlastVal)*(-1);
			 	// *(-1) to change directions of the arrows
			 rotXlastVal = (Integer)rotXspinner.getValue();
			 
			 Transform3D tfRot = new Transform3D();
			 tfRot.rotX(tmpAngle/57.295); // convert from radian to degree
			 Transform3D tfActual = new Transform3D();
			 tg.getTransform(tfActual);
			 
			 tfActual.mul(tfRot); // multiply the actual transform with the rotated one
			 
			 tg.setTransform(tfActual);
			 
			 cbView.setSelectedIndex(0);
		 }
		 // spinner rot. Z changed
		 else if (e.getSource() == rotZspinner) {
			// the actual value is always compared to the last value so that a delta rotation is possible
			 double tmpAngle = (Integer)rotZspinner.getValue()-rotZlastVal;
			 rotZlastVal = (Integer)rotZspinner.getValue();
			 
			 Transform3D tfRot = new Transform3D();
			 tfRot.rotZ(tmpAngle/57.295); // convert from radian to degree
			 Transform3D tfActual = new Transform3D();
			 tg.getTransform(tfActual);
			 
			 tfActual.mul(tfRot); // multiply the actual transform with the rotated one
			 
			 tg.setTransform(tfActual);
			 
			 cbView.setSelectedIndex(0);
		 }
	 }
	 
	 /**
	  * Called when AWT itemstate changed.
	  */
	 public void itemStateChanged(ItemEvent e) {
		 // itemstate in view combobox changed
		 if (e.getSource() == cbView) {
			 int ind = cbView.getSelectedIndex();
			 if (ind > 0) {
				 ind-=1;
				 Point3d eye = new Point3d(views.get(ind)[0],views.get(ind)[1],views.get(ind)[2]);
				 Vector3d up = new Vector3d(views.get(ind)[3],views.get(ind)[4],views.get(ind)[5]);
				 Point3d center = new Point3d(0.0,0.0,0.0); // (0,0,0) is the center of the cube
				 Transform3D actualTransform = new Transform3D();
				 tg.getTransform(actualTransform);
				 actualTransform.lookAt(eye, center, up);
				 tg.setTransform(actualTransform);
				 
				 rotXspinner.setValue(new Integer(0));
				 rotZspinner.setValue(new Integer(0));
			 }
		 }
	 }
	 
	 /**
	  * Called when mouse was clicked.
	  */
	 public void mouseClicked(MouseEvent e) {
		 // click was done on canvas
		 if (e.getSource() == canvas) {
			 // set all dialogs invisible when clicking on the canvas
			 if (frameMPoint != null && frameMPoint.isVisible()) frameMPoint.setVisible(false);
			 if (frameSettings != null && frameSettings.isVisible()) frameSettings.setVisible(false);
			 
			 rotXlastVal = 0;
			 rotZlastVal = 0;
			 rotXspinner.setValue(new Integer(0));
			 rotZspinner.setValue(new Integer(0));
			 
			 cbView.setSelectedIndex(0);
		 }
		 /* Right mouse button clicked.
		  * If the cursor was located on top of a MPoint,
		  * it's additional attributes will be shown in a frame.
		  */
		 if (e.getButton() == e.BUTTON3) {
			 pickCanvas.setShapeLocation(e); 
			 PickResult[] result = pickCanvas.pickAllSorted(); // stores potentially picked objects with right mouse button
			 
			 int[] indexesAfterFilter = STC.getIndexesAfterFilter();
			 		// if a filter is applied the order of the indexes may be different.
 			 
			 if (result != null) {
				 for (int i=0;i<result.length;i++) {
					 Shape3D tmpShape = null;
					 tmpShape = (Shape3D)result[i].getNode(PickResult.SHAPE3D);
					 for (int a=0;a<mPoints.size();a++) {
						 if (tmpShape.equals(mPoints.get(a))) { // when picked object matches an MPoint/pointarray
							 int ind = indexesAfterFilter[a];
							 MPoint tmpMPoint = STC.getMPointsVector().get(ind);
							 Hashtable<String,String> tmpHt = tmpMPoint.getAdditionalAttributes();
							 
							 // a frame/dialog for showing the additional attributes to an MPoint is created
							 int rows = tmpHt.size();
							 frameMPoint = new JFrame();
							 frameMPoint.setUndecorated(true);
							 GridBagLayout gbLayout = new GridBagLayout();
							 GridBagConstraints gbc = new GridBagConstraints();
							 gbc.fill=GridBagConstraints.HORIZONTAL;
							 gbc.insets = new Insets(2,2,2,2);
							 gbc.gridx=0;
							 gbc.gridy=0;
							 gbc.gridheight=1;
							 gbc.gridwidth=1;
							 frameMPoint.setLayout(gbLayout);								
							 
							 // looping through the Hashtable that stores the addtional attributes to a MPoint
							 JLabel labelKey=null, labelVal=null; 
							 for (Enumeration<String> b = tmpHt.keys(); b.hasMoreElements();) {
								 String key = b.nextElement();
								 labelKey = new JLabel(key+": ");
								 labelVal = new JLabel(tmpHt.get(key));
								 gbLayout.setConstraints(labelKey, gbc);
								 frameMPoint.add(labelKey);
								 gbc.gridx++;
								 gbLayout.setConstraints(labelVal, gbc);
								 frameMPoint.add(labelVal);
								 								 
								 gbc.gridy++;
								 gbc.gridx=0;
							 }
							 
							 // the dialog's height is calculated based on rows, fontsize and insets
							 int height = (rows*(labelKey.getFont().getSize()+((gbc.insets.top+2)*2)));
							 frameMPoint.setSize(frameMPoint.getPreferredSize().width, height);
							 // the dialog/frame will be shown in the center of the canvas
							 Point canvasLoc = canvas.getLocationOnScreen();
							 frameMPoint.setLocation(canvasLoc.x+e.getPoint().x-10, canvasLoc.y+e.getPoint().y-10);
							 frameMPoint.setVisible(true);

							 return;
						 }
					 }
				 }
			 }
		 }
	 }
	 
	 public void mouseReleased(MouseEvent e) { }
	 
	 public void mouseEntered(MouseEvent e) { }
	 
	 public void mousePressed(MouseEvent e) {
		 // mouse was pressed on viewing area/canvas
		 if (e.getSource() == canvas) {
			// set all dialogs invisible when clicking on the canvas
			 if (frameMPoint != null && frameMPoint.isVisible()) frameMPoint.setVisible(false);
			 if (frameSettings != null && frameSettings.isVisible()) frameSettings.setVisible(false);
			 
			 // rotation calculation attributes are reset
			 rotXlastVal = 0;
			 rotZlastVal = 0;
			 rotXspinner.setValue(new Integer(0));
			 rotZspinner.setValue(new Integer(0));
			 
			 /*
			  * Pressing the mouse indicates that the view was changed by dragging.
			  * That means the user is no longer working with a pre-defined view.
			  * => view combobox is reset.
			  */
			 cbView.setSelectedIndex(0);
		 }	
	 }
	 
	 public void mouseExited(MouseEvent e) { }
	 
	 public MenuVector getMenuVector(){
		 return MenuExtension;
	 }
	 
	 // Settings section /////////////////////
	 /**
	  * @return
	  * 	mpoint/trajectory color for each queried SecondoObject, defined by SecondoIDs.
	  */
	 public Hashtable<ID,float[]> getColorSO() { return colorSO; }
	 
	 /**
	  * @param colSO
	  * 	Hashtable which connects multiple SecondoIDs to colors as float[] values.
	  */
	 public void setColorSO(Hashtable<ID,float[]> colSO) { colorSO = colSO; }
	 
	 /**
	  * @return
	  * 	true if currently vertical lines are drawn.
	  */
	 public boolean isDrawVertLines() { return drawVertLines; }
	 
	 /**
	  * @param b
	  * 	indicates if vertical lines are currently drawn.
	  */
	 public void setDrawVertLines(boolean b) { drawVertLines = b; }
	 
	 /**
	  * @return
	  * 	the actual background/canvas color
	  */
	 public Color3f getColorCanvas() { return colorCanvas; }
	 
	 /**
	  * @param col
	  * 	actual background/canvas color that will be set
	  */
	 public void setColorCanvas(Color3f col) { colorCanvas = col; }
	 
	 /**
	  * @return
	  * 	the actual mpoint/trajectory line width/weight.
	  */
	 public float getLineWidth() { return lineWidth; }
	 
	 /**
	  * @param weight
	  * 	mpoint/trajectory line width/weight that will be set.
	  */
	 public void setLineWidth(float weight) { lineWidth = weight; }
	 
	 /**
	  * @return
	  * 	the actual mpoint/trajectory transparency value.
	  */
	 public float getTranspMPoints() { return transpMPoints; }
	 
	 /**
	  * @param transp
	  * 	mpoint/trajectory transparency value that will be set.
	  */
	 public void setTranspMPoints(float transp) { transpMPoints = transp; }
	 /////////////////////////////////////////
	 
	 /**
	  * Following computations are done:
	  * 1. Extraction of points out of all MPoints.
	  * 2. Creation of linestriparrays based on "1.".
	  * 3. Map will be drawn.
	  * 4. Creation of the 'outside' of the cube including axis, labels, etc..
	  * 5. Lines and map are added to the basic TransformGroup and the TG is added to a BranchGroup.
	  * 6. Behaviors are added to the BranchGroup.
	  */
	 public void recompute() {

		 if (lastLimits.size()>0) butBack.setEnabled(true); // if there is sth. to go back to
		 else butBack.setEnabled(false);
			 
		 Vector<Vector> STCpointArrays = STC.getPointArrays(); // corresponds to all MPoints maintained in the STC
		 Vector<Shape3D> mPointShapes = new Vector<Shape3D>();	// stores all MPoints as Shape3Ds
		 Vector<Shape3D> vertShapes = new Vector<Shape3D>(); // stores all vertical lines as Shape3Ds
		 Vector<Vector> p2dArrays = new Vector<Vector>(); /* stores the 2D point coordinates
		  	* of the mpoints/trajectories that will have to be displayed in the 2D filter view.
		 	*/

		 /*
		  * For each MPoint the single points are extraced and stored to a linestriparray.
		  * The vertical lines are also stored into a linestriparray. 
		  */
		 for (int i=0;i<STCpointArrays.size();i++) {
			 
			 Vector<Point3DSTC> pointArray = STCpointArrays.get(i);	// corresponds to a MPoint
			 Vector<Point2DSTC> p2ds = new Vector<Point2DSTC>(); // stores 2D point coordinates of the mpoint
			 p2dArrays.add(p2ds);
			 
			 int pointArraySize = pointArray.size();
			 int vertexCount = pointArraySize;
			 boolean uneven = false; /* if it is an uneven amount of points an additional point will be added at the end
			 	* that has the same coordinates as the actual last one.
			 	*/
			 if ((vertexCount%2)!=0) {
				 vertexCount++;
				 uneven = true;
			 }
			 int[] stripVertexCounts = new int[1];
			 stripVertexCounts[0] = vertexCount;
			 LineStripArray la = new LineStripArray(vertexCount,LineStripArray.COORDINATES | 
					 LineStripArray.COLOR_3 ,stripVertexCounts);
			 la.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
			 			 
			 Point3d p3d[] = new Point3d[vertexCount]; // stores all 3D points of this mpoint
			 double x=0,y=0,z=0;
			 
			 // definition for the vertical lines
			 int[] stripVertexCountsVert = new int[1];
			 stripVertexCountsVert[0] = 2;
			 int vertexCountVert = 2;
			 int count=0;
			 int vertLineAmount = 50;
			 int divider;
			 
			 /*
			  *  Usually 50 vertical lines per mpoint are drawn.
			  *  If the area is filtered a mpoint may contain less than
			  *  50 single points. Then 50 vertical lines can't be drawn.
			  */
			 if (pointArraySize > (vertLineAmount*vertLineAmount)) divider = vertLineAmount;
			 else divider = pointArraySize/vertLineAmount;
			 if (divider==0) divider = 1;
			 
			 float[] colVert = {0.5f, 0.5f, 0.5f}; // color of vertical lines
			 Vector<LineStripArray> linesVert = new Vector<LineStripArray>();
			 Appearance apVert = new Appearance();
			 // set 50% transparency for vertical lines
			 TransparencyAttributes taVert = new TransparencyAttributes(TransparencyAttributes.FASTEST, 0.5f);
			 apVert.setTransparencyAttributes(taVert);
			 
			 float[] colSO = {0,0,0}; // default color of all SecondoObjects/MPoints
			 
			 for (int a=0;a<pointArraySize;a++) { // looping through all points in this MPoint
				 Point3DSTC point = pointArray.get(a);
				 x = point.getX();
				 y = point.getY();
				 z = point.getZ();
				 colSO = colorSO.get(point.getSecondoID());
				 double xyLength = STC.getXYlength(); // x and y must have the same length
				 double zLength = STC.getZlength();
				 
				 /*
				  * x,y and z coordinates are converted to a value between 0 and 1 and
				  * then moved -0,5 in each direction so that the rotation center will be (0,0,0).
				  */
				 x = -0.5 + ((1/xyLength) * (x-STC.getMinX()));
				 y = -0.5 + ((1/xyLength) * (y-STC.getMinY()));
				 z = -0.5 + ((1/zLength) * (z-STC.getMinZ()));
				 	 
				 p3d[a] = new Point3d(x,y,z);
				 if (drawVertLines) { // if vertical lines shall be drawn
					 if (count <= (a/divider)) { // a = point number; divider = see above;
						 if (a%divider == 0) {
							 LineStripArray laVert = new LineStripArray(vertexCountVert, LineStripArray.COORDINATES | 
									 LineStripArray.COLOR_3, stripVertexCountsVert);
							 laVert.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
							 Point3d p3dVert[] = new Point3d[vertexCountVert];
							 p3dVert[0] = p3d[a];
							 p3dVert[1] = new Point3d(p3d[a].x,p3d[a].y,-0.5+0);
							 laVert.setCoordinates(0, p3dVert);
							 laVert.setColor(0, colVert);
							 laVert.setColor(1, colVert);
							 
							 linesVert.add(laVert);
							 
							 count++;
						 }
					 }
				 }
				 p2ds.add(new Point2DSTC(x+0.5,y+0.5,point.getSecondoID())); // 2D points for 2D-view
			 }			 
			 if (uneven) {
				 p3d[vertexCount-1] = new Point3d(x,y,z); // an additional point will be added
			 }
			 la.setCoordinates(0, p3d);
			 
			 // each mpoint/trajectory will be colored a little bit different (factor 0.70)
			 float[] tmpColor = {0,0,0};
			 if ((i+2)%2 == 0) {
				 tmpColor = colSO.clone();
			 }
			 else {
				 tmpColor[0] = 0.70f * colSO[0];
				 tmpColor[1] = 0.70f * colSO[1];
				 tmpColor[2] = 0.70f * colSO[2];
			 }
			 
			 for (int a=0;a<la.getVertexCount();a++) {
				 la.setColor(a, tmpColor); // color is set for each point
			 }
			 
			 for (int a=0;a<linesVert.size();a++) {
				 vertShapes.add(new Shape3D(linesVert.get(a), apVert)); // vertical lines are stored in a single vector
			 }
			 
			 mPointShapes.add(new Shape3D(la));
		 }
		 
		 // drawing map
		 drawMap();
		 
		 if (STC.downloadsCompleted()) {
			 frameLoad.setVisible(false);
			 
			 if (lastBg!=null) universe.getLocale().removeBranchGraph(lastBg);
			 
			 tg = new TransformGroup();
			 tg.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
			 tg.setCapability(TransformGroup.ENABLE_PICK_REPORTING);
			 
			 BranchGroup bg = new BranchGroup();
			 bg.setCapability(BranchGroup.ALLOW_DETACH);
			 
			 // create the 'outside' of the cube including axis, labels, etc.
			 createCube(tg);
			 
			 mPoints.clear();
			 
			 // TransformGroup tg will be filled
			 for (int i=0;i<mPointShapes.size();i++) {
				 Shape3D tempShape = mPointShapes.get(i);
				 				 
				 // only works under OpenGL
				 Appearance tmpApp = new Appearance();
				 LineAttributes tmpLA = new LineAttributes();
				 tmpLA.setLineWidth(lineWidth); 
				 tmpLA.setLinePattern(LineAttributes.PATTERN_SOLID);
				 tmpApp.setLineAttributes(tmpLA);
				 
				 // set transparency as per defined settings
				 TransparencyAttributes taMPoints = new TransparencyAttributes(TransparencyAttributes.FASTEST, transpMPoints);
				 tmpApp.setTransparencyAttributes(taMPoints);
				 
				 Shape3D mPoint = new Shape3D(tempShape.getGeometry(), tmpApp); // corresponds to 1 MPoint
				 
				 mPoints.add(mPoint);
				 
				 tg.addChild(mPoint);
			 }
			 for (int i=0;i<vertShapes.size();i++) {
				 Shape3D tempShape = vertShapes.get(i);
				 Shape3D vertLine = new Shape3D(tempShape.getGeometry(), tempShape.getAppearance());
				 tg.addChild(vertLine);
			 }
			 if (mapShape != null) {
				 Shape3D tempShape = new Shape3D(mapShape.getGeometry(),mapShape.getAppearance());
				 tgMap = new TransformGroup();
				 tgMap.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
				 tgMap.addChild(tempShape);
				 Transform3D tmpTransform = new Transform3D();
				 tmpTransform.setTranslation(new Vector3d(-0.5+0,-0.5+0,-0.5+0));
				 tgMap.setTransform(tmpTransform);
				 tg.addChild(tgMap);
			 }
			 
			 // initialize panel for 2D-view
			 panelMapOverview.initialize(map, p2dArrays, panelMapOverview.getPreferredSize().width, this);
			 panelMapOverview.updateUI();
			 
			 tg.setTransform(transform); // initial transform of tg (basic TransformGroup)

			 lastBg = bg;
			 bg.addChild(tg);
			 
			 // add Behaviors to the BranchGroup
			 PickRotateBehavior rotateBehavior = new PickRotateBehavior(bg, canvas, tg.getBounds());
			 bg.addChild(rotateBehavior);
			 PickTranslateBehavior translateBehavior = new PickTranslateBehavior(bg, canvas, tg.getBounds());
			 bg.addChild(translateBehavior);
			 PickZoomBehavior zoomBehavior = new PickZoomBehavior(bg, canvas, tg.getBounds());
			 bg.addChild(zoomBehavior);
			 
			 pickCanvas = new PickCanvas(canvas, bg); 
		     pickCanvas.setMode(PickCanvas.GEOMETRY); 
		     canvas.addMouseListener(this);
		     
		     // set background
		     Background background = new Background(colorCanvas);
		     BoundingSphere sphere = new BoundingSphere(new Point3d(0,0,0), 100000);
		     background.setApplicationBounds(sphere);
		     bg.addChild(background);

			 universe.addBranchGraph(bg);
		 }
		 else { // if map download is going on
			 frameLoad.setLocation(getLocationOnScreen().x+(getWidth()/2)-frameLoad.getWidth()/2,
					 getLocationOnScreen().y+(getHeight()/2)-frameLoad.getHeight()/2);
			 frameLoad.setVisible(true);
		 }
		 
		 // setting JSlider values as per STC min and max values
		 zMinSlide.setMinimum((int)(STC.getMinZ()/1000));
		 zMinSlide.setMaximum((int)(STC.getMaxZ()/1000));
		 zMinSlide.setValue((int)(STC.getMinZlimit()/1000));
		 zMaxSlide.setMinimum((int)(STC.getMinZ()/1000));
		 zMaxSlide.setMaximum((int)(STC.getMaxZ()/1000));
		 zMaxSlide.setValue((int)(STC.getMaxZlimit()/1000));
		 mapSlide.setMinimum((int)(STC.getMinZ()/1000));
		 mapSlide.setMaximum((int)(STC.getMaxZ()/1000));
		 mapSlide.setValue((int)(STC.getMinZlimit()/1000));
		 mapSlide.addChangeListener(this);
		 
		 zMinLabel.setText(getTimestampForMilliseconds((long)zMinSlide.getValue()*1000));
		 zMaxLabel.setText(getTimestampForMilliseconds((long)zMaxSlide.getValue()*1000));
		 
		 String tmp[] = getTimestampForMilliseconds((long)mapSlide.getValue()*1000).split(" ");
		 labelDateMap.setText(tmp[0]);
		 labelTimeMap.setText(tmp[1]);
		 
	 }
	 
	 /*
	  * A (quadratic) Shape3D is created which Appearance/Texture is the map
	  * as BufferedImage provided by STC.generateMap().
	  */
	 private void drawMap() {
		 mapShape = null;
		 map = STC.generateMap();
		 if (map != null) {				 
			 Texture txt = new TextureLoader(map).getTexture();
			 Appearance ap = new Appearance();			  
			 ap.setTexture(txt);
			 
			 float[] tcoords =
			 {
			  0, 0,
			  1, 0,
			  1, 1,
			  0, 1
			 };
			 QuadArray plane = new QuadArray(4, QuadArray.COORDINATES
						| QuadArray.TEXTURE_COORDINATE_2);
			 plane.setCoordinate(0, new Point3d(0,0,0));
			 plane.setCoordinate(1, new Point3d(1,0,0));
			 plane.setCoordinate(2, new Point3d(1,1,0));
			 plane.setCoordinate(3, new Point3d(0,1,0));
			 plane.setTextureCoordinates(0, 0, tcoords);
			 
			 mapShape = new Shape3D(plane, ap);
		 }
	 }
	 
	 /*
	  * Creates the 'outside' of the cube as lines, quads and texts/labels.
	  */
	 private void createCube(TransformGroup tg) {

		 Appearance ap = new Appearance();
		 // setting 50% transparency for the lines
		 TransparencyAttributes ta = new TransparencyAttributes(TransparencyAttributes.FASTEST, 0.5f);
		 PolygonAttributes pa = new PolygonAttributes(PolygonAttributes.POLYGON_LINE, PolygonAttributes.CULL_NONE, 0.0f);
		 	// POLYGON_LINE is necessary so that the polygons are not filled
		 ap.setTransparencyAttributes(ta);
		 ap.setPolygonAttributes(pa);
		 int[] stripVertexCounts = new int[1];
		 stripVertexCounts[0] = 2;
		 int vertexCount = 2;
		 float[] colAxis = {1,0.4f,0}; // axis color
		 float[] colCube = {0.5f,0.5f,0.5f}; // usual line color of the cube
		
		 LineStripArray laXaxis = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laXaxis.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dXaxis[] = new Point3d[vertexCount];
		 p3dXaxis[0] = bottomFrontLeft;
		 p3dXaxis[1] = bottomFrontRight;
		 laXaxis.setCoordinates(0, p3dXaxis);
		 laXaxis.setColor(0, colAxis);
		 laXaxis.setColor(1, colAxis);
		 lineXaxis = new Shape3D(laXaxis, ap);
		
		 LineStripArray laYaxis = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laYaxis.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dYaxis[] = new Point3d[vertexCount];
		 p3dYaxis[0] = bottomFrontLeft;
		 p3dYaxis[1] = bottomBackLeft;
		 laYaxis.setCoordinates(0, p3dYaxis);
		 laYaxis.setColor(0, colAxis);
		 laYaxis.setColor(1, colAxis);
		 lineYaxis = new Shape3D(laYaxis, ap);
		
		 LineStripArray laZaxis = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laZaxis.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dZaxis[] = new Point3d[vertexCount];
		 p3dZaxis[0] = bottomFrontLeft;
		 p3dZaxis[1] = topFrontLeft;
		 laZaxis.setCoordinates(0, p3dZaxis);
		 laZaxis.setColor(0, colAxis);
		 laZaxis.setColor(1, colAxis);
		 lineZaxis = new Shape3D(laZaxis, ap);
		
		 LineStripArray laBottomBack = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laBottomBack.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dBottomBack[] = new Point3d[vertexCount];
		 p3dBottomBack[0] = bottomBackLeft;
		 p3dBottomBack[1] = bottomBackRight;
		 laBottomBack.setCoordinates(0, p3dBottomBack);
		 laBottomBack.setColor(0, colCube);
		 laBottomBack.setColor(1, colCube);
		 lineBottomBack = new Shape3D(laBottomBack, ap);
		
		 LineStripArray laBottomRight = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laBottomRight.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dBottomRight[] = new Point3d[vertexCount];
		 p3dBottomRight[0] = bottomFrontRight;
		 p3dBottomRight[1] = bottomBackRight;
		 laBottomRight.setCoordinates(0, p3dBottomRight);
		 laBottomRight.setColor(0, colCube);
		 laBottomRight.setColor(1, colCube);
		 lineBottomRight = new Shape3D(laBottomRight, ap);
		
		 LineStripArray laLeftBack = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laLeftBack.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dLeftBack[] = new Point3d[vertexCount];
		 p3dLeftBack[0] = bottomBackLeft;
		 p3dLeftBack[1] = topBackLeft;
		 laLeftBack.setCoordinates(0, p3dLeftBack);
		 laLeftBack.setColor(0, colCube);
		 laLeftBack.setColor(1, colCube);
		 lineLeftBack = new Shape3D(laLeftBack, ap);
		
		 LineStripArray laRightFront = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laRightFront.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dRightFront[] = new Point3d[vertexCount];
		 p3dRightFront[0] = bottomFrontRight;
		 p3dRightFront[1] = topFrontRight;
		 laRightFront.setCoordinates(0, p3dRightFront);
		 laRightFront.setColor(0, colCube);
		 laRightFront.setColor(1, colCube);
		 lineRightFront = new Shape3D(laRightFront, ap);
		
		 LineStripArray laRightBack = new LineStripArray(vertexCount, LineStripArray.COORDINATES | 
				 LineStripArray.COLOR_3, stripVertexCounts);
		 laRightBack.setCapability(LineStripArray.ALLOW_COLOR_WRITE);
		 Point3d p3dRightBack[] = new Point3d[vertexCount];
		 p3dRightBack[0] = bottomBackRight;
		 p3dRightBack[1] = topBackRight;
		 laRightBack.setCoordinates(0, p3dRightBack);
		 laRightBack.setColor(0, colCube);
		 laRightBack.setColor(1, colCube);
		 lineRightBack = new Shape3D(laRightBack, ap);
		 
		 // the top of the cube can be created as whole because all lines of the quad look same
		 QuadArray top = new QuadArray(4, QuadArray.COORDINATES | QuadArray.COLOR_3);
		 top.setCapability(QuadArray.ALLOW_COLOR_WRITE);
		 top.setColor(0,colCube);
		 top.setColor(1,colCube);
		 top.setColor(2,colCube);
		 top.setColor(3,colCube);
		 top.setCoordinate(0, topFrontLeft);
		 top.setCoordinate(1, topFrontRight);
		 top.setCoordinate(2, topBackRight);
		 top.setCoordinate(3, topBackLeft);
		 topShape = new Shape3D(top, ap);
		 
		 // all axis, lines and quads are added to the basic TransformGroup
		 tg.addChild(new Shape3D(lineXaxis.getGeometry(),lineXaxis.getAppearance()));
		 tg.addChild(new Shape3D(lineYaxis.getGeometry(),lineYaxis.getAppearance()));
		 tg.addChild(new Shape3D(lineZaxis.getGeometry(),lineZaxis.getAppearance()));
		 tg.addChild(new Shape3D(lineBottomBack.getGeometry(),lineBottomBack.getAppearance()));
		 tg.addChild(new Shape3D(lineBottomRight.getGeometry(),lineBottomRight.getAppearance()));
		 tg.addChild(new Shape3D(lineLeftBack.getGeometry(),lineLeftBack.getAppearance()));
		 tg.addChild(new Shape3D(lineRightFront.getGeometry(),lineRightFront.getAppearance()));
		 tg.addChild(new Shape3D(lineRightBack.getGeometry(),lineRightBack.getAppearance()));
		 tg.addChild(new Shape3D(topShape.getGeometry(),topShape.getAppearance()));
		 
		 Color3f colTextCoords = new Color3f(0.3f,0.3f,1); // color of the coordinate labels
		 Color3f colTextAxis = new Color3f(1,0.4f,0); // color of the axis' labels
		 Appearance apTextCoords = new Appearance();
		 apTextCoords.setColoringAttributes(new ColoringAttributes(colTextCoords, ColoringAttributes.SHADE_FLAT));
		 Appearance apTextAxis = new Appearance();
		 apTextAxis.setColoringAttributes(new ColoringAttributes(colTextAxis, ColoringAttributes.SHADE_FLAT));
		 Font3D font = new Font3D(new Font("Arial", Font.ROMAN_BASELINE, 1), new FontExtrusion());
		 double scaleTextCoords = 0.15; // size of coordinate labels
		 double scaleTextAxis = 0.2; // size of axis' labels
		 String temp;
		 
		 Text3D textMinX = new Text3D(font);
		 if (String.valueOf(STC.getMinXtext()).length() >= 7) temp = String.valueOf(STC.getMinXtext()).substring(0, 7);
		 else temp = String.valueOf(STC.getMinXtext());
		 textMinX.setString(temp);
		 OrientedShape3D osTextMinX = new OrientedShape3D(textMinX, apTextCoords, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextMinX.setConstantScaleEnable(true);
		 osTextMinX.setScale(scaleTextCoords);
		 Point3d p3dTextMinX = new Point3d(-0.5+0.1,-0.5+0,-0.5+0.01);
		 Transform3D transTextMinX = new Transform3D();
		 transTextMinX.setTranslation(new Vector3d(p3dTextMinX));
		 TransformGroup tgTextMinX = new TransformGroup();
		 tgTextMinX.setTransform(transTextMinX);
		 tgTextMinX.addChild(osTextMinX);
		 
		 Text3D textMinY = new Text3D(font);
		 if (String.valueOf(STC.getMinYtext()).length() >= 7) temp = String.valueOf(STC.getMinYtext()).substring(0, 7);
		 else temp = String.valueOf(STC.getMinYtext());
		 textMinY.setString(temp);
		 OrientedShape3D osTextMinY = new OrientedShape3D(textMinY, apTextCoords, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextMinY.setConstantScaleEnable(true);
		 osTextMinY.setScale(scaleTextCoords);		 
		 Point3d p3dTextMinY = new Point3d(-0.5+0,-0.5+0.1,-0.5+0.01);
		 Transform3D transTextMinY = new Transform3D();
		 transTextMinY.setTranslation(new Vector3d(p3dTextMinY));
		 TransformGroup tgTextMinY = new TransformGroup();
		 tgTextMinY.setTransform(transTextMinY);
		 tgTextMinY.addChild(osTextMinY);
		 
		 Text3D textMinZ = new Text3D(font);
		 if (String.valueOf(STC.getMinZtext()).length() >= 7) temp = String.valueOf(STC.getMinZtext());
		 else temp = String.valueOf(STC.getMinZtext());
		 textMinZ.setString(temp);
		 OrientedShape3D osTextMinZ = new OrientedShape3D(textMinZ, apTextCoords, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextMinZ.setConstantScaleEnable(true);
		 osTextMinZ.setScale(scaleTextCoords);		 
		 Point3d p3dTextMinZ = new Point3d(-0.5+0,-0.5+0,-0.5+0.1);
		 Transform3D transTextMinZ = new Transform3D();
		 transTextMinZ.setTranslation(new Vector3d(p3dTextMinZ));
		 TransformGroup tgTextMinZ = new TransformGroup();
		 tgTextMinZ.setTransform(transTextMinZ);
		 tgTextMinZ.addChild(osTextMinZ);
		 
		 Text3D textMaxX = new Text3D(font);
		 if (String.valueOf(STC.getMaxXtext()).length() >= 7) temp = String.valueOf(STC.getMaxXtext()).substring(0, 7);
		 else temp = String.valueOf(STC.getMaxXtext());
		 textMaxX.setString(temp);
		 OrientedShape3D osTextMaxX = new OrientedShape3D(textMaxX, apTextCoords, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextMaxX.setConstantScaleEnable(true);
		 osTextMaxX.setScale(scaleTextCoords);		 
		 Point3d p3dTextMaxX = new Point3d(-0.5+1-0.1,-0.5+0,-0.5+0.01);
		 Transform3D transTextMaxX = new Transform3D();
		 transTextMaxX.setTranslation(new Vector3d(p3dTextMaxX));
		 TransformGroup tgTextMaxX = new TransformGroup();
		 tgTextMaxX.setTransform(transTextMaxX);
		 tgTextMaxX.addChild(osTextMaxX);
		 
		 Text3D textMaxY = new Text3D(font);
		 if (String.valueOf(STC.getMaxYtext()).length() >= 7) temp = String.valueOf(STC.getMaxYtext()).substring(0, 7);
		 else temp = String.valueOf(STC.getMaxYtext());
		 textMaxY.setString(temp);
		 OrientedShape3D osTextMaxY = new OrientedShape3D(textMaxY, apTextCoords, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextMaxY.setConstantScaleEnable(true);
		 osTextMaxY.setScale(scaleTextCoords);		 
		 Point3d p3dTextMaxY = new Point3d(-0.5+0,-0.5+1-0.1,-0.5+0.01);
		 Transform3D transTextMaxY = new Transform3D();
		 transTextMaxY.setTranslation(new Vector3d(p3dTextMaxY));
		 TransformGroup tgTextMaxY = new TransformGroup();
		 tgTextMaxY.setTransform(transTextMaxY);
		 tgTextMaxY.addChild(osTextMaxY);
		 
		 Text3D textMaxZ = new Text3D(font);
		 if (String.valueOf(STC.getMaxZtext()).length() >= 7) temp = String.valueOf(STC.getMaxZtext());
		 else temp = String.valueOf(STC.getMaxZtext());
		 textMaxZ.setString(temp);
		 OrientedShape3D osTextMaxZ = new OrientedShape3D(textMaxZ, apTextCoords, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextMaxZ.setConstantScaleEnable(true);
		 osTextMaxZ.setScale(scaleTextCoords);		 
		 Point3d p3dTextMaxZ = new Point3d(-0.5+0,-0.5+0,-0.5+1-0.1);
		 Transform3D transTextMaxZ = new Transform3D();
		 transTextMaxZ.setTranslation(new Vector3d(p3dTextMaxZ));
		 TransformGroup tgTextMaxZ = new TransformGroup();
		 tgTextMaxZ.setTransform(transTextMaxZ);
		 tgTextMaxZ.addChild(osTextMaxZ);
		 
		 Text3D textXaxis = new Text3D(font);
		 textXaxis.setString("X");
		 OrientedShape3D osTextXaxis = new OrientedShape3D(textXaxis, apTextAxis, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextXaxis.setConstantScaleEnable(true);
		 osTextXaxis.setScale(scaleTextAxis);		 
		 Point3d p3dTextXaxis = new Point3d(-0.5+0.5,-0.5+0,-0.5+0.01);
		 Transform3D transTextXaxis = new Transform3D();
		 transTextXaxis.setTranslation(new Vector3d(p3dTextXaxis));
		 TransformGroup tgTextXaxis = new TransformGroup();
		 tgTextXaxis.setTransform(transTextXaxis);
		 tgTextXaxis.addChild(osTextXaxis);
		 
		 Text3D textYaxis = new Text3D(font);
		 textYaxis.setString("Y");
		 OrientedShape3D osTextYaxis = new OrientedShape3D(textYaxis, apTextAxis, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextYaxis.setConstantScaleEnable(true);
		 osTextYaxis.setScale(scaleTextAxis);		 
		 Point3d p3dTextYaxis = new Point3d(-0.5+0,-0.5+0.5,-0.5+0.01);
		 Transform3D transTextYaxis = new Transform3D();
		 transTextYaxis.setTranslation(new Vector3d(p3dTextYaxis));
		 TransformGroup tgTextYaxis = new TransformGroup();
		 tgTextYaxis.setTransform(transTextYaxis);
		 tgTextYaxis.addChild(osTextYaxis);
		 
		 Text3D textZaxis = new Text3D(font);
		 textZaxis.setString("Z");
		 OrientedShape3D osTextZaxis = new OrientedShape3D(textZaxis, apTextAxis, 
				 OrientedShape3D.ROTATE_ABOUT_POINT, new Point3f(0,0,0));
		 osTextZaxis.setConstantScaleEnable(true);
		 osTextZaxis.setScale(scaleTextAxis);		 
		 Point3d p3dTextZaxis = new Point3d(-0.5+0,-0.5+0,-0.5+0.5);
		 Transform3D transTextZaxis = new Transform3D();
		 transTextZaxis.setTranslation(new Vector3d(p3dTextZaxis));
		 TransformGroup tgTextZaxis = new TransformGroup();
		 tgTextZaxis.setTransform(transTextZaxis);
		 tgTextZaxis.addChild(osTextZaxis);
		 
		 // all texts are added to the basic TransformGroup
		 tg.addChild(tgTextMinX);
		 tg.addChild(tgTextMinY);
		 tg.addChild(tgTextMinZ);
		 tg.addChild(tgTextMaxX);
		 tg.addChild(tgTextMaxY);
		 tg.addChild(tgTextMaxZ);
		 tg.addChild(tgTextXaxis);
		 tg.addChild(tgTextYaxis);
		 tg.addChild(tgTextZaxis);
	 }
	 
	 /*
	  * Get timestamp as String for milliseconds since 1970.
	  */
	 private String getTimestampForMilliseconds(long millisec) {
		 Timestamp ts = new Timestamp(millisec);
		 return ts.toString();		 
	 }
	 
	 /*
	  * Limits/filters the current SecondoObjects according to the selection in the 2D-view.
	  * That means X,Y min and max limit values will be reset.
	  */
	 private void setAreaFilter() {
		 // get the selected area as pixel value
		 // rectangle from bottom left to top right
		 Rectangle2D.Double area2DView = panelMapOverview.getFilterArea();
		 
		 // pixel values need to be translated to STC values
		 double selectedAreaWidth = STC.getXYlength()/panelMapOverview.getWidth()*area2DView.getWidth();
		 double selectedAreaHeight = STC.getXYlength()/panelMapOverview.getHeight()*area2DView.getHeight();
		 
		 double changeFactor = 0.05; // puffer that is added to the selection
		 double widthChange, heightChange;
		 
		 // puffer values as STC values
		 widthChange = selectedAreaWidth * changeFactor;
		 heightChange = selectedAreaHeight * changeFactor;
		 
		 // X,Y values are converted to values between 0 and 1
		 double xMin = STC.getMinX()+(STC.getXYlength()/panelMapOverview.getWidth()*area2DView.getX())-widthChange;
		 double yMin = STC.getMinY()+(STC.getXYlength()/panelMapOverview.getHeight()*
		 		(panelMapOverview.getHeight()-area2DView.getY()-area2DView.getHeight()))-heightChange;
		 double xMax = xMin+selectedAreaWidth+(2*widthChange);
		 double yMax = yMin+selectedAreaHeight+(2*heightChange);
		 
		 Point2D.Double p1 = new Point2D.Double();
		 if (STC.isWorldcoord()) osmm.getOrig(xMin,yMin,p1);
		 else p1.setLocation(xMin, yMin);
		 Point2D.Double p2 = new Point2D.Double();
		 if (STC.isWorldcoord()) osmm.getOrig(xMax,yMax,p2);
		 else p2.setLocation(xMax, yMax);
		 
		 // set the new X,Y limit/filter according to the selection in the 2D-view 
		 STC.setMinXlimit(p1.getX());
		 STC.setMinYlimit(p1.getY());
		 STC.setMaxXlimit(p2.getX());
		 STC.setMaxYlimit(p2.getY());
	 }
	 
	 /*
	  * Class holding all filter values to make "back"-button work multiple times.
	  */
	 private class LastLimits {
		 
		 private double minXlimit, maxXlimit, minYlimit, maxYlimit;
		 private long minZlimit, maxZlimit;
		 
		 public LastLimits(double minX ,double maxX, double minY, double maxY, long minZ, long maxZ) {
			 minXlimit = minX;
			 maxXlimit = maxX;
			 minYlimit = minY;
			 maxYlimit = maxY;
			 minZlimit = minZ;
			 maxZlimit = maxZ;
		 }
		 
		 public double getMinXlimit() { return minXlimit; }
		 public double getMaxXlimit() { return maxXlimit; }
		 public double getMinYlimit() { return minYlimit; }
		 public double getMaxYlimit() { return maxYlimit; }
		 public long getMinZlimit() { return minZlimit; }
		 public long getMaxZlimit() { return maxZlimit; }
	 }
}

