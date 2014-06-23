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

package com.secondo.webgui.server;

import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import com.secondo.webgui.server.controller.SecondoConnector;
import sj.lang.ListExpr;

/**
 * This test class analyses the list expression result from secondo for different datatypes
 * 
 * @author Kristina Steiger
 */
public class TestListExpression {
	
    private SecondoConnector sc = new SecondoConnector();
	
	private ListExpr le;

	@Before
	public void setUp() throws Exception {
		
		//set connection data
		sc.setConnection("testuser", "s3c0nd0", "agnesi.fernuni-hagen.de", 1302);
		
		//connect to secondo-server
		assertTrue(this.sc.connect());
		
		//open the berlin database
		assertTrue(sc.openDatabase("BERLINTEST"));
		
	}

	@Test
	public void testListExpression() throws Exception {
		
		//Check inquiry
		le = sc.doQuery("list databases");
		assertEquals("First tuple contains the datatype", le.first().symbolValue(), "inquiry");
		System.out.println("### Data-Type: " + le.first().symbolValue()); 	
		System.out.println("#### First database: " + le.second().second().first().writeListExprToString());//BERLINTEST
		
		//Check standard type
		le = sc.doQuery("query Trains count");
		assertEquals("Integer Value", le.second().writeListExprToString().trim(), "562");
		System.out.println("### Integer Value: " + le.second().writeListExprToString());
		
		//Check point datatype
		le = sc.doQuery("query mehringdamm");
		assertEquals("Check if its a point Value", le.first().symbolValue(), "point");
		assertEquals("Check the x-value of the point", le.second().first().writeListExprToString().trim(), "9396.0");
		System.out.println("X-value of point: " + le.second().first().writeListExprToString());
		
		//Check line datatype
		le = sc.doQuery("query BGrenzenLine");
		assertEquals("Check if its a line Value", le.first().symbolValue(), "line");
		assertEquals("Check the Line Value", le.second().first().writeListExprToString().trim(), "(-10849.0 1142.0 -10720.0 454.0)");
		System.out.println("Value of first line: " + le.second().first().writeListExprToString());
		assertEquals("Check the first x-coordinate of the line", le.second().first().first().writeListExprToString().trim(), "-10849.0");
		System.out.println("First x-coordinate of the line: " + le.second().first().first().realValue());
		
		//Check datatype region
		le = sc.doQuery("query tiergarten");
		assertEquals("Check if its a region value", le.first().symbolValue(), "region");
		assertEquals("Check one point value of the region", le.second().first().first().first().writeListExprToString().trim(), "(5660.0 11825.0)");
		System.out.println("Value of first point of a region: " + le.second().first().first().first().writeListExprToString());
		
		//Check dataype mpoint
		le = sc.doQuery("query train7");
		assertEquals("Check if its a mpoint value", le.first().symbolValue(), "mpoint");
		assertEquals("Check one date value of the moving point", le.second().first().first().first().writeListExprToString().trim(), "\"2003-11-20-06:06\"");
		System.out.println("Value of first date value of a moving point: " + le.second().first().first().first().writeListExprToString());
		
		//Check relation datatype
		le = sc.doQuery("query Flaechen feed filter[.Name contains \"Grunewald\"] consume");
		assertEquals("First First tuple contains the relation type", le.first().first().writeListExprToString().trim(), "rel");
		System.out.println("### Relation: " + le.first().first().writeListExprToString());		
		assertEquals("Check if there are tuplevalues in the relation", le.first().second().first().writeListExprToString().trim(), "tuple");
		System.out.println("#### Tuplevalues available:" + le.first().second().first().writeListExprToString()); //tuple
		assertEquals("Check the first tuple name", le.first().second().second().first().first().writeListExprToString().trim(), "Name");
		System.out.println("### First Tuple name: " + le.first().second().second().first().first().writeListExprToString());
		assertEquals("Check the first tuple value", le.second().first().first().writeListExprToString().trim(), "\"Grunewald\"");
		System.out.println("### First Tuple value: " + le.second().first().first().writeListExprToString());
					
	}
	 
		@After
		public void tearDown() throws Exception {
			
			//close database
			assertTrue(sc.closeDatabase("BERLINTEST"));
			
			//disconnect from secondo server
			sc.disconnect();
		}
}
