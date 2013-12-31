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

package com.secondo.webgui.client.controller;

import com.google.gwt.user.client.Command;
import com.secondo.webgui.client.mainview.CommandPanel;

/**
 * This class creates all basic commands and adds them to the basic commands menu in the menubar of the commandpanel.
 * 
 * @author Kristina Steiger
 */
public class BasicCommandBuilder {
	
	private CommandPanel commandPanel;
	
	/*Commands for Example Commands in the MenuBar of the CommandPanel*/
	private Command basicCommand1;
	private Command basicCommand2;
	private Command basicCommand3;
	private Command basicCommand4;
	private Command basicCommand5;
	private Command basicCommand6;
	private Command basicCommand7;
	private Command basicCommand8;
	private Command basicCommand9;
	private Command basicCommand10;
	private Command basicCommand11;
	private Command basicCommand12;
	private Command basicCommand13;
	private Command basicCommand14;
	
	public BasicCommandBuilder(CommandPanel cp){
		
		this.commandPanel = cp;
	    
		/*creates a command to execute when the first basic command is selected in the menubar of the commandpanel*/
		this.basicCommand1 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query Stadt feed filter[.SName contains \"Bremen\"] consume");
		    	  commandPanel.getTextArea().setCursorPos(0);
		      }
		    };    	 
        this.commandPanel.getMenubarCP().getBasicCommandItem1().setScheduledCommand(basicCommand1);
       
       /*creates a command to execute when the second basic command is selected in the menubar of the commandpanel*/
		this.basicCommand2 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query Kreis feed filter[.KName contains \"LK Rosenheim\"] consume");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem2().setScheduledCommand(basicCommand2);
      
      /*creates a command to execute when the third basic command is selected in the menubar of the commandpanel*/
		this.basicCommand3 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query Autobahn feed filter[.AName contains \"6\"] consume");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem3().setScheduledCommand(basicCommand3);
        
        /*creates a command to execute when the fourth basic command is selected in the menubar of the commandpanel*/
		this.basicCommand4 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query Stadt feed filter[.SName contains \"Bre\"] consume");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem4().setScheduledCommand(basicCommand4);
        
        /*creates a command to execute when the fifth basic command is selected in the menubar of the commandpanel*/
		this.basicCommand5 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query Flaechen feed filter[.Name contains \"Grunewald\"] consume");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem5().setScheduledCommand(basicCommand5);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand6 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query BGrenzenLine");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem6().setScheduledCommand(basicCommand6);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand7 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query Kinos");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem7().setScheduledCommand(basicCommand7);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand8 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query UBahn");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem8().setScheduledCommand(basicCommand8);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand9 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query train7");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem9().setScheduledCommand(basicCommand9);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand10 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query DayTrips feed filter[.Date contains \"2011-07-31\"] consume");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem10().setScheduledCommand(basicCommand10);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand11 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query Trains feed filter[.Trip passes tiergarten] consume");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem11().setScheduledCommand(basicCommand11);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand12 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >query DayTrips feed filter[.Date contains \"2011\"] consume");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem12().setScheduledCommand(basicCommand12);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand13 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >select count(*) from trains where trip passes mehringdamm");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem13().setScheduledCommand(basicCommand13);
        
        /*creates a command to execute when the command is selected in the menubar of the commandpanel*/
		this.basicCommand14 = new Command() {

		      public void execute() {
		    	  
		    	  commandPanel.getTextArea().setText("Sec >select * from trains where trip passes mehringdamm");
		      }
		    };    		 
        this.commandPanel.getMenubarCP().getBasicCommandItem14().setScheduledCommand(basicCommand14);
	}
}
