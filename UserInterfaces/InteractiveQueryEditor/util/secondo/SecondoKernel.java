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

import util.exceptionhandling.ExceptionHandler;

/**
 * This class acts as a method stub defining all the methods which will result
 * in a native call to methods within the loaded library.
 * @author D.Merle
 */
class SecondoKernel {

	static {
		String libraryName = "";
		try {
			libraryName = "SecondoKernel";//Will result in an attempt to load libSecondoKernel.so
			System.loadLibrary(libraryName);
		} catch (final Throwable e) {
			ExceptionHandler.showException(null, e, String.format("The library %s could not be loaded.", libraryName));
		}
	}

	/**
	 *
	 */
	private SecondoKernel() {}

	/**
	 *
	 * @param configPath
	 * @return
	 */
	public static native boolean initialize(String configPath);

	/**
	 *
	 * @param command
	 * @return
	 */
	public static native Object query(String command, boolean showResult);

	/**
	 *
	 * @param debugLevel
	 */
	public static native void setDebugLevel(int debugLevel);

	/**
	 *
	 * @return
	 */
	public static native String errorMessage();

	/**
	 *
	 */
	public static native void shutdown();
}
