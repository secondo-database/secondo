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

import java.util.HashMap;
import java.util.Map;

/**
 * 
 * This class is our application context
 * Some global references are stored here 
 * 
 */ 
public class AppCtx {
	
	protected static AppCtx instance;

	protected FileWatcherWorker worker;
	
	protected ObservableString lastQuery = new ObservableString();
	
	protected Map<WindowType, AbstractProgressWindow> windows 
		= new HashMap<WindowType, AbstractProgressWindow>();
	
	protected Map<WindowType, AbstractTableWindow> queryProgressTableWindow
		= new HashMap<WindowType, AbstractTableWindow>();
	
	public static synchronized AppCtx getInstance() {
		if(instance == null) {
			instance = new AppCtx();
		}
		
		return instance;
	}
	
	private AppCtx() {
		// private constructor
	}
	
	protected Object clone() throws CloneNotSupportedException {
		throw new CloneNotSupportedException("Unable to clone a singleton!");
	}

	public FileWatcherWorker getWorker() {
		return worker;
	}

	public void setWorker(final FileWatcherWorker controllerWorker) {
		this.worker = controllerWorker;
	}

	// Get the last executed query
	public ObservableString getLastQuery() {
		
		if(lastQuery == null) {
			lastQuery = new ObservableString();
		}
		
		return lastQuery;
	}
	
	/**
	 * Returns a reference to the given window
	 */
	public AbstractProgressWindow getWindow(final WindowType windowType) {
		return windows.get(windowType);
	}
	
	/**
	 * Set a reference to the given window
	 */
	public void setWindow(final WindowType windowType, final AbstractProgressWindow window) {
		windows.put(windowType, window);
	}

	/**
	 * Set a reference to a progress window
	 */
	public void setQueryProgressTableWindow(final WindowType type, 
			final AbstractTableWindow window) {
		queryProgressTableWindow.put(type, window);
	}
	
	/**
	 * Returns a reference to a progress window
	 */
	public AbstractTableWindow getQueryProgressTableWindow(final WindowType type) {
		return queryProgressTableWindow.get(type);
	}
	
}