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

package util.secondo;

import sj.lang.ListExpr;

/**
 * The SecondoFacade hides the fact that an initialzation of the underlying SecondoKernel
 * need to be done first. Every method checks prior to the call to the SecondoKernel if the Kernel has been initalized
 * @author D.Merle
 */
public class SecondoFacade {
	private static boolean initalized = false;

	private SecondoFacade() {

	}

	/**
	 * This method need to be called first
	 * @param configPath
	 */
	public static void initialize(final String configPath) {
		initalized = SecondoKernel.initialize(configPath);
		if (!initalized) {
			throw new IllegalStateException("Initialization of the SecondoKernel failed!");
		}
	}

	/**
	 * Send a query to the secondo database system
	 * @param command The query to be executed
	 * @param showResult Defines if secondo will print the result to the output file
	 * @return The result in form of a {@link ListExpr}
	 */
	public static ListExpr query(final String command, final boolean showResult) {
		if (!initalized) {
			throw new IllegalStateException("The SecondoFacade hasn`t yet been initialised");
		}
		return (ListExpr)SecondoKernel.query(command, showResult);
	}

	/**
	 * If the execution of a query resulted in an error it is possible to retrieve the
	 * corresponding error message
	 * @return
	 */
	public static String getErrorMessage() {
		if (!initalized) {
			throw new IllegalStateException("The SecondoFacade hasn`t yet been initialised");
		}
		return SecondoKernel.errorMessage();
	}

	/**
	 * Sets the debug level within secondo
	 * @param debugLevel
	 */
	public static void setDebugLevel(final int debugLevel) {
		if (!initalized) {
			throw new IllegalStateException("The SecondoFacade hasn`t yet been initialised");
		}
		SecondoKernel.setDebugLevel(debugLevel);
	}

	/**
	 * Method which can be used to reinitialize the interface
	 */
	public static void closeInterface() {
		SecondoKernel.shutdown();
		initalized = false;
	}
}