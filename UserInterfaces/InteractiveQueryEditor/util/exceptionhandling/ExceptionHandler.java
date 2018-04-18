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
	 * Public Konstruktor, damit eine Instanz der Klasse als UncaughtExceptionHandler
	 * für das Programm registriert werden kann.
	 */
	public ExceptionHandler() {
		//Ein leerer Standardkonstruktor, weil die Methode uncaughtException
		//aus Thread.UncaughtExceptionHandler eine Instanzmethode ist.
	}

	/**
	 * Zeigt eine Exception mittels der Klasse {@link ErrorDialog} an.
	 * @param owner Das übergeordnete Frame des Dialogs
	 * @param exception Die anzuzeigende Exception
	 * @param additionalMessage Ein optionaler Text
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
		textArea.setPreferredSize(new Dimension(500, 300));
		JOptionPane.showMessageDialog(owner, new JScrollPane(textArea), "An error occured", JOptionPane.ERROR_MESSAGE);
	}

	/**
	 * Formatiert ein Throwable als String. Nutzt dazu die toString() Methode des Objektes
	 * und hängt daran alle StrackTrace Elemente. Sofern die Throwable über einen
	 * auslösenden "cause" verfügt, so ruft sich die Methode rekursiv auf,
	 * um auch den cause formatiert anzuhängen.
	 * @param throwable Die zu formatierende Throwable
	 * @return Die formatierte String Repräsentation der {@link Throwable}
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
	 *
	 */
	@Override
	public void uncaughtException(final Thread t, final Throwable e) {
		final JTextArea textArea = new JTextArea();
		textArea.setText(StringUtilities.appendStrings("An uncaught error occured on the event-dispatching-thread!", getTextRepresentationOfThrowable(e)));
		textArea.setPreferredSize(new Dimension(500, 300));
		JOptionPane.showMessageDialog(null, new JScrollPane(textArea));
	}
}