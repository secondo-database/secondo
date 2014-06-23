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
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import sj.lang.ListExpr;
import com.secondo.webgui.server.controller.SecondoConnector;

/**
 * This test class analyses the moving point data of the listexpression result from secondo
 * 
 * @author Kristina Steiger
 */
public class TestAnalyzeMPoint {
	
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
		
		//send command to secondo to return a moving point
		le = sc.doQuery("query train7");
	}

	@Test
	public void testAnalyzeMPoint() {
		
		//is it really an mpoint?
		assertEquals("Result has to be of type mpoint.", le.first().symbolValue(), "mpoint");
		
		assertEquals("One date of the moving point", le.second().first().first().first().writeListExprToString().trim(), "\"2003-11-20-06:06\"");
		System.out.println("#### One date: " + le.second().first().first().first().writeListExprToString()); // one time-tupel: "2003-11-20-06:06"
		
		assertEquals("One line of the moving point", le.second().first().second().writeListExprToString().trim(), "(16821.0 1252.0 16673.0 1387.0)");
		System.out.println("#### One line: " + le.second().first().second().writeListExprToString()); //Line: (16821.0 1252.0 16673.0 1387.0)
		
		assertEquals("One point of the moving point", le.second().first().second().first().writeListExprToString().trim(), "16821.0");
		System.out.println("#### One point:  " + le.second().first().second().first().writeListExprToString());// one point: 16821.0
				
		assertEquals("Parsed date of the moving point", parseStringToDate(le.second().first().first().first().writeListExprToString()).toString(), "Thu Nov 20 06:06:00 CET 2003");
		System.out.println("#### Parsed date to java date object: " + parseStringToDate(le.second().first().first().first().writeListExprToString())); // Thu Nov 20 06:06:00 CET 2003
	}
	
	/**Get a time string from secondo and parse it into a java date object*/
	public Date parseStringToDate(String time){
		
		SimpleDateFormat formatterMinutes = new SimpleDateFormat("yyyy-MM-dd-HH:mm", Locale.GERMAN); //length with minutes only: 16
		SimpleDateFormat formatterSeconds = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss", Locale.GERMAN); //length with seconds: 19
		SimpleDateFormat formatterMilliseconds = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss.SSS", Locale.GERMAN); //length: 21-23
		SimpleDateFormat formatter = new SimpleDateFormat();

		time = time.substring(2, time.length()-1); //remove double quotes
		
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
	
	
	@After
	public void tearDown() throws Exception {
		
		//close database
		assertTrue(sc.closeDatabase("BERLINTEST"));
		
		//disconnect from secondo server
		sc.disconnect();
	}
}
