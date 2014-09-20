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

import gui.idmanager.*;
import java.sql.Timestamp;
import java.util.*;
import sj.lang.ListExpr;


/**
 * Class representing object type MPoint
 * @author Franz Fahrmeier
 *
 */
public class MPoint {

	private double[] xCoords;
	private double[] yCoords;
	private String[] timestamps;
	private long[] milliseconds;
	private ID secondoId;
	private Hashtable<String,String> additionalAttr;
	
	
	/**
	 * @param LE
	 * 		ListExpression that holds the MPoint.
	 * @param secId
	 * 		Id of the SecondoObject that includes the MPoint.
	 */
	public MPoint(ListExpr LE, ID secId) {
		readFromListExpr(LE);
		secondoId = secId;
	}
	
	/**
	 * Stores all additional attributes (apart from MPoint) in a Hashtable. 
	 * @param ht
	 * 		Hashtable that stores the additional attributes.
	 */
	public void setAdditionalAttributes(Hashtable<String,String> ht) {
		additionalAttr = ht;
	}
	
	/**
	 * @return
	 * 		Hashtable that stores the additional attributes to the MPoint.
	 */
	public Hashtable<String,String> getAdditionalAttributes() {
		return additionalAttr;
	}
	
	/**
	 * @return
	 * 		ID of corresponding SecondoObject.
	 */
	public ID getSecondoId() {
		return secondoId;
	}
	
	/**
	 * Get all X coordinates from MPoint in a row.
	 * @return
	 * 		X coordinates from MPoint in a row
	 */
	public double[] getXarray() { return xCoords; }
	
	/**
	 * Get all Y coordinates from MPoint in a row.
	 * @return
	 * 		Y coordinates from MPoint in a row
	 */
	public double[] getYarray() { return yCoords; }
	
	/**
	 * Get all timestamps from MPoint in a row.
	 * @return
	 * 		timestamps from MPoint in a row
	 */
	public String[] getTimesArray() { return timestamps; }
	
	/**
	 * Get all timestamps represented as milliseconds since 01/01/1970 from MPoint in a row.
	 * @return
	 * 		timestamps represented as milliseconds since 01/01/1970 from MPoint in a row
	 */
	public long[] getMilliSecondsArray() { return milliseconds; } 
	
	/**
	 * Get a JDBC timestamp from a MPoint time specification.
	 * @param timestamp
	 * 		MPoint time specification.
	 * @return
	 * 		JDBC timestamp.
	 */
	public Timestamp getTimestamp(String timestamp) {
		String part1 = timestamp.substring(0, 10);
		String part2 = "";
		if (timestamp.length()>10) {
			part2 = timestamp.substring(11, timestamp.length());
		}
		 
		if (timestamp.length()==10) {
			part2+="00:00:00.000";
		}
		else if (timestamp.length()==16) {
			part2+=":00.000";
		}
		else if (timestamp.length()==19) {
			part2+=".000";
		}
		 
		String res = part1+" "+part2;
		 
		return Timestamp.valueOf(res);
	}
	
	/*
	 * Reads from given MPoint-ListExpression and
	 * stores X coordinates, y coordinates, timestamps and
	 * timestamps represented as milliseconds since 01/01/1970 in a row.
	 */
	private void readFromListExpr(ListExpr LE) {
		ListExpr value = LE;
		   
		Vector segments = new Vector();
		ListExpr segment;
		ListExpr timeSegment;
		ListExpr pointSegment;
		ListExpr tmp = value;
	   
		while (!tmp.isEmpty()) {
			segments.add(tmp.first());
			tmp = tmp.rest();
		}
	   
		// one segment includes 2 points, that also means 2 X-coordinates, Y-coordinates, timestamps
		xCoords = new double[segments.size()];
		yCoords = new double[segments.size()];
		timestamps = new String[segments.size()];
		milliseconds = new long[segments.size()];
	   
		for (int i=0;i<segments.size();i++) {
			segment = (ListExpr)segments.get(i);
			timeSegment = segment.first();
			pointSegment = segment.second();
		   
			if (pointSegment.first().isAtom() && pointSegment.first().atomType()==ListExpr.REAL_ATOM) {
				double x = pointSegment.first().realValue();			   
				xCoords[i] = x;
			}
			if (pointSegment.second().isAtom() && pointSegment.second().atomType()==ListExpr.REAL_ATOM) {
			   double y = pointSegment.second().realValue();			   
			   yCoords[i] = y;
			}
			if (timeSegment.first().isAtom() && timeSegment.first().atomType()==ListExpr.STRING_ATOM) {
			   String timeSeg = timeSegment.first().stringValue();
			   long millisecs = getTimestamp(timeSeg).getTime();
			   timestamps[i] = timeSeg;
			   milliseconds[i] = millisecs;
			}
		   
		}
	}
	
}


