package com.secondo.webgui.server;

import java.util.ArrayList;

import com.secondo.webgui.client.datatypes.Point;
import com.secondo.webgui.utils.LEUtils;

import sj.lang.ListExpr;

public class TextFormatter {
	
	private ArrayList<String> formattedList = new ArrayList<String>();
	private ArrayList<String> firstTuplesOfValues = new ArrayList<String>();
	
	public TextFormatter(){
		
	}
	
	/** This method gets a listexpression, formats the data and returns a formatted string*/
	public ArrayList<String> formatData(ListExpr le){
		
		formattedList.clear();
		firstTuplesOfValues.clear();
		
		formattedList.add("unknown type");
			
		//formattedList.add("\n--------------------\n");
	
		//System.out.println("################### first() Data-Type: " + le.first().symbolValue()); //inquiry, mpoint, geht nicht für rel, da leer
		//System.out.println("################### first().first(): " + le.first().first().writeListExprToString()); //doesnt work for inquiry, for rel = rel!
		//System.out.println("################### first().second(): " + le.first().second().writeListExprToString()); //doesnt work for inquiry, for rel = (tuple ((Name string)(Typ string)(GeoData line)))
		//System.out.println("################### second(): " + le.second().writeListExprToString());// doesnt work for inquiry, for rel ( ("U1,U12,U15" "Zone A" 
		//System.out.println("################### second().first(): " + le.second().first().writeListExprToString()); //databases, ("U1,U12,U15" "Zone A" für rel
		//System.out.println("################### second().second()first(): " + le.second().second().first().writeListExprToString());//BERLINTEST
		//System.out.println("################### first. symbol atom: " + le.first().atomType());
		//System.out.println("####### Tuplevalues:" + le.first().second().first().writeListExprToString()); //tuple
		//System.out.println("####### Tuplevalues:" +le.first().second().second().writeListExprToString()); //( (Name string)(Typ string) (GeoData line))
		
		
		//format all SecondoObjects
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("inquiry")){
		    formattedList.clear();
		    formatSO(le);
		    }

		//format point data
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("point")){
			formattedList.clear();
			return formatPoint(le);
            }
		
		//format mpoint data
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("mpoint")){
			formattedList.clear();
			return formatMPoint(le);
            }
		
		// type is line
		if (le.first().atomType() == ListExpr.SYMBOL_ATOM
						&& le.first().symbolValue().equals("line")) {
			formattedList.clear();
			this.formatLine(le);
			}
		
		//format relation data
		if(le.first().atomType()!=ListExpr.SYMBOL_ATOM && le.first().first().writeListExprToString().contains("rel")){
			
			//get tuple
			ListExpr tuple = le.first().second();
			
			formattedList.clear();
			return formatRel(le);
		    }
		
		//format region data
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("region")){
			formattedList.clear();
			return formatRegion(le);
		    }

		return formattedList;
	}
	
	 /** returns the formatted list with data of a secondoobject if the result can be displayed in the formatted view else unknown type*/
	public ArrayList<String> formatSO(ListExpr LE){
		
		if(LE.listLength() != 2) 
	    	formattedList.add("unknown type");

	    ListExpr Value = LE.second();
	    if(Value.listLength()!=2)
	    	formattedList.add("unknown type");

	    ListExpr Type = Value.first();
	    
	    System.out.println("################### Data-Type: " + LE.second().first().symbolValue()); //databases
	    
	    /* if(Type.atomType()!=ListExpr.SYMBOL_ATOM)
	    	formattedList.add("unknown type");*/
	    
	     String Name = Type.symbolValue();
	     if(Name.equals("constructors") || Name.equals("operators") || Name.equals("algebra") ||
	       Name.equals("algebras") || Name.equals("databases") || Name.equals("types") ||
	       Name.equals("objects")) {

	    	 LE = LE.second(); // ignore first entry
	       	 
	         // String type = le.first().writeListExprToString(); 
	         ListExpr value = LE.second();
	         
	         String formattedTextentry = "";
	         
	         formattedTextentry = formattedTextentry + "Type: " + LE.first().symbolValue()  + "\n \n"; // inquiry
	 		

		     //formattedList.add(value.listLength()>1?"\n":"\n");
	
		     int index = 0;

		     while (!value.isEmpty()) {
			    String data = value.first().writeListExprToString();
			
			    formattedTextentry = formattedTextentry + value.first().writeListExprToString() + "\n"; //BERLINTEST
		 		
		 		formattedList.add(formattedTextentry);
		 		formattedTextentry = "";
			    
			   // formattedList.add(data); // "  * "+ 

			    //formattedList.add("\n--------------------\n");

			    //testausgabe
			    //System.out.println("Ein Element der formattierten Datenliste: " + value.first().writeListExprToString() +"\n");// korrekt und ohne Klammer
			
			    value = value.rest();
			    index++;
		     }
	     }    
	    return formattedList;		
	}
	
	 /** returns the formatted list with mpointdata if the result can be displayed in the formatted view else unknown type*/
	 public ArrayList<String> formatPoint(ListExpr LE){
		 
		 //is it really a point?
		System.out.println("################### Data-Type: " + LE.first().symbolValue()); // point
		
	    if(LE.listLength() != 2) 
	    	formattedList.add("unknown type");

	    if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM )
	    	formattedList.add("unknown type");
	    
	    String formattedTextentry = "";

	    ListExpr values= LE.second();
	    
	    formattedTextentry = formattedTextentry + "Type: " + LE.first().symbolValue()  + "\n \n"; // point
		formattedTextentry = formattedTextentry + "X: " +LE.second().first().realValue() + "\n \n"; //X: 7.472602
		formattedTextentry = formattedTextentry +  "Y: " +LE.second().second().realValue() + "\n"; //Y: 51.51242
		
		formattedList.add(formattedTextentry);
		formattedTextentry = "";
   
	    return formattedList;
	 }
	 
	 /** returns the formatted list with mpointdata if the result can be displayed in the formatted view else unknown type*/
	 public ArrayList<String> formatMPoint(ListExpr LE){
		 
		 //is it really an mpoint?
		System.out.println("################### Data-Type: " + LE.first().symbolValue()); // mpoint
		//System.out.println("################### second().first().first() " + LE.second().first().first().writeListExprToString()); // ("2003-11-20-06:06" "2003-11-20-06:06:08.692" TRUE FALSE)
		//System.out.println("################### second().first().first().first() " + LE.second().first().first().first().writeListExprToString()); // "2003-11-20-06:06"

	    if(LE.listLength() != 2) 
	    	formattedList.add("unknown type");

	    if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM )
	    	formattedList.add("unknown type");

	    ListExpr values= LE.second();
		
	    formattedList.add("Type: " + LE.first().symbolValue());
		//formattedList.add(LE.second().listLength()>1?"\n":"\n");
		formattedList.add("\n--------------------\n");
	
		int index = 0;

		while (!values.isEmpty()) {
			
			formattedList.add(values.first().first().writeListExprToString() + "\n" + values.first().second().writeListExprToString() + "\n"); // Zeitinfos: 4 in Klammern ("2003-11-20-07:01:18.469" "2003-11-20-07:01:29.267" TRUE FALSE)
			
			//formattedList.add(values.first().second().writeListExprToString() + "\n");// Koordinaten: (-3278.0 14555.0 -3423.0 14495.0)

			formattedList.add("\n--------------------\n");
			
			values = values.rest();
			index++;
		}
	    
	    return formattedList;
	 }
	 
	 /** returns the formatted list of relation data if the result can be displayed in the formatted view else unknown type*/
	 public ArrayList<String> formatRel(ListExpr LE){
		 
		 //is it really an relation?
		//System.out.println("################### first().first() : " + LE.first().first().writeListExprToString());  // rel
		//System.out.println("################### first().second(): " + LE.first().second().writeListExprToString()); //for rel = (tuple ((Name string)(Typ string)(GeoData line)))
		//System.out.println("################### second().first(): " + LE.second().first().writeListExprToString()); //("U1,U12,U15" "Zone A" für rel oder (1) für ten
		//System.out.println("################### second().first().first() " + LE.second().first().first().writeListExprToString()); // "U1,U12,U15" für UBahn und 1 für ten
		//System.out.println("################### second().second() " + LE.second().second().writeListExprToString()); // (2)
		//System.out.println("####### Tuplevalues:" + LE.first().second().first().writeListExprToString()); //tuple
		//System.out.println("####### Tuplevalues:" +LE.first().second().second().writeListExprToString()); //( (Name string)(Typ string) (GeoData line))
		//System.out.println("####### Tuplevalues:" +LE.first().second().second().first().writeListExprToString()); // (Name string)
		//System.out.println("####### Tuplevalues:" +LE.first().second().second().first().first().writeListExprToString()); // Name

	    if(LE.listLength() != 2) 
	    	formattedList.add("unknown type");    
	    
	  //  formattedList.add("DataType: " + LE.first().first().stringValue() + "\n \n");    
		//formattedList.add(LE.second().listLength()>1?"\n":"\n");
	  //  formattedList.add("\n--------------------\n");
	    
	    String formattedTextentry = "";
	    firstTuplesOfValues.clear();
	
		ListExpr values= LE.second();
		int index = 0;

		//get all values of the result
		while (!values.isEmpty()) {
			
			ListExpr listentry = values.first();
			ListExpr tuplelist = LE.first().second().second();
			
			int index1 = 0;
			
			//add the first tuple of the value to the firstTuples-list
			firstTuplesOfValues.add(tuplelist.first().first().stringValue() + " : " + listentry.first().writeListExprToString() + "\n \n"); //SName: Bremen
			
			//get all tuples of one value
			while (!listentry.isEmpty()) {
				
				//1 tuple
				//don¥t add the line or region points to the text representation
	    		if (!tuplelist.first().first().stringValue().equals("line") || !tuplelist.first().first().stringValue().equals("region")){
				   formattedTextentry = formattedTextentry + tuplelist.first().first().stringValue() + " : "; //Name
				   formattedTextentry = formattedTextentry + listentry.first().writeListExprToString() + "\n \n"; //  "U1,U12,U15" fuer UBahn und 1 fuer ten
	    		}

			    tuplelist = tuplelist.rest();
			    listentry = listentry.rest();
			    index1++;		
			}	
			
			//add all tuples for one entry
			formattedList.add(formattedTextentry);
			formattedTextentry = "";
			
			// if list has just one entry no line is needed
			if(formattedList.size() > 1){
		       formattedList.add("\n--------------------\n");
			}
			
			values = values.rest();
			index++;
		}
	    
	    return formattedList;
	 }
	 
	 public ArrayList<String> formatLine(ListExpr le){
		 
		 //System.out.println("######## values of line type: " + le.second().first().writeListExprToString()); //(-10849.0 1142.0 -10720.0 454.0)
		 
		    if(le.listLength() != 2) 
		    	formattedList.add("unknown type");

		    if(le.first().atomType()!=ListExpr.SYMBOL_ATOM )
		    	formattedList.add("unknown type");
			
	        ListExpr values= le.second();
	        
	        String formattedTextentry = "";
		    
		    formattedTextentry = formattedTextentry + "Type: " + le.first().symbolValue()+ "\n \n"; //line
		
			int index = 0;

			while (!values.isEmpty()) {
				
				   formattedTextentry = formattedTextentry + "X1: "+ values.first().first().realValue() + ", Y1: " + values.first().second().realValue()  + "\n"; //2 coordinates for a line
				   formattedTextentry = formattedTextentry + "X2: "+ values.first().third().realValue() + ", Y2: " + values.first().fourth().realValue()  + "\n"; //2 coordinates for a line
				   formattedTextentry = formattedTextentry + "\n--------------------\n"; 
				    formattedList.add(formattedTextentry);
					formattedTextentry = "";
				   
				   values = values.rest();
				   index++;
			}
			return formattedList;
	 }
	
	 /** returns the formatted list with mpointdata if the result can be displayed in the formatted view else unknown type*/
	 public ArrayList<String> formatRegion(ListExpr LE){
		 
		 //is it really a region?
		System.out.println("################### Data-Type: " + LE.first().symbolValue()); // 
		//System.out.println("################### second().first().first() " + LE.second().first().first().writeListExprToString()); // ( (11.520967 52.077755) (11.524854 52.06822) (11.5346 52.066082)..................
		//System.out.println("################### second().first().first().first() " + LE.second().first().first().first().writeListExprToString()); // (11.520967 52.077755)

	    if(LE.listLength() != 2) 
	    	formattedList.add("unknown type");

	    if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM )
	    	formattedList.add("unknown type");

	    ListExpr values= LE.second().first().first();
	    
	    String formattedTextentry = "";
	    
	    formattedTextentry = formattedTextentry + "Type: " + LE.first().symbolValue()+ "\n \n"; //region

		//formattedList.add(LE.second().listLength()>1?"\n":"\n");
		//formattedList.add("\n--------------------\n");
	
		int index = 0;

		while (!values.isEmpty()) {
			
		    formattedTextentry = formattedTextentry + "X: "+ values.first().first().realValue() + ", Y: " + values.first().second().realValue()  + "\n"; //2 coordinates for a line
		   // formattedTextentry = formattedTextentry + "\n--------------------\n"; 
		    formattedList.add(formattedTextentry);
			formattedTextentry = "";

			values = values.rest();
			index++;
		}
	    
	    return formattedList;
	 }

	public ArrayList<String> getFormattedList() {
		return formattedList;
	}

	public void setFormattedList(ArrayList<String> formattedList) {
		this.formattedList = formattedList;
	}

	public ArrayList<String> getFirstTuplesOfValues() {
		return firstTuplesOfValues;
	}

	public void setFirstTuplesOfValues(ArrayList<String> firstTuplesOfValues) {
		this.firstTuplesOfValues = firstTuplesOfValues;
	}

}
