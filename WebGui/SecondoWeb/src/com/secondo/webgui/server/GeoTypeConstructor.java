package com.secondo.webgui.server;

import java.io.Serializable;
import java.util.ArrayList;

import sj.lang.ListExpr;

import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Line;
import com.secondo.webgui.client.datatypes.Point;
import com.secondo.webgui.client.datatypes.Polygon;
import com.secondo.webgui.client.datatypes.Polyline;
import com.secondo.webgui.utils.LEUtils;

public class GeoTypeConstructor implements Serializable{
	
	private ArrayList<DataType> resultTypeList = new ArrayList<DataType>();

	public GeoTypeConstructor() {}


	/**Checks the given listexpression for the type of the queried Secondo Object*/
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

		// type is mpoint
		if (le.first().atomType() == ListExpr.SYMBOL_ATOM
				&& le.first().symbolValue().equals("mpoint")) {
			dataType = "mpoint";
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
		
		// type is relation
		if (le.first().atomType() != ListExpr.SYMBOL_ATOM
				&& le.first().first().writeListExprToString().contains("rel")) {
			dataType = "rel";			
			this.analyseRelation(le);
		}

		return dataType;
	}
	
	/** Creates a new DataType of Type Point and adds the information from the given listexpression*/
	public Point addPoint(ListExpr resultList){
		
		Point point = new Point();
		
		//add location
		Double x = resultList.second().first().realValue(); //longitude = X // check if realValue is save here or if LEUtils is needed
		point.setX(x);
		Double y = resultList.second().second().realValue();//latitude = Y
		point.setY(y);
				
		return point;
	}
	
	/** Creates a new DataType of Type Line and adds the information from the given listexpression*/
	public Line addLine(Point a, Point b){
		
		Line line = new Line();
		line.setA(a);
		line.setB(b);
		
		return line;
	}
	
	/** Creates a new DataType of Type Polyline and adds the information from the given pointlist*/
	public Polyline addPolyline(ArrayList<Point> pointlist){
		
		Polyline polyline = new Polyline();
		
		for(Point point : pointlist){
			polyline.addPointToPath(point);
		}

		return polyline;
	}
	
	/** Creates a new DataType of Type Polygon and adds the information from the given pointlist*/
	public Polygon addPolygon(ArrayList<Point> pointlist){
		
		Polygon polygon = new Polygon();
		
		for(Point point : pointlist){
			polygon.addPointToPath(point);
		}

		return polygon;
	}
	
	
	/**Gets the relation result in a list expression and analyzes its tuple for types and puts the result into the resultTypeList*/
	public void analyseRelation(ListExpr le){
				
		ListExpr values= le.second();
		int index = 0;

		while (!values.isEmpty()) {
			
			ListExpr listentry = values.first();
			ListExpr tuplelist = le.first().second().second();
			
			int index1 = 0;

			//get all tuple
			while (!listentry.isEmpty()) {
				
				//if tuples include points
				if (tuplelist.first().second().stringValue().equals("point")) {
					
					System.out.println(tuplelist.first().first().stringValue() + " : " + listentry.first().writeListExprToString() + "\n \n"); //Ort :	(8.798653 53.08336)

					Point point = new Point();
					
					//add location
					Double x = listentry.first().first().realValue(); //longitude = X // check if realValue is save here or if LEUtils is needed
					point.setX(x);
					Double y = listentry.first().second().realValue();//latitude = Y
					point.setY(y);
					
					setTextToDataType(le, point);

					resultTypeList.add(point);
						
				}	
				
				//if tuples include lines
                if (tuplelist.first().second().stringValue().equals("line")) {
                	
                	//System.out.println("########" + tuplelist.first().first().stringValue() + " : " + listentry.first().first().first().writeListExprToString() + "\n \n"); //AVerlauf : 8.573387
                	
                	ListExpr lineList = listentry.first(); 	
                	int index2 = 0;
                	
                	//get all lines of one tuple
                	while(!lineList.isEmpty()){

					   Point pointA = new Point();
					   Point pointB = new Point();					
					
					   //add location
					   Double xA = LEUtils.readNumeric(lineList.first().first()); //longitude = X // check if realValue is save here or if LEUtils is needed
					   pointA.setX(xA);
					   Double yA = LEUtils.readNumeric(lineList.first().second());//latitude = Y
					   pointA.setY(yA);
					
					   Double xB = lineList.first().third().realValue(); //longitude = X // check if realValue is save here or if LEUtils is needed
					   pointB.setX(xB);
					   Double yB = LEUtils.readNumeric(lineList.first().fourth());//latitude = Y
					   pointB.setY(yB);
					
					   resultTypeList.add(addLine(pointA, pointB));
					   
					   lineList = lineList.rest();
	    			   index2++;
                	}				
				}	
                
                if (tuplelist.first().second().stringValue().equals("region")) {
                	
                	System.out.println("########" + tuplelist.first().first().stringValue() + " : " + listentry.first().first().first().first().first().writeListExprToString() + "\n \n"); //Gebiet : 11.80777
                	
                	ArrayList<Point> pointlist = new ArrayList<Point>();
                	
                	ListExpr outerlineList = listentry.first(); 	
                	int index2 = 0;
                	
                	//get all outlines of one region tuple
                	while(!outerlineList.isEmpty()){
                		
                	   ListExpr innerlineList = outerlineList.first(); 	
                       int index3 = 0;
                       
                       //System.out.println("#############" + outerlineList.first().writeListExprToString()); //(  ( (12.057415 47.81727)...
                       
                      //get all outlines of one region tuple
                   	  while(!innerlineList.isEmpty()){
                   		  
                   		//System.out.println("#############" + innerlineList.first().writeListExprToString()); //( (12.057415 47.81727)
                   		
                   		ListExpr lineList = innerlineList.first(); 	
                        int index4 = 0;
                        
                        
                        
                        while(!lineList.isEmpty()){
                        	
                         //  System.out.println("#############" + lineList.first().writeListExprToString()); // (12.401131 47.85333)
                           
                           Point point = new Point();				
    					
    					   //add location
    					   Double x = lineList.first().first().realValue(); //longitude = X // check if realValue is save here or if LEUtils is needed
    					   point.setX(x);
    					   Double y = LEUtils.readNumeric(lineList.first().second());//latitude = Y
    					   point.setY(y);
    					
    					   pointlist.add(point);
                        	
                           lineList = lineList.rest();
     	    			   index4++;
                        }
                        
                        resultTypeList.add(addPolyline(pointlist));
                        //check if the first point is the same as the last point, if so, its a polygon not a polyline == stimmt nicht überein!!
                        //System.out.println("###########First element of pointlist: " + pointlist.get(0).getX() + "Last element of pointlist: "+ pointlist.get(pointlist.size()-1).getX());
                       // resultTypeList.add(addPolygon(pointlist));
                        pointlist.clear();
                   		  
                   	   innerlineList = innerlineList.rest();
  	    			   index3++;
                   	  }
                	   
                	   outerlineList = outerlineList.rest();
 	    			   index2++;
                	}
				}	

				tuplelist = tuplelist.rest();
				listentry = listentry.rest();
				index1++;		
				}	
			
			values = values.rest();
			index++;
		}	
    	System.out.println("############## Size of resultTypeList: " +resultTypeList.size());
	}
	
	/**Gets the line-result from secondo, builds a list of lines and puts it into the resultTypeList*/
	public void analyzeLine(ListExpr le){
		
		System.out.println("######## values of line type: " + le.second().first().writeListExprToString()); //(-10849.0 1142.0 -10720.0 454.0)
		
        ListExpr values= le.second();
	
		int index = 0;

		while (!values.isEmpty()) {

			   Point pointA = new Point();
			   Point pointB = new Point();					
			
			   //add location
			   Double xA = LEUtils.readNumeric(values.first().first()); //longitude = X // check if realValue is save here or if LEUtils is needed
			   pointA.setX(xA);
			   Double yA = LEUtils.readNumeric(values.first().second());//latitude = Y
			   pointA.setY(yA);
			
			   Double xB = values.first().third().realValue(); //longitude = X // check if realValue is save here or if LEUtils is needed
			   pointB.setX(xB);
			   Double yB = LEUtils.readNumeric(values.first().fourth());//latitude = Y
			   pointB.setY(yB);
			
			   resultTypeList.add(addLine(pointA, pointB));
			   
			   values = values.rest();
			   index++;
		}
	}
	
	/**Gets the region-result from secondo, builds a polyline and puts it into the resultTypeList*/
	public void analyzeRegion(ListExpr le){
		
		ArrayList<Point> pointlist = new ArrayList<Point>();
		
        ListExpr values= le.second().first().first();
	
		int index = 0;

		while (!values.isEmpty()) {

		    Point point = new Point();				
			
			   //add location
			   Double x = values.first().first().realValue(); //longitude = X // check if realValue is save here or if LEUtils is needed
			   point.setX(x);
			   Double y = LEUtils.readNumeric(values.first().second());//latitude = Y
			   point.setY(y);
			
			   pointlist.add(point);

			values = values.rest();
			index++;
		}
		resultTypeList.add(addPolyline(pointlist));
        pointlist.clear();
		
	}
	
	
	//not necessary? done in textformatter
	/**Gets the result from the query and the datatype and sets the result as textrepresetation to the datatype*/
	public void setTextToDataType(ListExpr le, DataType data){
		
		ListExpr values= le.second();
		int index = 0;

		  while (!values.isEmpty()) {
			
			ListExpr listentry = values.first();
			ListExpr tuplelist = le.first().second().second();
			
			int index1 = 0;
		
		   //add all tuples as strings to the textarray
	    	while (!tuplelist.isEmpty()){
	    		ArrayList<String> textList = data.getTextList();
	    		
	    		//don´t add the line or region points to the text representation
	    		if (!tuplelist.first().first().stringValue().equals("line") || !tuplelist.first().first().stringValue().equals("region")){
	    		   textList.add(tuplelist.first().first().stringValue() + " : " + listentry.first().writeListExprToString() + "\n \n");//Ort :	(8.798653 53.08336)
		           data.setTextList(textList);
		           //System.out.println("############## One tuple added to textarray: " + tuplelist.first().first().stringValue());
	    		}
		    	
		    	tuplelist = tuplelist.rest();
				listentry = listentry.rest();
				index1++;
		       }
			  values = values.rest();
			  index++;
		  }
	}


	public ArrayList<DataType> getResultTypeList() {
		return resultTypeList;
	}


	public void setResultTypeList(ArrayList<DataType> resultTypeList) {
		this.resultTypeList = resultTypeList;
	}


	
}
