package  viewer.hoese.algebras;

import sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import javax.swing.*;
import java.awt.*;

public class Dsplnrel extends DsplGeneric{
	 public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
		 long startTime=0;
		 if(tools.Environment.MEASURE_TIME){
			 startTime = System.currentTimeMillis();
		 }
		 long usedMemory=0;
		 if(tools.Environment.MEASURE_MEMORY){
			 usedMemory=tools.Environment.usedMemory();
		 }
		 int maxAttribNameLen = maxAttributLength(type.second().second());
		 while (!value.isEmpty()) {
			displayPrimaryTuple(type.second().second(), value.first(), maxAttribNameLen,indent,  qr);
		    value = value.rest();
		    if (!value.isEmpty())
		    	qr.addEntry("---------");
		}
			    
		if(tools.Environment.MEASURE_TIME){
		 Reporter.writeInfo(" Building nested relation has taken :"+
		             (System.currentTimeMillis()-startTime)+" milliseconds");
		}
		if(tools.Environment.MEASURE_MEMORY){
		 Reporter.writeInfo("Memory-Difference :"+ 
		             tools.Environment.formatMemory( tools.Environment.usedMemory()-usedMemory));
		}
	 }
	 
	 /**
	  * Displays the primary tuple of a nested relation.
	  */
	 private void displayPrimaryTuple (ListExpr type, ListExpr value, int maxNameLen, int indent,  QueryResult qr) {
		    int i;
		    String s;
		    DsplBase dg;
		    while (!value.isEmpty()) {
		      s = type.first().first().symbolValue();
		      ListExpr subType = type.first().second();
		      while(subType.atomType()!=ListExpr.SYMBOL_ATOM){
		         subType = subType.first();
		      }
		      dg = LEUtils.getClassFromName(subType.symbolValue());
		      String typeName = subType.symbolValue(); 
		       // ensure to add exactly one entry per attribute
		
		      int oldnum = qr.getModel().getSize();
		      String name = type.first().first().symbolValue();
		      subType = type.first().second();
		      dg.init(name, maxNameLen, indent, subType, value.first(), qr);
		      int newnum = qr.getModel().getSize();
		      int diff = newnum-oldnum;
		      if(diff<1){
		         tools.Reporter.writeError("missing entry for attribute "+s);
		         tools.Reporter.writeError("check the implementation of the class " + dg.getClass());
		         qr.addEntry("error");
		      }
		      if (!(typeName.equals("arel"))){
		    	  if(diff>1){
		    		  tools.Reporter.writeError("to many entries for attribute "+s+
		                             "\n please check the implementation of the "+dg.getClass() + " class");
		    	  }
		      }
		      type = type.rest();
		      value = value.rest();
		    }
		    return;
		}

		/**
		* Calculate the length of the longest attribute name.
		* @param type A ListExpr of the attribute types
		* @return maximal length of attributenames
		* @see <a href="Dspltuplesrc.html#maxAttributLength">Source</a>
		*/
		private static final int maxAttributLength (ListExpr type) {
		    int max, len;
		    String s;
		    max = 0;
		    while (!type.isEmpty()) {
		      s = type.first().first().symbolValue();
		      len = s.length();
		      if (len > max)
		        max = len;
		      type = type.rest();
		    }
		    return  max;
		}
}
