package de.fernunihagen.dna.hoese.algebras;

import sj.lang.ListExpr;
import de.fernunihagen.dna.hoese.DsplGeneric;
import de.fernunihagen.dna.hoese.LabelAttribute;
import de.fernunihagen.dna.hoese.QueryResult;

public class Dsplstring extends DsplGeneric implements LabelAttribute{
	  
	   String label;


	  public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr)
	  {
	     String T = name;
	     String V;
	     if(value.atomType()==ListExpr.STRING_ATOM){
	        V = value.stringValue();
	     } else if(isUndefined(value)){
	         V = "undefined";
	    } else{
	         V = "<error>";
	     }
	     label = V;
	     T=extendString(T,nameWidth, indent);
	     qr.addEntry(T + " : " + V);
	     return;

	  }

	  public String getLabel(double time){
	     return label;
	  }


	}
