//This file is part of SECONDO.

//Copyright (C) 2006, University in Hagen, Department of Computer Science, 
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

// Written 2012 by Jan Kristof Nidzwetzki 

package progresswatcher;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import org.apache.log4j.Logger;

/**
 * The main class for the application 
 *
 */
public class Main {

	protected static Logger logger = Logger.getLogger(Main.class);

	public static void main(final String[] args) throws IOException {
		
		// is a filename as parameter given?
		if(args.length == 0) {
			
			final String secondoDir = System.getenv("SECONDO_BUILD_DIR");
			
			if(secondoDir == null) {
				logger.error("No filename specified and environment " +
						"SECONDO_BUILD_DIR is not set, unable to determine " +
						"Path for secondo/bin/proglogt.csv");
				System.exit(-1);
			}
			
			final String filename = secondoDir + "/bin/proglogt.csv";
			startApplication(filename);
		} else {
			startApplication(args[0]);
		}
		
	}

	/**
	 * Start our application and create window instances
	 */
	protected static void startApplication(final String filename) throws IOException {
		
		// Does SECONDO progress info file exists?
		logger.info("Starting watching file " + filename);
		if(! (new File(filename)).isFile()) {
			logger.error("Unable to find file: " + filename);
			System.exit(-1);
		}
		
		final AbstractProgressWindow progressWindow = initGUI();
		
		// Watch SECONDO progress log, like "tail -f"
		final BufferedReader br = new BufferedReader(new FileReader(filename));
		final FileWatcherWorker worker = new FileWatcherWorker(br);
		AppCtx.getInstance().setWorker(worker);

		final Thread workerThread = new Thread(worker);
		workerThread.start();
		
		// Show progress window
		progressWindow.show();
		
		// If you like to show other windows on startup 
		// - like the time window - use the following lines:
		
		//timeWindow.show();
		//cardWindow.show();
	}

	public static AbstractProgressWindow initGUI() {
		GuiHelper.configureUI();
		
		// Prepare GUI
		final AbstractProgressWindow progressWindow = new EstimatedProgressWindow();
		AppCtx.getInstance().setWindow(WindowType.PROGRESS, progressWindow);
		progressWindow.init();
		
		final AbstractProgressWindow timeWindow = new EstimatedTimeWindow();
		AppCtx.getInstance().setWindow(WindowType.TIME, timeWindow);
		timeWindow.init();
		
		final AbstractProgressWindow cardWindow = new EstimatedCardinalityWindow();
		AppCtx.getInstance().setWindow(WindowType.CARD, cardWindow);
		cardWindow.init();
		
		final EstimatedProgressTableWindow queryTable 
			= new EstimatedProgressTableWindow();
		AppCtx.getInstance().setQueryProgressTableWindow(WindowType.PROGRESS, 
				queryTable);
		queryTable.init();
		
		final EstimatedCardinalityTableWindow cardTable 
			= new EstimatedCardinalityTableWindow();
		AppCtx.getInstance().setQueryProgressTableWindow(WindowType.CARD, 
				cardTable);
		cardTable.init();

		final EstimatedTimeTableWindow timeTable = new EstimatedTimeTableWindow();
		AppCtx.getInstance().setQueryProgressTableWindow(WindowType.TIME, 
				timeTable);
		timeTable.init();
		return progressWindow;
	} 
}
