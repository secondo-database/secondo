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
import java.io.IOException;
import java.text.NumberFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.log4j.Logger;
import org.jfree.data.time.Millisecond;
import org.jfree.data.time.TimeSeries;

/**
 * This class provides a file watcher
 * Implements functionality like the unix command "tail -f" 
 *
 */
public class FileWatcherWorker implements Runnable {

	protected final static Logger logger 
		= Logger.getLogger(FileWatcherWorker.class);
	
	protected final static int MIN_DELAY = 10;
	protected String line;
	protected String lastCommand;
	protected BufferedReader br;
	protected volatile boolean active = true;
	protected final Pattern pattern;
	protected boolean lastMatches = false;

	public FileWatcherWorker(final BufferedReader br) {
		this.br = br;
		pattern = Pattern.compile("(\\d+);(\\d+);(\\d+);\\s*(.+);\\s*(.+);\\s*(.+);");
	}

	/**
	 * Scan the SECONDO Query log every 200ms
	 * Search for new Queries and progress data
	 */
	public void run() {
		try {
			// Skip old lines
			line = br.readLine();

			while (line != null) {
				line = br.readLine();
			}

			while (active) {

				line = br.readLine();

				while (line != null) {
					handleNewLine(line);
					line = br.readLine();
				}

				try {
					Thread.sleep(200);
				} catch (InterruptedException e) {
					active = false;
					return;
				}
			}
		} catch (IOException e) {
			logger.warn("Got new Exception", e);
		}
	}

	/**
	 * 
	 * We got a new line in the logfile
	 */
	protected void handleNewLine(final String line) {
		final Matcher matcher = pattern.matcher(line);

		// The line contains progress data
		if (matcher.matches()) {

			// Got new Progress series
			if (lastMatches == false) {
				logger.info("Got new progress series");
				
				for(WindowType type : WindowType.values()) {
				
					final AbstractProgressWindow abstractProgressWindow = 
						AppCtx.getInstance().getWindow(type);
					
					if(abstractProgressWindow != null) {
						
						abstractProgressWindow.getGraph()
							.getSeriesOut().clear();
						
					}
				}
			}

			lastMatches = true;

			// Get Progress valued
			int time = Integer.parseInt(matcher.group(1));
			double card = Double.parseDouble(matcher.group(2));
			double estTime = Double.parseDouble(matcher.group(3));
			
			NumberFormat format = NumberFormat.getInstance(Locale.GERMAN);

			try {
				Number number = format.parse(matcher.group(4));
				double progress = number.doubleValue();
				
				// Add the data to the time series
				final TimeSeries progressSeries = AppCtx.getInstance()
					.getWindow(WindowType.PROGRESS).getGraph().getSeriesOut();
				
				final TimeSeries cardSeries = AppCtx.getInstance()
					.getWindow(WindowType.CARD).getGraph().getSeriesOut();
				
				final TimeSeries estTimeSeries = AppCtx.getInstance()
					.getWindow(WindowType.TIME).getGraph().getSeriesOut();

				final SimpleDateFormat formatter 
					= new SimpleDateFormat("yyyy-MM-dd");
				
		        final Date dateStr = formatter.parse("2000-01-01");

				final Date date = new Date(time + dateStr.getTime());
				
				progressSeries.addOrUpdate(new Millisecond(date), progress);
				cardSeries.addOrUpdate(new Millisecond(date), card);
				estTimeSeries.addOrUpdate(new Millisecond(date), estTime);		        
			} catch (ParseException e) {
				logger.warn("Got excetption", e);
			}
		} else {
			lastMatches = false;
		}
		
		// We got a new query 
		if(line.startsWith("'")) {
			AppCtx.getInstance().getLastQuery().setData(line);
		}
	}

	public void setActive(boolean active) {
		this.active = active;
	}
}
