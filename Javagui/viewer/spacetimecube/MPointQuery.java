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

import gui.SecondoObject;
import java.util.*;
import sj.lang.ListExpr;
import tools.Reporter;

/**
 * Class to query a Secondoobject concerning object type MPoint 
 * @author Franz Fahrmeier
 *
 */
public class MPointQuery {
	
	private Vector<MPoint> mPointsVector = new Vector<MPoint>();
	private String oType;
	
	
	/**
	 * Read from relation including MPoints or plain MPoint
	 * @param SO
	 * 		Secondoobject provided by viewer GUI
	 * @return
	 * 		true if SO can be displayed in the viewer
	 */
	public boolean readFromSecondoObject(SecondoObject SO) {
		
	 	   if (checkType(SO)){
	 		  
	 		   if (oType == "rel") {
	 			   String[] columns;
	 			   
	 			  ListExpr LE = SO.toListExpr();
	 			  
	 			  ListExpr value=LE.first(); // (rel...
				   value = value.second(); // (tuple...
				   value = value.second(); // (...
				   
				   ListExpr tempLE;
				   Vector<Integer> mPointPos = new Vector<Integer>(); // stores which columns are from type mpoint
				   Vector<ListExpr> relTuples = new Vector<ListExpr>();
				   tempLE = value;
				   columns = new String[value.listLength()];
				   for (int i=0;i<value.listLength();i++) {
					   columns[i] = tempLE.first().first().toString();
					   String s = tempLE.first().second().toString(); // extract data type from temporary ListExpression
					   if (s.length()>=6) {
		 				  if (s.substring(s.length()-6, s.length()).equals("mpoint")) {
		 					 mPointPos.add(new Integer(i));	// data type MPoint might occur multiple times				  
		 				  }
					   }
					  tempLE = tempLE.rest();
				   }
				  
				   // extract all tuples from LE and store them into a vector
				  value =  LE.second();
				  tempLE = value;
				  for (int i=0;i<value.listLength();i++) {
					  relTuples.add(tempLE.first());
					  if (tempLE.rest().listLength()>=1) tempLE = tempLE.rest();
				  }
				  
				  // loop through tuples
				  for (int i=0;i<relTuples.size();i++) {
					 tempLE = relTuples.get(i);
					 
					 String tmp;
					 int substrLength = 20;
					 Vector<MPoint> mPoints = new Vector<MPoint>();
					 Hashtable<String,String> tmpHt = new Hashtable<String,String>();

					 // loop through columns
					 for (int a=0;a<columns.length;a++) {
						 for (int b=0;b<mPointPos.size();b++) {
							 if (a==mPointPos.get(b).intValue()) {
								 MPoint mp = new MPoint(tempLE.first(), SO.getID());
								 mPoints.add(mp);
							 }
							 else { // exclude MPoint value from additional attributes
								 tmp = tempLE.first().toString();
								 if (tmp.length() >= substrLength) tmp = tempLE.first().toString().substring(0, substrLength);
								 else tmp = tempLE.first().toString();
								 tmpHt.put(columns[a], tmp);
							 }
						 }
						 
						 tempLE = tempLE.rest();
					 }
					 
					 for (int a=0;a<mPoints.size();a++) {
						 MPoint tmpMp = mPoints.get(a);
						 tmpMp.setAdditionalAttributes(tmpHt);
						 mPointsVector.add(tmpMp);
					 }
				  }				       
				  return true;
				  
	 		   }
	 		   else if (oType == "mpoint") {
	 			   ListExpr LE = SO.toListExpr();
	 			   MPoint mp = new MPoint(LE.second(), SO.getID());
	 			   mPointsVector.add(mp);
	 			   return true;
	 		   }
	 		   else return false;
	 
	 	   }
		   else {
			   Reporter.reportWarning("Object type not supported by viewer.", null, false, false, false);
			   return false;
		   }
	}
	
	/**
	 * Get vector of MPoints that have been read by readFromSecondoObject(SecondoObject SO)
	 * @return
	 * 		vector of MPoints
	 */
	public Vector<MPoint> getMPointsVector() { return mPointsVector; }
	
	/*
	 * Check if the type from SO is a relation including MPoints
	 * or a plain MPoint
	 * @param SO
	 * 		Secondoobject provided by viewer GUI
	 * @return
	 * 		true if SO can be displayed in the viewer
	 */
	private boolean checkType(SecondoObject SO) {
		ListExpr LE = SO.toListExpr();
		ListExpr type;
		  if(LE==null)
		     return false;
		  else{
		     if(LE.listLength()!=2){
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
		 					 oType = "rel";
		 					 return true;	 				  
		 				  }
						   
					   }
					  tempLE = tempLE.rest();
				   }
	 			  return false;
		     }
		     else if (typeName.equals("mpoint")) {
		    	 oType = "mpoint";
		    	 return true;
		     }
		     else {
		    	 return false;
		     }
		  }
	}

}
