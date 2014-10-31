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

package com.secondo.webgui.server.controller;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import sj.lang.ListExpr;

import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MLabel;
import com.secondo.webgui.shared.model.MPoint;
import com.secondo.webgui.shared.model.Point;
import com.secondo.webgui.shared.model.Polygon;
import com.secondo.webgui.shared.model.Polyline;
import com.secondo.webgui.shared.model.TimeInterval;

/**
*  This class represents a controller class which gets the resultlist from a secondo query, searches for implemented datatypes 
*  and creates objects for all datatypes that have been found. All objects are put into a new resultlist of datatypes.
*  
*  @author Kristina Steiger
*  
*  
**/
public class DataTypeConstructor{

	/**The new resultlist containing objects for implemented datatypes*/
	private ArrayList<DataType> resultTypeList = new ArrayList<DataType>();
	
	/**The object counter, which creates a unique id for each object*/
	private int objectCounter = 1;

	public DataTypeConstructor() {
	}

	/**Checks the given listexpression list for implemented datatypes
	 * 
	 * @param le The secondo result in nested list format
	 * @return The datatype that has been found in the list or "unknown type"
	 * */
	public String getDataType(ListExpr le) {

		String dataType = "unknown type";
		
		this.resultTypeList.clear();

		// all general inquiries
		if (le.first().atomType() == ListExpr.SYMBOL_ATOM
				&& le.first().symbolValue().equals("inquiry")) {
			dataType = "inquiry";
		}

		// type is point
		if (le.first().atomType() == ListExpr.SYMBOL_ATOM
				&& le.first().symbolValue().equals("point")) {
			dataType = "point";
			resultTypeList.add(addPoint(le));
		}
		
		// type is line
		if (le.first().atomType() == ListExpr.SYMBOL_ATOM
				&& le.first().symbolValue().equals("line")) {
			dataType = "line";
			this.analyzeLine(le);
		}

		// type is region
		if (le.first().atomType() == ListExpr.SYMBOL_ATOM
				&& le.first().symbolValue().equals("region")) {
			dataType = "region";
			this.analyzeRegion(le);
		}
		
		// type is mpoint
		if (le.first().atomType() == ListExpr.SYMBOL_ATOM
				&& le.first().symbolValue().equals("mpoint")) {
			dataType = "mpoint";
			this.analyzeMPoint(le);
		}
		
		// type is relation
		if (le.first().atomType() != ListExpr.SYMBOL_ATOM
				&& le.first().first().writeListExprToString().contains("rel")) {
			dataType = "rel";			
			this.analyzeRelation(le);
			
			
		}
		
		//add attributelist to datatypes
		if(!resultTypeList.isEmpty()){
			this.addAttributeListToDataType(le);		
		}

		return dataType;
	}
	
	/** Creates a new DataType of Type Point and adds the information from the given listexpression
	 * 
	 * @param resultList The resultlist of the secondo query
	 * @return The point object that has been created
	 * */
	public Point addPoint(ListExpr resultList){
		
		Point point = new Point();
		
		//add location
		Double x = resultList.second().first().realValue(); //longitude = X 
		point.setX(x);
		Double y = resultList.second().second().realValue();//latitude = Y
		point.setY(y);
		
		point.setId(objectCounter);
		objectCounter++;
				
		return point;
	}
	
	/** Creates a new DataType of Type Line and adds the information from the given points
	 * 
	 * @param a First point of the line
	 * @param b Second point of the line
	 * @return The line object that has been created
	 * */
	public Line addLine(Point a, Point b){
		
		Line line = new Line();
		line.setPointA(a);
		line.setPointB(b);
		
		return line;
	}
	
	/** Creates a new DataType of Type Polygon and adds the information from the given pointlist
	 * 
	 * @param pointlist The list of points representing the outline of the polygon
	 * @return The polygon object that has been created
	 * */
	public Polygon addPolygon(ArrayList<Point> pointlist){
		
		Polygon polygon = new Polygon();
		
		for(Point point : pointlist){
			polygon.addPointToPath(point);
		}
		
		polygon.setId(objectCounter);
		objectCounter++;

		return polygon;
	}
	
	/** Creates a new DataType of Type Polyline and adds the information from the given linelist
	 * 
	 * @param linelist The list of lines that represent the path of the polyline
	 * @return The polyline object that has been created
	 * */
	public Polyline addPolyline(ArrayList<Line> linelist){
		
		Polyline polyline = new Polyline();
		
		for(Line line : linelist){
			polyline.addLineToPath(line);
		}
		
		polyline.setId(objectCounter);
		objectCounter++;

		return polyline;
	}
	
	/** Creates a new DataType of Type MPoint and adds the information from the given elements
	 * 
	 * @param linelist The list of lines representing the path of the moving point
	 * @param timelist The list of dates respresenting the time that the moving point takes to move along the path
	 * */
	public MPoint addMPoint(ArrayList<Line> linelist, ArrayList<TimeInterval> timelist){
		
		MPoint mpoint = new MPoint();
		mpoint.setPath(linelist);
		mpoint.setTime(timelist);
		
		mpoint.setId(objectCounter);
		objectCounter++;
		
		return mpoint;
	}
	
	
	/**Analyzes the relation result in the secondo resultlist for implemented datatypes and puts the found objects into the resultTypeList
	 * 
	 * @param le The resultlist of the secondo query
	 * */
	public void analyzeRelation(ListExpr le){
				
		ListExpr values= le.second();
		int index = 0;

		while (!values.isEmpty()) {
			
			ListExpr listentry = values.first();
			ListExpr tuplelist = le.first().second().second();
			
			int index1 = 0;
			
			//get the first tuple as the name of the datatype
			String firstTuple = listentry.first().writeListExprToString();

			//get all tuple
			while (!listentry.isEmpty()) {
				
				//if tuples include points
				if (tuplelist.first().second().stringValue().equals("point")) {

					Point point = new Point();					
					point.setName(firstTuple.trim());
										
					//add location
					Double x = listentry.first().first().realValue(); //longitude = X
					point.setX(x);
					Double y = listentry.first().second().realValue();//latitude = Y
					point.setY(y);

					point.setId(objectCounter);
					objectCounter++;

					resultTypeList.add(point);						
				}	
				
				//if tuples include lines
                if (tuplelist.first().second().stringValue().equals("line")) {
                	
                	ArrayList<Line> lines = new ArrayList<Line>();
                	
                	ListExpr lineList = listentry.first(); 	
                	int index2 = 0;
                	
                	//get all lines of one tuple
                	while(!lineList.isEmpty()){

					   Point pointA = new Point();
					   Point pointB = new Point();					
					
					   //add location
					   Double xA = lineList.first().first().realValue(); //longitude = X
					   pointA.setX(xA);
					   Double yA = lineList.first().second().realValue();//latitude = Y
					   pointA.setY(yA);
					
					   Double xB = lineList.first().third().realValue(); //longitude = X
					   pointB.setX(xB);
					   Double yB = lineList.first().fourth().realValue();//latitude = Y
					   pointB.setY(yB);
					
					   Line line = new Line();
					   line.setPointA(pointA);
					   line.setPointB(pointB);
					   lines.add(line);
					   
					   lineList = lineList.rest();
	    			   index2++;
                	}	
                	
                	//create a new polyline and add it to the resultlist
                	Polyline polyline = new Polyline();             		
            		for(Line line : lines){
            			polyline.addLineToPath(line);
            		}
            		polyline.setName(firstTuple.trim());
            		polyline.setId(objectCounter);
            		objectCounter++;
                    resultTypeList.add(polyline);
                    lines.clear();
				}	
                
                if (tuplelist.first().second().stringValue().equals("region")) {
   
                	ArrayList<Point> pointlist = new ArrayList<Point>();
                	
                	ListExpr outerlineList = listentry.first(); 	
                	int index2 = 0;
                	
                	//get all outlines of one region tuple
                	while(!outerlineList.isEmpty()){
                		
                	   ListExpr innerlineList = outerlineList.first(); 	
                       int index3 = 0;
                       
                      //get all outlines of one region tuple
                   	  while(!innerlineList.isEmpty()){
                   		
                   		ListExpr lineList = innerlineList.first(); 	
                        int index4 = 0;

                        while(!lineList.isEmpty()){
                           
                           Point point = new Point();				
    					
    					   //add location
    					   Double x = lineList.first().first().realValue(); //longitude = X 
    					   point.setX(x);
    					   Double y = lineList.first().second().realValue();//latitude = Y
    					   point.setY(y);
    					
    					   pointlist.add(point);
                        	
                           lineList = lineList.rest();
     	    			   index4++;
                        }
                        
                        Polygon polygon = new Polygon();             		
                		for(Point point : pointlist){
                			polygon.addPointToPath(point);
                		}
                		polygon.setName(firstTuple.trim());
                		polygon.setId(objectCounter);
                		objectCounter++;
                        resultTypeList.add(polygon);

                        pointlist.clear();
                   		  
                   	   innerlineList = innerlineList.rest();
  	    			   index3++;
                   	  }              	   
                	   outerlineList = outerlineList.rest();
 	    			   index2++;
                	}
				}	
                
                if (tuplelist.first().second().stringValue().equals("mpoint")) {

                	ArrayList<Line> linelist = new ArrayList<Line>();
            		ArrayList<TimeInterval> timelist = new ArrayList<TimeInterval>();
            		MPoint mpoint = new MPoint();
            		
            		ListExpr valueList = listentry.first(); 
            	
            		int index2 = 0;

            		while (!valueList.isEmpty()) {			
            			
            			   //get time
            			   Date timeA = parseStringToDate(valueList.first().first().first().writeListExprToString());
            			   Date timeB = parseStringToDate(valueList.first().first().second().writeListExprToString());	
            			   
            			   //add time to timelist
            			   TimeInterval time = new TimeInterval();
            			   time.setTimeA(timeA);
            			   time.setTimeB(timeB);
            			   timelist.add(time);
            			
            			   //get location
            			   Point pointA = new Point();
            			   Point pointB = new Point();	
            			   Double xA = valueList.first().second().first().realValue(); //longitude = X1
            			   pointA.setX(xA);
            			   Double yA = valueList.first().second().second().realValue();//latitude = Y1
            			   pointA.setY(yA);
            			
            			   Double xB = valueList.first().second().third().realValue(); //longitude = X2
            			   pointB.setX(xB);
            			   Double yB = valueList.first().second().fourth().realValue();//latitude = Y2
            			   pointB.setY(yB);	
            			   
            			   Line line = new Line();
            			   line.setPointA(pointA);
            			   line.setPointB(pointB);
            			   linelist.add(line);         			   
            			   
            			   valueList = valueList.rest();
            			   index2++;
            		}	
            		   //add the MPoint to the resultlist
            		    mpoint.setId(objectCounter);
            		    objectCounter++;
            		    mpoint.setName(firstTuple.trim());
            		    mpoint.setPath(linelist);
            		    mpoint.setTime(timelist);
            		    resultTypeList.add(mpoint);
				}
                
                if (tuplelist.first().second().stringValue().equals("mlabel")) {

                	ArrayList<String> labellist = new ArrayList<String>();
            		ArrayList<TimeInterval> timelist = new ArrayList<TimeInterval>();
            		MLabel mlabel = new MLabel();
            		
            		ListExpr valueList = listentry.first();            	
            		

            		while (!valueList.isEmpty()) {			
            			
            			   //get time
            			   Date timeA = parseStringToDate(valueList.first().first().first().writeListExprToString());
            			   Date timeB = parseStringToDate(valueList.first().first().second().writeListExprToString());	
            			   
            			   //add time to timelist
            			   TimeInterval time = new TimeInterval();
            			   time.setTimeA(timeA);
            			   time.setTimeB(timeB);
            			   timelist.add(time);
            			
            			   //get label
            			   String label=valueList.first().second().symbolValue();
            			   //generate color for each label
            			   mlabel.generateColorsForLabel2(label);
            			   if(labellist.contains(label)){
            				   int indexOfFirstOccurrance=labellist.indexOf(label);
            				   labellist.add(label);
            				   mlabel.generateColorsForDuplicateLabel(indexOfFirstOccurrance);
            				   
            			   }else{
            			   labellist.add(label);
            			   mlabel.generateColorsForLabel(labellist.size()-1);
            			   
            			   }           			      			   
            			   
            			   valueList = valueList.rest();
            			  
            		}	
            		   //add the MLabel to the resultlist
            		    mlabel.setId(objectCounter);
            		    objectCounter++;  
            		    mlabel.setName(firstTuple.trim());
            		    //name - attribute name of type mlabel in the relation 
            		    mlabel.setAttributeNameInRelation(tuplelist.first().first().stringValue());            		    
            		    mlabel.setTime(timelist);
            		    mlabel.setLabel(labellist);
            		    
            		    resultTypeList.add(mlabel);
				}
				tuplelist = tuplelist.rest();
				listentry = listentry.rest();
				index1++;		
				}	
			
			values = values.rest();
			index++;
		}	
    	System.out.println("############## Size of resultTypeList: " +resultTypeList.size());
    	for(DataType each:resultTypeList){
    		System.out.println(" "+each.getType());
    	}
	}
	
	/**Gets the line-result from secondo, builds a list of lines and puts it into the resultTypeList
	 * 
	 * @param le The secondo result containing a line object
	 * */
	public void analyzeLine(ListExpr le){
		
		ArrayList<Line> lines = new ArrayList<Line>();
		
        ListExpr values= le.second();
	
		int index = 0;

		while (!values.isEmpty()) {

			   Point pointA = new Point();
			   Point pointB = new Point();					
			
			   //add location
			   Double xA = values.first().first().realValue(); //longitude = X
			   pointA.setX(xA);
			   Double yA = values.first().second().realValue();//latitude = Y
			   pointA.setY(yA);
			
			   Double xB = values.first().third().realValue(); //longitude = X
			   pointB.setX(xB);
			   Double yB = values.first().fourth().realValue();//latitude = Y
			   pointB.setY(yB);
			
			   Line line = new Line();
			   line.setPointA(pointA);
			   line.setPointB(pointB);
			   lines.add(line);
			   
			   values = values.rest();
			   index++;
		}
        resultTypeList.add(addPolyline(lines));
        lines.clear();
	}
	
	/**Gets the region-result from secondo, builds a polyline and puts it into the resultTypeList
	 * 
	 * @param le The secondo resultlist containing a region
	 * */
	public void analyzeRegion(ListExpr le){
		
		ArrayList<Point> pointlist = new ArrayList<Point>();
		
        ListExpr values= le.second().first().first();
	
		int index = 0;

		while (!values.isEmpty()) {

		    Point point = new Point();				
			
			   //add location
			   Double x = values.first().first().realValue(); //longitude = X 
			   point.setX(x);
			   Double y = values.first().second().realValue();//latitude = Y
			   point.setY(y);
			
			   pointlist.add(point);

			values = values.rest();
			index++;
		}
		resultTypeList.add(addPolygon(pointlist));
        pointlist.clear();		
	}
	
	/**Gets the mpoint-result from secondo, builds a pointlist and a timelist and puts it into the resultTypeList
	 * 
	 * @param le The secondo resultlist containing a moving point
	 * */
	public void analyzeMPoint(ListExpr le){
		
		ArrayList<Line> linelist = new ArrayList<Line>();
		ArrayList<TimeInterval> timelist = new ArrayList<TimeInterval>();
		MPoint mpoint = new MPoint();
	
        ListExpr values= le.second();
	
		int index = 0;

		while (!values.isEmpty()) {			
			
			   //get time
			   Date timeA = parseStringToDate(values.first().first().first().writeListExprToString());
			   Date timeB = parseStringToDate(values.first().first().second().writeListExprToString());	
			   
			   //add time to timelist
			   TimeInterval time = new TimeInterval();
			   time.setTimeA(timeA);
			   time.setTimeB(timeB);
			   timelist.add(time);
			
			   //get location
			   Point pointA = new Point();
			   Point pointB = new Point();	
			   Double xA = values.first().second().first().realValue(); //longitude = X1
			   pointA.setX(xA);
			   Double yA = values.first().second().second().realValue();//latitude = Y1
			   pointA.setY(yA);
			
			   Double xB = values.first().second().third().realValue(); //longitude = X2
			   pointB.setX(xB);
			   Double yB = values.first().second().fourth().realValue();//latitude = Y2
			   pointB.setY(yB);	
			   
			   Line line = new Line();
			   line.setPointA(pointA);
			   line.setPointB(pointB);
			   linelist.add(line); 
			   
			   values = values.rest();
			   index++;
		}	
		
		   //add the MPoint to the resultlist
	       mpoint.setId(objectCounter);
	       objectCounter++;
	       mpoint.setPath(linelist);
	       mpoint.setTime(timelist);
	       resultTypeList.add(mpoint);
	}
	
	/**Gets a time string from the secondo result and parses it into a java date object
	 * 
	 * @param time The time string from the secondo result
	 * @return The date object
	 * */
	public Date parseStringToDate(String time){
		
		SimpleDateFormat formatterMinutes = new SimpleDateFormat("yyyy-MM-dd-HH:mm", Locale.GERMAN); //length with minutes only: 16
		SimpleDateFormat formatterSeconds = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss", Locale.GERMAN); //length with seconds: 19
		SimpleDateFormat formatterMilliseconds = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss.SSS", Locale.GERMAN); //length: 21-23
		SimpleDateFormat formatter = new SimpleDateFormat();
		
		//remove double quotes
		time = time.substring(2, time.length()-1); 
		
		if(time.length()==16){
			formatter = formatterMinutes;
		}
		if(time.length()==19){
			formatter = formatterSeconds;
		}
		if(time.length() > 20){
			formatter = formatterMilliseconds;
		}
		
		Date date = new Date();
		try {
			date = (Date)formatter.parse(time);
		} catch (ParseException e) {
			e.printStackTrace();
		}
	return date;
	}
	
	/**Adds the list of attributes to the given relation datatype from the current result
	 * 
	 * @param le The current secondo result list
	 * */
	public void addAttributeListToDataType(ListExpr le){
		
		int indexResult = 0;
		DataType datatype = resultTypeList.get(indexResult);
		
		//get relation data and add attributes
		if(le.first().atomType()!=ListExpr.SYMBOL_ATOM && le.first().first().writeListExprToString().contains("rel")){
		
		ListExpr values= le.second();
		int index = 0;

		while (!values.isEmpty()) {
			
			ListExpr listentry = values.first();
			ListExpr tuplelist = le.first().second().second();
			datatype = resultTypeList.get(indexResult);
			
			int index1 = 0;

			//get all tuples of one object
			while (!listentry.isEmpty()) {
			    
			  //for the line or region add just (geometry) not all points
	    		if (tuplelist.first().second().stringValue().equals("line") || tuplelist.first().second().stringValue().equals("region") || 
	    				tuplelist.first().second().stringValue().equals("mpoint")){
	    			datatype.getAttributeList().add(tuplelist.first().first().stringValue().trim() + " : " +  "(geometry) \n \n"); 
	    		}
	    		
	    		else{
			        datatype.getAttributeList().add(tuplelist.first().first().stringValue().trim() + " : " + listentry.first().writeListExprToString().trim()); //Ort :	(8.798653 53.08336)
	    		}
	    		
			    tuplelist = tuplelist.rest();
			    listentry = listentry.rest();
			    index1++;	
		    }
			indexResult++;
			
			values = values.rest();
			index++;
		  }
		}
	}
	
	/**Resets object counter to 1 */
	public void resetCounter(){
		objectCounter = 1;
	}

	/**Returns the list of datatype objects
	 * 
	 * @return The resultlist with datatype objects
	 * */
	public ArrayList<DataType> getResultTypeList() {
		return resultTypeList;
	}	
}
