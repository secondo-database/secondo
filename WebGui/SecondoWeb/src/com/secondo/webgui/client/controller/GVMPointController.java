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
import com.google.gwt.i18n.client.DateTimeFormat;
import com.google.gwt.user.client.Timer;
import com.google.gwt.user.client.ui.TextBox;
import com.secondo.webgui.client.mainview.TimeSlider;
import com.secondo.webgui.client.mainview.ToolBox;
import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MPoint;

/**
*  This class is a controller for the datatype moving point, to display moving points in the graphical view.
*  It defines JSNI Functions that are defined in the JavaScript-File d3-geodata.js.
*  It also includes methods with timers for the animation of moving points.
*  
*  @author Kristina Steiger
*  
**/
public class GVMPointController {
	
    /**Contains all moving points from last secondo queries*/
    private ArrayList<MPoint> mpointArray = new ArrayList<MPoint>();
    
	//elements for animation of moving points
    private ArrayList<Date> timeBounds = new ArrayList<Date>();
    private int maxTime = 0;
	private Timer timeTimer;
	private Date currentTime = new Date();
	private int timeCounter = 0;
	private ArrayList<Timer> mpointTimerList = new ArrayList<Timer>();
	private boolean timerIsRunning = false;
	private int speed = 50;
	private ToolBox toolbox;
	
	public GVMPointController(){
	}
	
	/**Draws the first moving point of the array to the graphical view*/
	public void drawFirstMovingPoint(){
		
		drawMovingPoint(000, 0, mpointArray.get(0).getColor());	
	}
	
	
	/**Animates all points of the mpointArray
	 * 
	 *  @param tb The toolbox element to communicate with the animation panel
	 */ 
    public void animateMovingPoints(ToolBox tb){ 
    	
    	this.toolbox = tb;
    	
    	if(mpointArray.isEmpty()){
    		//do nothing
    	}
    	else{
    		
    	//change play to pause icon
    	toolbox.getAnimationPanel().remove(0);
		toolbox.getAnimationPanel().insert(toolbox.getPausepanel(), 0);
    	
    	//removes the default moving point
    	removeMovingPoint(000); 

    	//sort dates in ascending order
    	Collections.sort(timeBounds); 
        
        maxTime = (timeBounds.size())-1;
        final TextBox cb = toolbox.getTimeCounter();
        final TimeSlider ts = toolbox.getTimeSlider();
    	
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
        if(timerIsRunning){
        	
        	timeTimer.scheduleRepeating(speed);
        }
        else{
        
       //GWT timer to schedule animation of moving points
   	    timeTimer = new Timer()  {   		 	 
   	    	
            @Override
            public void run() {

            	timerIsRunning = true;
            	
            	//add current time to the timeslider in the toolbox
            	currentTime = timeBounds.get(timeCounter);           	
            	cb.setText(DateTimeFormat.getFormat(DateTimeFormat.PredefinedFormat.DATE_TIME_MEDIUM).format(currentTime));
            	ts.moveSlider(timeCounter, DateTimeFormat.getFormat(DateTimeFormat.PredefinedFormat.DATE_TIME_MEDIUM).format(currentTime));
            	
            //stop timer if time bounds are reached
           	 if (timeCounter== maxTime) {
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
           			 animateMovingPoint(mpoint, indexMpoint);
           		 }
           	 }
           	timeCounter++;
            }
        };
        timeTimer.scheduleRepeating(speed);      
        }
    	}
    }
    
    /**Moves the point along the path in the patharray
     * 
     * @param mpoint The moving point to be animated
     * @param index The position of the moving point in the resultlist of the d3-dataset of mpoints
     * */
    public void animateMovingPoint(MPoint mpoint, int index){
    	
    	removeMovingPoint(mpointArray.get(index).getId());
    	
    	//draw mpoint and its path to the view
       drawMovingPoint(mpoint.getId(), index, mpoint.getColor());
    	 
    	final int size = mpoint.getPath().size();
    	final int mpointID = mpoint.getId();
    	final ArrayList<Line> path = mpoint.getPath();
        
        //start timer to schedule animation
   	    Timer timer = new Timer() {  
   	    	
   	    	private int counter = 0;
   	    	
            @Override
            public void run() {
 
             //Stop timer
           	 if (counter == size) {
                    cancel();
                    removeMovingPoint(mpointID);
                    return;
             }
           	 
           	//move point to the given position
           	movePoint(mpointID, path.get(counter).getPointA().getX(), path.get(counter).getPointA().getY());
           	counter++;
            }
        };
        mpointTimerList.add(timer);
        timer.scheduleRepeating(speed);        
    }
    
    /**Speeds up the animation at the current position*/
    public void speedUpMovingPoint(){
    	
    	if(timerIsRunning){

    	//double the speed if its not faster than 1 ms per step
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
    	
    	if(timerIsRunning){
    	
    	//reduces speed to half if its not slower than 2000 ms per step
    	if(speed < 2000){
        	speed = speed*2; 
    	}
    	
    	timeTimer.scheduleRepeating(speed);
    	for(Timer timer : mpointTimerList){
    		timer.scheduleRepeating(speed);
    	}
    	}
    }
    
    /**Stops the animation at the current position*/
    public void pauseMovingPoint(){
    	
    	if(timerIsRunning){
    	
    	timeTimer.cancel();
    	for(Timer timer : mpointTimerList){
    		timer.cancel();
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
    	
    	removeAllMovingPoints();
    }   
    
    /**Draw the given moving point to the view
     * 
     * @param mpoint The moving point to be shown
     * */
    public void showMovingPoint(MPoint mpoint){
    	int indexMPoint = mpointArray.indexOf(mpoint);
    	
    	drawMovingPoint(mpoint.getId(), indexMPoint, mpoint.getColor());
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
	
	/**Removes all moving points from the view*/
	public void removeAllMovingPoints(){
		
		//removes the default moving point
    	removeMovingPoint(000); 
		
		for(MPoint mpoint : mpointArray){
			removeMovingPoint(mpoint.getId());
		}
		
	}
    
/*########### JSNI JavaScript Functions #########
*************************************************/
	
	/** Check if any mpoints are in the array*/
	public native boolean hasMPoints()/*-{		
		return $wnd.hasMPoints();
			}-*/;
	
	/** Add a point to the dataset of path mpoints*/
	public native void addPointToMPointPath(double x, double y)/*-{
		$wnd.addPointToMPointPath(x, y);
			}-*/;
	
	/** Add a mpointpath to the dataset of mpoint paths*/
	public native void addMPoint()/*-{
		$wnd.addMPoint();
			}-*/;
	
	/** Get the data from the dataset, scale the paths to the size of the svg and draw them*/
	public native void drawMovingPoint(int id, int index, String color)/*-{		
		$wnd.drawMovingPoint(id, index, color);
			}-*/;	
	
	/** Start the animation of the moving pointe smooth along the path in the patharray*/
	public native void animateMovingPointAlongPath(int id, int index, String color)/*-{		
		$wnd.animateMovingPoint(id, index, color);
			}-*/;
	
	/**Moves the point to the given position*/
	public native void movePoint(int id, double x, double y)/*-{		
		$wnd.movePoint(id, x, y);
			}-*/;
	
	/**Removes to given moving point from the view*/
	public native void removeMovingPoint(int id)/*-{		
		$wnd.removeMovingPoint(id);
			}-*/;
	
	/**Change the color of the moving point with the given id*/
	public native void changeMPointColor(int id, String color)/*-{	
		$wnd.changeMPointColor(id, color);
			}-*/;
	
	/** Delete all data from the dataset of path mpoints */
	public native void deletePathOfMPoint()/*-{
		$wnd.deletePathOfMPoint();
			}-*/;		
	
	/** Delete all data from the dataset of mpoint paths*/
	public native void deleteAllMPointPaths()/*-{
		$wnd.deleteAllMPointPaths();
			}-*/;

}
