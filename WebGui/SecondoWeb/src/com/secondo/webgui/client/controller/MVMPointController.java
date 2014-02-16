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

package com.secondo.webgui.client.controller;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.LonLat;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.Style;
import org.gwtopenmaps.openlayers.client.feature.VectorFeature;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.Vector;
import com.google.gwt.i18n.client.DateTimeFormat;
import com.google.gwt.user.client.Timer;
import com.google.gwt.user.client.ui.TextBox;
import com.secondo.webgui.client.mainview.TimeSlider;
import com.secondo.webgui.client.mainview.ToolBox;
import com.secondo.webgui.shared.model.MPoint;

/**
 * This class is a controller for the datatype moving point, to display moving points in
 * the map view.
 * 
 * @author Kristina Steiger
 * 
 **/
public class MVMPointController {
    
    /**Map with IDs of mpoints as keys and the layer for mpoints as values*/
    private HashMap<Number, Vector> mpointMap = new HashMap<Number, Vector>(); 
    
    /**Array with layer features of mpoints because only features can be moved*/
    private ArrayList<VectorFeature> mpointFeatures = new ArrayList<VectorFeature>();
    
    //mpoint arrays
    private ArrayList<LonLat> mpointPath = new ArrayList<LonLat>();
    private ArrayList<ArrayList<LonLat>> mpointPathArray = new ArrayList<ArrayList<LonLat>>(); 
    private ArrayList<MPoint> mpointArray = new ArrayList<MPoint>();
    
	//elements for the animation of moving points
    private ArrayList<Date> timeBounds = new ArrayList<Date>();
    private int maxTime = 0;
    private Date currentTime = new Date();
	private Timer timeTimer;
	private int timeCounter = 0;
	private ArrayList<Timer> mpointTimerList = new ArrayList<Timer>();
	private boolean timerIsRunning = false;
	private int speed = 50;
	
	private ToolBox toolbox;
	
	public MVMPointController(){
	}
	
    /**Checks if the given number is a geographic latitude
     * 
     * @param lat The latitude value to be checked
     * @return Returns true if the value is a geographic latitude
     * */
    public boolean isLatitude(double lat){
   	 
   	 //range lat => -90 +90
   	 if(lat < 90 && lat > -90){
   		 return true; 		 
   	 }	 
   	 return false;
    }
    
    /**Checks if the given number is a geographic longitude
     * 
     * @param lng The longitude value to be checked
     * @return Returns true if the value is a geographic longitude
     * */
    public boolean isLongitude(double lng){ 	 
   	 
   	 //range lng => -180 +180
   	 if(lng < 180 && lng > -180){
   		 return  true;
   	 } 	 
   	 return false;
    }
    
    /**Creates a lonlat element from the given values and adds it to the mpatharray
     * 
     * @param lat The Latitude of the point
     * @param lng The Longitude of the point
     * @param bounds The Bounds object
     * */
    public void addLonLat(double lat, double lng, Bounds boundsAll, Bounds boundsLast){

    	if(isLatitude(lat) && isLongitude(lng)){
    		
    	 LonLat lonLat = new LonLat(lng, lat);
    	 lonLat.transform("EPSG:4326", "EPSG:900913");

          mpointPath.add(lonLat);       
          boundsAll.extend(lonLat);
          boundsLast.extend(lonLat);
    	}	
    }
    
    /**Adds the LonLat path from the patharray to the mpointarray
     * 
     * @param key The ID of the moving point to be added
     * */
    public void addMPoint(int key){	
        
       // Create the vector layer
        Vector mpointLayer = new Vector("Moving Point Overlay");
        mpointLayer.setIsBaseLayer(false);
        mpointLayer.setDisplayInLayerSwitcher(false);
        
    	if(!mpointPath.isEmpty()){
        
        //Create a point feature with the given style and the first point of the array
        Point startpoint = new Point(mpointPath.get(0).lon(), mpointPath.get(0).lat());
        Style pointStyle = new Style();
        pointStyle.setFillColor("red");
        pointStyle.setStrokeColor("blue");  
        pointStyle.setStrokeWidth(2);
        pointStyle.setPointRadius(6);
        pointStyle.setFillOpacity(1);
        VectorFeature pointFeature = new VectorFeature(startpoint, pointStyle); 
        
        mpointFeatures.add(pointFeature); 
        mpointLayer.addFeature(pointFeature);
        
        mpointMap.put(key, mpointLayer);
        
        //Add path to path array
    	ArrayList<LonLat> path = new ArrayList<LonLat>();
    	
    		for(LonLat lonlat : mpointPath){
       	     path.add(lonlat);	
       	     }
       	     mpointPathArray.add(path);
    	}  	
    }
    
    /**Adds the first moving point layer to the map and zooms the map to bounds
     * 
     * @param map The Map object
     * @param bounds The bounds object
     * */
    public void drawFirstMovingPoint(Map map, Bounds bounds){
    	
    	//if the timer is still running, stop it
    	if(timerIsRunning == true){
    		timeTimer.cancel();
    	}  	
    	
    	//add first mpoint layer with starting point to the map
       if(!mpointMap.isEmpty()){

    	   int idOfFirstPoint = mpointArray.get(0).getId();
    	   map.addLayer(mpointMap.get(idOfFirstPoint));
        
           map.zoomToExtent(bounds); 
           if(map.getZoom() > 10){ 
    			map.zoomTo(10);
    		}
        }
    }
    
    /**Shows the given moving point in the view
     * 
     * @param mpoint The moving point to be shown on the map
     * */
    public void showMPointObject(MPoint mpoint){ 	
   	 mpointMap.get(mpoint.getId()).setIsVisible(true);  
    }
    
    /**Hides the given moving point from the view
     * 
     * @param mpoint The moving point to be hidden from the map
     * */
    public void hideMPointObject(MPoint mpoint){
   	 mpointMap.get(mpoint.getId()).setIsVisible(false); 
    }
    
	/** Changes the color of the given mpoint id to the given color 
	 * 
	 * @param mpointId The ID of the moving point
	 * @param color The new color of the moving point
	 * */
	public void changeMPointColor(int mpointId, String color) {

		VectorFeature[] features = mpointMap.get(mpointId).getFeatures();
		for(VectorFeature feature : features){
			feature.getStyle().setFillColor(color);
		}		
		mpointMap.get(mpointId).redraw();
	}
    
    
    /**Deletes all lonlats from the mpatharray*/
    public void deleteAllLonLats(){
    	mpointPath.clear();
    }
    
    /**Deletes all mpoints from the mpointarray*/
    public void deleteAllMPoints(){
    	mpointPathArray.clear();
    	mpointFeatures.clear();
    	mpointArray.clear();
    }
    
    /**Animates all moving points from the mpointarray
     * 
     *  @param tb The toolbox element to communicate with the animation panel
	 *  @param map The map object
	 *  */ 
    public void animateMovingPoints(ToolBox tb, Map map){      
    	
    	this.toolbox = tb;
    	
    	if(mpointArray.isEmpty()){
    		//do nothing
    	}
    	else{
    		
    	//change play to pause icon
        toolbox.getAnimationPanel().remove(0);
    	toolbox.getAnimationPanel().insert(toolbox.getPausepanel(), 0);
    	
    	//sort dates in ascending order
    	Collections.sort(timeBounds); 
        
    	maxTime = (timeBounds.size())-1;
    	final TextBox cb = toolbox.getTimeCounter();
    	final TimeSlider ts = toolbox.getTimeSlider();
    	final Map mapIntern = map;
    	
    	//set time ticks to the timeslider
    	String minTick = DateTimeFormat.getFormat(DateTimeFormat.PredefinedFormat.DATE_TIME_SHORT).format(timeBounds.get(0));
    	String maxTick = DateTimeFormat.getFormat(DateTimeFormat.PredefinedFormat.DATE_TIME_SHORT).format(timeBounds.get(maxTime));
    	ts.setTicks(minTick, maxTick); 	
    	ts.setNumberOfTimeValues(maxTime);
        
        System.out.println("####animateMovingPoints is called, timebounds size:" + timeBounds.size());
        
      //if during pausing of the animation any timers for moving points have been interrupted they need to be started again
        if(!mpointTimerList.isEmpty()){
        	for(Timer timer : mpointTimerList){
        		timer.scheduleRepeating(speed);
        	}
        }
        
      //GWT timer to schedule animation
   	    timeTimer = new Timer()  {   		 	 
   	    	
            @Override
            public void run() {

            	timerIsRunning = true;
            	
            	//add current time to timerBox in toolbar
            	currentTime = timeBounds.get(timeCounter);           	
            	cb.setText(DateTimeFormat.getFormat(DateTimeFormat.PredefinedFormat.DATE_TIME_MEDIUM).format(currentTime));
            	ts.moveSlider(timeCounter, DateTimeFormat.getFormat(DateTimeFormat.PredefinedFormat.DATE_TIME_MEDIUM).format(currentTime));
            	
            //stop timer if time bounds are reached
           	 if (timeCounter == maxTime) {
                    cancel();
                    timerIsRunning = false;
                    timeCounter = 0;
                    toolbox.resetAnimationPanel();
  	        	    mpointTimerList.clear();
                    return;
             }
           	 
           //search for mpoints which start with the current time and start the animation for them
           	 for(MPoint mpoint : mpointArray){
           		 
           		int indexMpoint = mpointArray.indexOf(mpoint);
           		 
           		 if(mpoint.getTime().get(0).getTimeA().equals(timeBounds.get(timeCounter))){
           			 animateMovingPoint(mpoint, indexMpoint, mapIntern);
           		 }
           	 }
           	timeCounter++;
            }
        };
        timeTimer.scheduleRepeating(speed);
    	}
    }
    
    /**Moves the point along the path in the patharray
     * 
     * @param mpoint The moving point to be animated
     * @param index The position of the moving point in the resultlist 
     * @param map The map object
     * */
    public void animateMovingPoint(MPoint mpoint, int index, Map map){
    	
    	//add mpoint layer to the map
        map.addLayer(mpointMap.get(mpoint.getId())); 	
    	
    	final int size = (mpoint.getPath().size())-1;
    	final int i = index;
    	final int mpointID = mpoint.getId();
    	final Map mapIntern = map;
        
        //start timer to schedule animation
   	    Timer timer = new Timer() {  
   	    	
   	    	private int counter = 0;
   	    	
            @Override
            public void run() {
 
           	 if (counter == size) {
                    cancel();
                    mapIntern.removeLayer(mpointMap.get(mpointID));
                    return;
                  }
           	mpointFeatures.get(i).move(mpointPathArray.get(i).get(counter)); //move point
           	counter++;
            }
        };
        timer.scheduleRepeating(speed);        
        mpointTimerList.add(timer);
    }
    
    /**Speeds up the animation at the current position*/
    public void speedUpMovingPoint(){
    	
    	//double the speed if its not faster than 1 ms per step
    	if(timerIsRunning){
    	if(speed > 1){
        	speed = speed/2; 
    	}
    	
    	timeTimer.scheduleRepeating(speed);
    	for(Timer timer : mpointTimerList){
    		timer.scheduleRepeating(speed);
    	}
    	}
    }
    
    /**Reduces the speed of the animation at the current position*/
    public void reduceSpeedOfMovingPoint(){
    	
    	//reduces speed to half if its not slower than 2000 ms per step
    	if(timerIsRunning){
    	if(speed < 2000){
        	speed = speed*2;
    	}
    	
    	timeTimer.scheduleRepeating(speed);
    	for(Timer timer : mpointTimerList){
    		timer.scheduleRepeating(speed);
    	}
    	}
    }
    
    /**Pauses the animation at the current position*/
    public void pauseMovingPoint(){
    	
    	if(timerIsRunning){
    	
    	timeTimer.cancel();
    	for(Timer timer : mpointTimerList){
    		timer.cancel();
    	}
    	}
    }
    
    /**Resumes the animation at the current position*/
    public void resumeMovingPoint(){
    	
    	if(timerIsRunning){
    	
    	timeTimer.scheduleRepeating(speed);
    	for(Timer timer : mpointTimerList){
    		timer.scheduleRepeating(speed);
    	}
    	}
    }
    
    /**Stops all timers for animations*/
    public void stopAllAnimations(){
    	if (timerIsRunning){
        	timeTimer.cancel();
        	toolbox.resetAnimationPanel();
    	}
    	if(!mpointTimerList.isEmpty()){
    		for(Timer timer : mpointTimerList){
        		timer.cancel();
        	}
        	mpointTimerList.clear();
    	} 	
    	timeCounter = 0;
    	timerIsRunning = false;
    }
    
    /**Deletes all data from the timebounds array*/
    public void resetTimeBounds(){
    	timeBounds.clear();
    }

    /**Returns the list of all date objects
     * 
     * @return The list of all date objects
     * */
	public ArrayList<Date> getTimeBounds() {
		return timeBounds;
	}

	/**Returns the list of all moving point objects
     * 
     * @return The list of all moving point objects
     * */
	public ArrayList<MPoint> getMpointArray() {
		return mpointArray;
	}

	/**Returns the list of all timer objects
     * 
     * @return The list of all timer objects
     * */
	public ArrayList<Timer> getMpointTimerList() {
		return mpointTimerList;
	}	
}
