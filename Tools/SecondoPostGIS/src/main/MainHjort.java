/**
 * 
 */
package main;

import gui.HjortGUI;

import java.io.IOException;
import java.util.logging.ConsoleHandler;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

import secondo.ConnectSecondo;
import util.Configuration;
import util.Global;
import util.IMyLogger;
import utilgui.Meldung;

/**
 * @author Bill
 *
 */
public class MainHjort {

	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub

		
		//System.out.println("Start");
		
		new Global();
		
		Configuration configDatei = new Configuration();
		
		/*configDatei.insertDialogs();
		System.exit(1);
		*/
		
		if(configDatei.read() == false)
		{
			new Meldung("Config: Stanard values used, because can not read or write");
			System.exit(1);
		}
		
		
	/*	ConnectSecondo connectSec = new ConnectSecondo(Global.gsbSEC_Host,
				Integer.valueOf(Global.gsbSEC_Port.toString()), Global.gsbSEC_User,
				Global.gsbSEC_Pwd, Global.gbSEC_UseBinaryList);
		*/
		
		
		//connectSec.connect();
		//connectSec.closeConnection();
		
		//Hier jetzt die Verbindungen checken zu den Interfaces
		
		// Dann erst die GUI aufbauen
	
		
		new HjortGUI();
		
		
		
		
	}



}
