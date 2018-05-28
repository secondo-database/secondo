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

package application;


import java.awt.Dimension;

import ui.InteractiveQueryEditorFrame;
import ui.InteractiveQueryEditorModel;
import util.common.UITools;
import util.exceptionhandling.ExceptionHandler;
import util.secondo.SecondoFacade;

/**
 * The class Application {@link Application} is responsible to initialize the UI.
 * The main method gets called when the jar is started.
 * @author D.Merle
 */
public class Application {
	private static InteractiveQueryEditorFrame frame;

	/**
	 *
	 */
	private Application() {}

	/**
	 * Initializes the UI and configures the exception handler for uncaught exceptions.
	 * @param args The standard commandline arguments
	 */
	public static void main(final String[] args) {
		try {
			Thread.setDefaultUncaughtExceptionHandler(new ExceptionHandler());
			initializeUI();
			SecondoFacade.initialize("SecondoConfig.ini");
		} catch(final Exception exception) {
			ExceptionHandler.showException(null, exception, "The initialization of the IQE could not be finished. An exception occured.");
		}
	}

	/**
	 * Creates objects of the editor view, controller and model
	 */
	private static void initializeUI() {
		UITools.scaleUI();
		final InteractiveQueryEditorModel model = new InteractiveQueryEditorModel();
		frame = new InteractiveQueryEditorFrame(model);
		final Dimension size = UITools.calculateDimension(75);
		frame.setSize((int)size.getWidth(), (int)size.getHeight());
		frame.setLocation(UITools.calculateCenterPosition(frame.getSize()));
		frame.setVisible(true);
		frame.setDividerLocation(0.65);
	}
}