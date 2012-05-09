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
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;

import org.apache.log4j.Logger;


/**
 * This class provides a data collector. The progress data from 
 * the query log of secondo is collected and written 
 * 
 *
 */
public class DataCollectorMain {

	protected static Logger logger = Logger.getLogger(DataCollectorMain.class);

	/**
	 * Main method for our DataCollector
	 * @throws IOException 
	 */
	public static void main(String[] args) throws IOException {

		final String secondoDir = System.getenv("SECONDO_BUILD_DIR");
		
		if(secondoDir == null) {
			logger.error("No filename specified and environment " +
					"SECONDO_BUILD_DIR is not set, unable to determine " +
					"Path for secondo/bin/proglogt.csv");
			System.exit(-1);
		}
		
		final String filename = secondoDir + "/bin/proglogt.csv";
		startApplication(filename);
	}

	protected static void startApplication(final String filename) throws IOException {
		Main.initGUI();
				
		// Watch SECONDO progress log, like "tail -f"
		final BufferedReader br = new BufferedReader(new FileReader(filename));
		final FileWatcherWorker worker = new FileWatcherWorker(br);
		AppCtx.getInstance().setWorker(worker);

		final Thread workerThread = new Thread(worker);
		workerThread.start();
		
		BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
		
		// Install progress listener
		for(WindowType w: WindowType.values()) {
			AppCtx.getInstance().getQueryProgressTableWindow(w).installListener();
		}
		
		String line = null;
		while(true)	{
			System.out.println("Type EXIT to exit");
			System.out.println("Press enter to write data: ");

			line = stdin.readLine();
			
			if ("EXIT".equals(line)) {
				break;
			}
			
			// Export progress data
			for(WindowType w: WindowType.values()) {
				 AbstractTableWindow window 
				 	= AppCtx.getInstance().getQueryProgressTableWindow(w);
				 
				 final String data = window.getCVSData();
				 
				 final FileWriter fw = new FileWriter(new File("progress_" + w.name() + ".csv"));
				 fw.write(data);
				 fw.close();
			}
			
			System.out.println("Data written");
		}
		
		System.exit(0);
	}
}