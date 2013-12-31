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
import org.junit.Test;

/**
 * This test class tests the calculation of the correct slider width
 * 
 * @author Kristina Steiger
 */
public class TestTimeSlider {

	@Test
	public void test() {
		double position = (170 / 7345.0); //has to be divided by a double value
		System.out.println("handleposition after division " + position);
		
    	position = position * 1234;
    	System.out.println("handleposition after multiplication: " + position);
    	
    	int handlePosition = new Double(Math.round(position)).intValue();
    	System.out.println("handleposition in pixel: " + handlePosition);
    	
    	assertTrue(position != 0);
    	
    	assertTrue(handlePosition == 29);

	}
}
