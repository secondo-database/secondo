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

package  viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;


/**
 * A displayclass for the array-type
 */
public class Dsplarray extends DsplGeneric {

  public void init (String name, int nameWidth, int indent,
                    ListExpr type, ListExpr value, 
                    QueryResult qr) {

    if(type.listLength()!=2){
      qr.addEntry("Error in array type");
      return;   
    }
    ListExpr simpleTypeList =  type.second();
    if(simpleTypeList.atomType()!=ListExpr.SYMBOL_ATOM){
    	String complexType = type.second().first().toString();
    	//display it if its a ~relation~ object
    	if (complexType.trim().equals("rel")){
    		int fieldNo = 1;
        	while (!value.isEmpty()) {
        		qr.addEntry("Field No." + fieldNo++ + "------------");
            	LEUtils.analyse(simpleTypeList.toString(), nameWidth, indent, simpleTypeList, value.first(), qr);
            	value = value.rest();
            	if (!value.isEmpty())
            		qr.addEntry("------------");
        	}
        	return;
    	}
    	else{  	
    		qr.addEntry("Error in array type");
    		return;
    	}
    }
    String simpleType = type.second().symbolValue();

    while (!value.isEmpty()) {
      displayElem(simpleTypeList, simpleType, indent, value.first(), qr);
      value = value.rest();
      if (!value.isEmpty())
        qr.addEntry("---------");
    }
  }


  protected void displayElem(ListExpr typelist, String type, int indent, ListExpr value, QueryResult qr){
    int i;
    DsplBase dg;
		dg = LEUtils.getClassFromName(type);
		// ensure to add exactly one entry per attribute
		int oldnum = qr.getModel().getSize();
	  dg.init("",0,indent, typelist, value, qr);
		
		int newnum = qr.getModel().getSize();
		int diff = newnum-oldnum;
		if(diff<1){
			 tools.Reporter.writeError("missing entry for attribute "+type);
			 tools.Reporter.writeError("check the implementation of the class " + dg.getClass());
			 qr.addEntry("error");
		}
		if(diff>1){
			 tools.Reporter.writeError("to many entries for attribute "+type+
													 "\n please check the implementation of the "+dg.getClass() + " class");
		}
  }


}
