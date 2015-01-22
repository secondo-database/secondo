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


package viewer.spacetimecube;

import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Point2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.net.URL;
import java.util.*;
import gui.idmanager.*;
import javax.imageio.ImageIO;
import project.OSMMercator;
import tools.downloadmanager.ActiveDownload;
import tools.downloadmanager.DownloadEvent;
import tools.downloadmanager.DownloadManager;
import tools.downloadmanager.DownloadObserver;
import tools.downloadmanager.DownloadState;
import tools.Pair;
import viewer.SpaceTimeCubeViewer;

/**
 * Class representing a SpaceTimeCube in a 3D-World.
 * @author Franz Fahrmeier
 *
 */
public class SpaceTimeCube {
	
	private boolean worldcoord; // stores if MPoint added to STC is based on world coordinates
	private Vector<MPoint> mPointsVector; // list of all MPoints maintained in this SpaceTimeCube
	/*
	 * Min and max coordinates of the MPoints, not the SpaceTimeCube itself!
	 * ...in world coordinates
	 */
	private double minXorig, maxXorig, minYorig, maxYorig;
	private long minZorig, maxZorig;
	// labels for axis' min and max values
	private double minXtext, maxXtext, minYtext, maxYtext;
	private String minZtext, maxZtext;
	/*
	 * Min and max coordinates of the MPoints, not the SpaceTimeCube itself!
	 * ...in OSMMercator coordinates
	 */
	private double minXproj, maxXproj, minYproj, maxYproj;
	/*
	 * Min/max values of SpaceTimeCube
	 * either in original coordinates or in OSMMercator coordinates.
	 */
	private double minX, minY, maxX, maxY;
	private long minZ, maxZ;
	/*
	 * Min and max values defined by the user in the GUI.
	 */
	private double minXlimit, maxXlimit, minYlimit, maxYlimit;
	private long minZlimit, maxZlimit;
	private boolean limitSet;
	
	/*
	 * Min and max values without having a filter/limit set by the user.
	 */
	private double minXnoLimit, maxXnoLimit, minYnoLimit, maxYnoLimit;
	private long minZnoLimit, maxZnoLimit;
	
	private OSMMercator osmm;
	private int tileAmount; // amount of tiles to download
	private int downloadsDone; // each tile-download is counted
	private SpaceTimeCubeViewer STCviewer;
	private Vector<Vector> pointArrays; // stores arrays of points (Point3DSTC)
	private int[] indexesAfterFilter; // stores which indexes in mPointsVector are still used after filtering
	
	// shared variables for map download and generation
	private int xCount, yCount;
	private String[] tilePaths;
	private double xySTCproj;
	private LinkedList<Pair<URL, AffineTransform>> urls;
	
	// variables for tile handling
	private static final String PROTOCOL = "http";
	private static final String SERVER = "tile.openstreetmap.org";
	private static final int PORT = 80;
	private static final String DIRECTORY = "/";
	private static final String PREFIX = "";
	private static final int MINZOOMLEVEL = 0;
	private static final int MAXZOOMLEVEL = 18;
	private static final int MAXDOWNLOADS = 2;
	private static final int TILESIZEX = 256;
	private static final int TILESIZEY = 256;
	private static String PATH = "osmDowloads";
	private static int CACHESIZE = 8 * 1024 * 1024; // 8 MB cache
	
	/**
	 * Constructor for SpaceTimeCube.
	 * @param viewer
	 * 		viewer to put the SpaceTimeCube into.
	 */
	public SpaceTimeCube(SpaceTimeCubeViewer viewer) {
		
		worldcoord = true;
		mPointsVector = new Vector<MPoint>();
		osmm = new OSMMercator();
		STCviewer = viewer;
		pointArrays = new Vector<Vector>();
		limitSet = false;
		
	}
	
	/**
	 * @return
	 * 		true if SpaceTimeCube currently deals with world coordinates.
	 */
	public boolean isWorldcoord() { return worldcoord; }
	
	/**
	 * Returns all arrays of points (Point3DSTC).
	 * @return
	 * 		arrays of points as vectors. 
	 */
	public Vector<Vector> getPointArrays() { return pointArrays; }
	
	/**
	 * Removes all MPoints with a given ID from SpaceTimeCube.
	 * @param id
	 * 		ID.
	 */
	public void removeMPoints(ID id) {
		
		Vector<MPoint> tempMPoints = new Vector<MPoint>();
		for (int i=0;i<mPointsVector.size();i++) {
			MPoint tempMP = mPointsVector.get(i);
			if (tempMP.getSecondoId().equals(id)) {
				tempMPoints.add(tempMP);
			}
		}
		for (int i=0;i<tempMPoints.size();i++) {
			mPointsVector.remove(tempMPoints.get(i));
		}
		
		limitSet = false;
		recompute();
	}
	
	/**
	 * Returns the length of the X-/Y-axis
	 * either in world coordinates or in OSMMercator coordinates
	 * depending if the STC is storing only world coordinates or not.
	 * @return
	 * 		length of the X-/Y-axis.
	 */
	public double getXYlength() { 
		double XYlength;
		if (worldcoord) {
			double XYwidth = Math.abs(maxXproj - minXproj);
			double XYheight = Math.abs(maxYproj - minYproj);
			
			if (XYwidth > XYheight) XYlength = XYwidth;
			else XYlength = XYheight;
		}
		else {
			if ((maxXorig-minXorig) > (maxYorig-minYorig)) XYlength = maxXorig-minXorig;
			else XYlength = maxYorig-minYorig;
		}
		
		return XYlength;
	}
	
	/**
	 * Returns the length of the Z-axis in world coordinates.
	 * @return
	 * 		length of the Z-axis in world coordinates.
	 */
	public double getZlength() {
		return (maxZorig-minZorig);
	}
	
	/**
	 * @return
	 * 		min value of X either in original coordinates or in OSMMercator coordinates.
	 */
	public double getMinX() { return minX; }
	/**
	 * @return
	 * 		min value of Y either in original coordinates or in OSMMercator coordinates.
	 */
	public double getMinY() { return minY; }
	/**
	 * @return
	 * 		max value of X either in original coordinates or in OSMMercator coordinates.
	 */
	public double getMaxX() { return maxX; }
	/**
	 * @return
	 * 		max value of Y either in original coordinates or in OSMMercator coordinates.
	 */
	public double getMaxY() { return maxY; }
	/**
	 * @return
	 * 		min value of Z in milliseconds since 01/01/1970.
	 */
	public long getMinZ() { return minZ; }
	/**
	 * @return
	 * 		max value of Z in milliseconds since 01/01/1970.
	 */
	public long getMaxZ() { return maxZ; }
	
	// Methods for publishing actual axis' start and end values.
	public double getMinXtext() { return minXtext; }
	public double getMinYtext() { return minYtext; }
	public String getMinZtext() { return minZtext; }
	public double getMaxXtext() { return maxXtext; }
	public double getMaxYtext() { return maxYtext; }
	public String getMaxZtext() { return maxZtext; }
	
	/*
	 *  Methods for publishing min and max values without limitation due to filtering.
	 *  That means these are the values based on all currently existing MovingPoints.
	 */
	public double getMinXnoLimit() { return minXnoLimit; }
	public double getMinYnoLimit() { return minYnoLimit; }
	public long getMinZnoLimit() { return minZnoLimit; }
	public double getMaxXnoLimit() { return maxXnoLimit; }
	public double getMaxYnoLimit() { return maxYnoLimit; }
	public long getMaxZnoLimit() { return maxZnoLimit; }
	
	/*
	 * Methods for setting limits/filter.
	 */
	public void setMinXlimit(double d) { minXlimit = d; }
	public void setMinYlimit(double d) { minYlimit = d; }
	public void setMaxXlimit(double d) { maxXlimit = d; }
	public void setMaxYlimit(double d) { maxYlimit = d; }
	public void setMinZlimit(long l) { minZlimit = l; }
	public void setMaxZlimit(long l) { maxZlimit = l; }
	
	/*
	 * Methods for getting limits/filter.
	 */
	public double getMinXlimit() { return minXlimit; }
	public double getMinYlimit() { return minYlimit; }
	public double getMaxXlimit() { return maxXlimit; }
	public double getMaxYlimit() { return maxYlimit; }
	public long getMinZlimit() { return minZlimit; }
	public long getMaxZlimit() { return maxZlimit; }
	
	/**
	 * Defines that a filter was set.
	 * @param b
	 * 		true is filter set.
	 */
	public void setLimitSet(boolean b) { limitSet = b; }
	
	/**
	 * @return
	 * 		true if a filter was set.
	 */
	public boolean getLimitSet() { return limitSet; }
	
	/**
	 * Adds a vector of MovingPoints to the SpaceTimeCube.
	 * @param mpoints
	 * 		vector of MPoints
	 */
	public void addMPointsVector(Vector<MPoint> mpoints) {
		for (int i=0;i<mpoints.size();i++) {
			mPointsVector.add(mpoints.get(i));
		}
		
		recompute();
	}
	
	/**
	 * @return
	 * 		Vector including all moving points (MPoint).
	 */
	public Vector<MPoint> getMPointsVector() { return mPointsVector; }
	
	/**
	 * @return
	 * 		Indexes in mPointsVector that are still used after filtering.
	 */
	public int[] getIndexesAfterFilter() { return indexesAfterFilter; }
	
	/**
	 * Computations will be performed like
	 * - borders of the SpaceTimeCube as world coordinates and projected values
	 * - points of the MPoints are stored into a vector
	 * OSMMercator projection is used.
	 */
	public void recompute() {
				
		if (mPointsVector.isEmpty()) worldcoord = true;
		
		if (!limitSet) {
			
			double minXnoLimitTemp=0, maxXnoLimitTemp=0, minYnoLimitTemp=0, maxYnoLimitTemp=0;
			long minZnoLimitTemp=0, maxZnoLimitTemp=0;
			
			for (int i=0;i<mPointsVector.size();i++) {
				MPoint tempMP = mPointsVector.get(i);
				double[] x = tempMP.getXarray().clone();
				
				if (x.length > 0) {
					double[] y = tempMP.getYarray().clone();
					long[] z = tempMP.getMilliSecondsArray().clone();
					
					Arrays.sort(x);
					Arrays.sort(y);
					Arrays.sort(z);
					
					if (i==0) {
						minXnoLimitTemp = x[0];
						maxXnoLimitTemp = x[x.length-1];
						minYnoLimitTemp = y[0];
						maxYnoLimitTemp = y[y.length-1];
						minZnoLimitTemp = z[0];
						maxZnoLimitTemp = z[z.length-1];
					}
					
					if (x[0] < minXnoLimitTemp) minXnoLimitTemp = x[0];
					if (x[x.length-1] > maxXnoLimitTemp) maxXnoLimitTemp = x[x.length-1];
					if (y[0] < minYnoLimitTemp) minYnoLimitTemp = y[0];
					if (y[y.length-1] > maxYnoLimitTemp) maxYnoLimitTemp = y[y.length-1];
					if (z[0] < minZnoLimitTemp) minZnoLimitTemp = z[0];
					if (z[z.length-1] > maxZnoLimitTemp) maxZnoLimitTemp = z[z.length-1];	
					
				}
			}
			
			minXnoLimit = minXnoLimitTemp;
			maxXnoLimit = maxXnoLimitTemp;
			minYnoLimit = minYnoLimitTemp;
			maxYnoLimit = maxYnoLimitTemp;
			minZnoLimit = minZnoLimitTemp;
			maxZnoLimit = maxZnoLimitTemp;
			
			minXlimit = minXnoLimit;
			maxXlimit = maxXnoLimit;
			minYlimit = minYnoLimit;
			maxYlimit = maxYnoLimit;
			minZlimit = minZnoLimit;
			maxZlimit = maxZnoLimit;
		}
		
		int counter = 0;
		int countInd = 0;
		int lastInd = -1;
		int pointCount = 0;
		indexesAfterFilter = new int[mPointsVector.size()];
		pointArrays.clear();
		for (int i=0;i<mPointsVector.size();i++) {
			MPoint tempMP = mPointsVector.get(i);
			double[] x = tempMP.getXarray();
			double[] y = tempMP.getYarray();
			long[] z = tempMP.getMilliSecondsArray();
			String[] t = tempMP.getTimesArray();
			
			Point2D.Double p2d=null;
			Vector<Point3DSTC> points = new Vector<Point3DSTC>();
			double tolerance = 0.000000000001;
			
			for (int a=0;a<x.length;a++) {
				
				if (x[a] >= (minXlimit-tolerance) && x[a] <= (maxXlimit+tolerance) &&
						y[a] >= (minYlimit-tolerance) && y[a] <= (maxYlimit+tolerance) &&
						z[a] >= minZlimit && z[a] <= maxZlimit) {
					
					if (lastInd != i) { countInd++;	}
					indexesAfterFilter[countInd-1] = i;
					lastInd = i;
	
					if (worldcoord) {
						downloadsDone = 0;
						if (x[a]<-180 || x[a] >180 || y[a]>90 || y[a]<-90) {
							worldcoord = false;
							recompute();
							return;
						}
						p2d = new Point2D.Double();
						osmm.project(x[a], y[a], p2d);
					}
					
					if (counter==0) {
						minXorig = x[a];
						maxXorig = x[a];
						minYorig = y[a];
						maxYorig = y[a];
						minZorig = z[a];
						maxZorig = z[a];
						minZtext = t[a];
						maxZtext = t[a];
						if (worldcoord) {
							minXproj = p2d.x;
							maxXproj = p2d.x;
							minYproj = p2d.y;
							maxYproj = p2d.y;
						}
					}
					
					if (x[a] < minXorig) { minXorig = x[a];	}
					if (x[a] > maxXorig) { maxXorig = x[a];	}
					if (y[a] < minYorig) { minYorig = y[a];	}
					if (y[a] > maxYorig) { maxYorig = y[a];	}
					if (z[a] < minZorig) {
						minZorig = z[a];
						minZtext = t[a];
					}
					if (z[a] > maxZorig) {
						maxZorig = z[a];
						maxZtext = t[a];
					}
					
					if (worldcoord) {					
						if (p2d.x < minXproj) { minXproj = p2d.x;	}
						if (p2d.x > maxXproj) { maxXproj = p2d.x;	}
						if (p2d.y < minYproj) { minYproj = p2d.y;	}
						if (p2d.y > maxYproj) { maxYproj = p2d.y;	}
						
						points.add(new Point3DSTC(p2d.x,p2d.y,z[a],tempMP.getSecondoId()));
					}
					else {						
						points.add(new Point3DSTC(x[a],y[a],z[a],tempMP.getSecondoId()));	
					}
					counter++; // count up if a filter was met
				}
			}
			if (points.size()>0) {
				pointArrays.add(points);
				pointCount += points.size();
			}
		}
		
		System.out.println("Maintained points in STC: "+pointCount);
				
		/*
		 * Some additional space at X and Y axis needs to be added
		 * so that the map will not immediately start with a trajectory.
		 * Background: Otherwise it's hard to determine where the trajectory starts.
		 */
		double factorAdd = 0.05;
		double lengthXorig = maxXorig - minXorig;
		double lengthYorig = maxYorig - minYorig;
		double lengthXproj = maxXproj - minXproj;
		double lengthYproj = maxYproj - minYproj;
		minXorig -= (lengthXorig*factorAdd);
		maxXorig += (lengthXorig*factorAdd);
		minYorig -= (lengthYorig*factorAdd);
		maxYorig += (lengthYorig*factorAdd);
		minXproj -= (lengthXproj*factorAdd);
		maxXproj += (lengthXproj*factorAdd);
		minYproj -= (lengthYproj*factorAdd);
		maxYproj += (lengthYproj*factorAdd);
		
		if (worldcoord) {
			minX = minXproj;
			minY = minYproj;
			maxX = maxXproj;
			maxY = maxYproj;
		}
		else {
			minX = minXorig;
			minY = minYorig;
			maxX = maxXorig;
			maxY = maxYorig;
		}			
			
		Point2D.Double p1 = new Point2D.Double();
		if (worldcoord) osmm.getOrig(minX,minY,p1);
		else p1.setLocation(minX,minY);
		Point2D.Double p2 = new Point2D.Double();
		if (worldcoord) osmm.getOrig(maxX,maxY,p2);
		else p2.setLocation(maxX,maxY);
		minXlimit = p1.getX();
		maxXlimit = p2.getX();
		minYlimit = p1.getY();
		maxYlimit = p2.getY();
		
		minXtext = minXorig;
		minYtext = minYorig;
		maxXtext = maxXorig;
		maxYtext = maxYorig;
		minZ = minZorig;
		maxZ = maxZorig;
		
		minZlimit = minZ;
		maxZlimit = maxZ;
	}
	
	/**
	 * Downloads all necessary tiles from OSM tiles interface
	 * based on the currently maintained MPoints in the STC.
	 * @return
	 * 		true if all tiles have been downloaded or STC is not based on world coordinates
	 * 		false if downloads are going on
	 */
	public boolean downloadMap() {
		/*
		 * 	Bounding box of the SpaceTimeCube in world coordinates is calculated (clipRect).
		 * 	Tile URLs are calculated and tiles downloaded (as per clipRect).
		 */
		if (worldcoord && mPointsVector.size()>0) {			
			DownloadManager downloadManager;
			
			double widthSTCproj = Math.abs(maxXproj - minXproj);
			double heightSTCproj = Math.abs(maxYproj - minYproj);
			
			if (widthSTCproj > heightSTCproj) xySTCproj = widthSTCproj;
			else xySTCproj = heightSTCproj;
			
			Point2D.Double topleftSTCorig = new Point2D.Double();
			Point2D.Double bottomrightSTCorig = new Point2D.Double();
			osmm.getOrig(minXproj, minYproj+xySTCproj, topleftSTCorig);
			osmm.getOrig(minXproj+xySTCproj, minYproj, bottomrightSTCorig);
			
			double widthSTCorig = Math.abs(bottomrightSTCorig.x - topleftSTCorig.x);
			double heightSTCorig = Math.abs(topleftSTCorig.y - bottomrightSTCorig.y);
			maxXtext = minXtext + widthSTCorig;
			maxYtext = minYtext + heightSTCorig;
			
			// bounding box of the SpaceTimeCube in world coordinates
			Rectangle2D.Double clipRect = new Rectangle2D.Double(topleftSTCorig.x, topleftSTCorig.y-heightSTCorig
								, widthSTCorig, heightSTCorig);
			
			Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
			int DIM_X = (int)dim.getWidth();
			int DIM_Y = (int)dim.getHeight();
			URL baseUrl=null;
			try {
				baseUrl = new URL(PROTOCOL, SERVER, PORT, DIRECTORY);
			} catch (Exception e) {
				e.printStackTrace();
			}
			String prefix = PREFIX;
			
			StaticOSMMapper osmmapper = new StaticOSMMapper(TILESIZEX, TILESIZEY, DIM_X, DIM_Y,
					MINZOOMLEVEL, MAXZOOMLEVEL, baseUrl, prefix);
			
			try {
				downloadManager = new DownloadManager(new File(PATH), MAXDOWNLOADS, false);
			} catch (Exception e) {
				System.err.println("Problem in initiating download manager");
				downloadManager = null;
			}
			downloadManager.setConnectTimeout(5000);
			downloadManager.setReadTimeout(5000);
			
			DownloadObserver observer = new DownloadObserver() {
				public void downloadStateChanged(DownloadEvent evt) {
					SpaceTimeCube.this.downloadStateChanged(evt);
				}
			};
			
			urls = osmmapper.computeURLs((Rectangle2D.Double) clipRect);
					
			/*
			 *  Check if amount of tile rows and columns is identical.
			 *  If not a row/column needs to be added at the end of urls
			 *  to get a quadratic image finally.
			 */
			int z=0, x0=0, y0=0, x=0, y=0, counter;
			
			do {
				counter=0;
				Iterator<Pair<URL, AffineTransform>> it2 = urls.iterator();

				while (it2.hasNext()) {
					URL url = it2.next().first();			
					String temp = url.getPath();
					temp = temp.substring(1, temp.lastIndexOf("."));
					String[] s = temp.split("\\/");
					z = Integer.parseInt(s[0]);
		 			x = Integer.parseInt(s[1]);
		 			y = Integer.parseInt(s[2]);
		 			if (counter==0) {
		 				x0=x;
		 				y0=y;
		 			}
		 			counter++;
				}
				xCount = x-x0+1;
				yCount = y-y0+1;
				if (xCount>yCount) {
					Rectangle2D.Double rec = getBBoxForTile(x,y+1,z);
					clipRect.height = clipRect.height + rec.height;
					urls = osmmapper.computeURLs((Rectangle2D.Double) clipRect);
				}
				if (yCount>xCount) {
					Rectangle2D.Double rec = getBBoxForTile(x+1,y,z);
					clipRect.width = clipRect.width + rec.width;
					urls = osmmapper.computeURLs((Rectangle2D.Double) clipRect);
				}
			} while (xCount != yCount);
			
			Iterator<Pair<URL, AffineTransform>> it = urls.iterator();
			
			tileAmount = urls.size();
			downloadsDone = 0;
			while (it.hasNext()) {
				URL url = it.next().first();
				File f = downloadManager.getURL(url, observer);
				if (f == null) STCviewer.showLoadingDialog(true);
				else if (f != null) { // url already downloaded
					downloadsDone++;
				}
				if (downloadsDone == tileAmount) {
					STCviewer.showLoadingDialog(false);
					return true;
				}
			}
			return false; // downloads are going on
		}
		else return true; // STC is not based on world coordinates
	}
	
	/**
	 * Generates an image/map cropped according to the size of the SpaceTimeCube.
	 * Steps:
	 * 	- bounding box of all tiles in world coordinates is calculated (bboxTilesOrig)
	 * 	- BufferedImage is built out of single tiles (result)
	 * 	- Built image is cropped to the size of the SpaceTimeCube
	 *  @return
	 *  	map as BufferedImage.
	 */
	public BufferedImage generateMap() {
		if (worldcoord) {
			
			Iterator<Pair<URL, AffineTransform>> it = urls.iterator();
			double xImage=0, yImage=0;
			int counter=0;
			int x=0, y=0, z=0;
			double[] heights = new double[urls.size()];
			double[] widths = new double[urls.size()];
			tilePaths = new String[urls.size()];
			while (it.hasNext()) {
				URL url = it.next().first();		
				String temp = url.getPath();
				tilePaths[counter] = temp;
				temp = temp.substring(1, temp.lastIndexOf("."));
				String[] s = temp.split("\\/");
				z = Integer.parseInt(s[0]);
	 			x = Integer.parseInt(s[1]);
	 			y = Integer.parseInt(s[2]);
	 			Rectangle2D.Double bboxTile = getBBoxForTile(x, y, z);
	 			
	 			if (counter==0) {
	 				xImage = bboxTile.x;
	 				yImage = bboxTile.y;
	 			} else {
	 				if (bboxTile.x < xImage) {
	 					xImage = bboxTile.x;
	 				}
	 				if (bboxTile.y > yImage) {
	 					yImage = bboxTile.y;
	 				}
	 			}
	 			widths[counter] = bboxTile.width;
	 			heights[counter] = bboxTile.height;
	 			
	 			counter++;
			}
			
			double width=0;
			double height=0;
			
			// width of a tile is always same (in world coordinates)!
			if (widths.length > 0) width = widths[0] * xCount;
			
			// height calculation is only done for the first column (which is enough)
			for (int i=0; i<yCount; i++) {
				height+= heights[i];
			}
			
			if (heights.length > 0) yImage+=heights[0];
			
			int imageXY = xCount * TILESIZEX;
			
			// bounding box of all tiles in world coordinates
			Rectangle2D.Double bboxTilesOrig = new Rectangle2D.Double(xImage, yImage, width, height);
			
			Point2D.Double topleftTilesProj = new Point2D.Double();
			Point2D.Double bottomrightTilesProj = new Point2D.Double();
			osmm.project(bboxTilesOrig.x, bboxTilesOrig.y, topleftTilesProj);
			osmm.project(bboxTilesOrig.x+bboxTilesOrig.width, bboxTilesOrig.y-bboxTilesOrig.height, bottomrightTilesProj);
			
			BufferedImage result = new BufferedImage(imageXY, imageXY, BufferedImage.TYPE_INT_RGB);
			Graphics2D g = result.createGraphics();
						
			int xPos=0;
			int yPos=0;
			for (int i=0;i<tilePaths.length;i++) {
				BufferedImage bi=null;
		    	try {
		    		bi = ImageIO.read(new File("./"+PATH+"/"+PROTOCOL+"/"+SERVER+"/"+PORT+tilePaths[i]));
		    	} catch (Exception e) {
		    		e.printStackTrace();
		    	}
		    	
		    	g.drawImage(bi, null, xPos, yPos);
		    	
		        yPos += TILESIZEY;
		        if(yPos >= result.getHeight()){
		            yPos = 0;
		            xPos += TILESIZEX;
		        }
			}
			
			double xyImageProj = Math.abs(bottomrightTilesProj.x - topleftTilesProj.x);
			
			double offsetLeft = Math.abs(minXproj - topleftTilesProj.x);
			double offsetBottom = Math.abs(minYproj - bottomrightTilesProj.y);
			double subimgXY;
			
			offsetLeft = imageXY/xyImageProj*offsetLeft;
			offsetBottom = imageXY/xyImageProj*offsetBottom;
			subimgXY = imageXY/xyImageProj*xySTCproj;
			
			/*
			 * We start drawing lines/mpoints from x,y,z=0.
			 * That means offsetTop has to be dependent on offsetBottom.
			 */
			double offsetTop = imageXY - subimgXY - offsetBottom;
			
			result = result.getSubimage((int)Math.round(offsetLeft), (int)Math.round(offsetTop), 
					(int)Math.round(subimgXY), (int)Math.round(subimgXY));
			
			return result;
		}
		return null;
	}
	
	/**
	 * Removes all objects(MPoints) from SpaceTimeCube.
	 */
	public void clear() {
		mPointsVector.clear();
		pointArrays.clear();
		limitSet = false;
		recompute();
	}
	
	/**
	 * Callback method. Called when a state of a pending download is changed.
	 * 
	 * @param evt
	 *            Event to handle.
	 **/
	private void downloadStateChanged(DownloadEvent evt) {
		DownloadState state = evt.getState();
		ActiveDownload ad = evt.getSource();
		if (state == DownloadState.DONE) {
			downloadsDone++;			
			if (downloadsDone == tileAmount) {
				STCviewer.showLoadingDialog(false);
				STCviewer.recompute();
			}
		}
		else if (state == DownloadState.BROKEN) {
			System.out.println("Download broken: " + ad.getURL());
			System.out.println("Exception: " + evt.getException());
			if (evt.getException() != null) {
				evt.getException().printStackTrace();
			}
		}
	}	
	
	/**
	 * compute the bounding box covered by a specified tile in world coordinates.
	 * 
	 * @param x
	 *            X index of a tile
	 * @param y
	 *            Y index of a tile
	 * @param z
	 *            zoom level
	 * @return The images's MBR in world coordinates
	 **/
	private Rectangle2D.Double getBBoxForTile(int x, int y, int zoom) {
		double north = tile2lat(y, zoom);
		double south = tile2lat(y + 1, zoom);
		double west = tile2lon(x, zoom);
		double east = tile2lon(x + 1, zoom);
		return new Rectangle2D.Double(west, south, east - west, north - south);
	}

	/**
	 * computes the western boundary of a specified tile in world coordinates.
	 * 
	 * @param x
	 *            The map tile's X-index
	 * @param z
	 *            The zoom level
	 * @return The longitude of the western boundary of a map tile
	 **/
	private static double tile2lon(int x, int z) {
		return x / Math.pow(2.0, z) * 360.0 - 180;
	}

	/**
	 * computes the northern boundary of a specified tile in world coordinates.
	 * 
	 * @param x
	 *            The map tile's Y-index
	 * @param z
	 *            The zoom level
	 * @return The latitude of the northern boundary of a map tile
	 **/
	private static double tile2lat(int y, int z) {
		double n = Math.PI - (2.0 * Math.PI * y) / Math.pow(2.0, z);
		return Math.toDegrees(Math.atan(Math.sinh(n)));
	}
	
}
