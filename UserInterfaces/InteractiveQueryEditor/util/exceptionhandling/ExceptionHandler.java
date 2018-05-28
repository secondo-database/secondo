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

package util.exceptionhandling;

import java.awt.Dimension;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import util.common.StringUtilities;

/**
 * This class implements {@link Thread.UncaughtExceptionHandler}
 * to handle all uncaught exceptions of the event dispatching thread.
 * Implements a few utility methods
 * @author D.Merle
 */
public class ExceptionHandler implements Thread.UncaughtExceptionHandler {

	/**
	 * To implement Thread.UncaughtExceptionHandler this class needs a public constructor
	 * because uncaughtException(final Thread t, final Throwable e) is an instance method.
	 */
	public ExceptionHandler() {

	}

	/**
	 * This method displays an exception in an error dialog
	 * @param owner The the owner or parent of the error dialog
	 * @param exception The exception to be shown
	 * @param additionalMessage An additional error message
	 */
	public static void showException(final JFrame owner, final Throwable exception, final String additionalMessage) {
		String message = null;
		if(StringUtilities.isStringNull(additionalMessage)) {
			message = getTextRepresentationOfThrowable(exception);
		} else {
			message = getTextRepresentationOfThrowable(new Exception(additionalMessage, exception));
		}

		final JTextArea textArea = new JTextArea();
		textArea.setText(message);
		final JScrollPane scrollPane = new JScrollPane(textArea);
		scrollPane.setPreferredSize(new Dimension(500, 300));
		JOptionPane.showMessageDialog(owner, scrollPane, "An error occured", JOptionPane.ERROR_MESSAGE);
	}

	/**
	 * Method to create a proper error text for a Throwable.
	 * It appends all Stacktrace elements and does a recursive call to itself when a Throwable
	 * has another "cause".
	 */
	private static String getTextRepresentationOfThrowable(final Throwable throwable) {
		final StringBuilder builder = new StringBuilder();
		if(throwable != null) {
			builder.append(throwable.toString()).append("\n");
			final StackTraceElement[] stackTraceElements = throwable.getStackTrace();
			for (final StackTraceElement stackTraceElement : stackTraceElements) {
				builder.append(stackTraceElement.toString()).append("\n");
			}
			if(throwable.getCause() != null) {
				builder.append("Caused by:\n");
				builder.append(getTextRepresentationOfThrowable(throwable.getCause()));
			}
		}
		return builder.toString();
	}

	/**
	 * Uncaught exceptions result in a separate error dialog
	 */
	@Override
	public void uncaughtException(final Thread t, final Throwable e) {
		final JTextArea textArea = new JTextArea();
		textArea.setText(StringUtilities.appendStrings("An uncaught error occured on the event-dispatching-thread!", getTextRepresentationOfThrowable(e)));
		final JScrollPane scrollPane = new JScrollPane(textArea);
		scrollPane.setPreferredSize(new Dimension(500, 300));
		JOptionPane.showMessageDialog(null, scrollPane);
	}
}