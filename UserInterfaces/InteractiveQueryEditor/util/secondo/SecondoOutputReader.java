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

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Path;

import javax.swing.SwingUtilities;

import ui.console.ConsolePane;

/**
 * An implementation of runnable which is responsible to read
 * all the output from secondo. After reaching the end of the output file
 * the UI gets notified via the eventdispatching thread
 * @author D.Merle
 */
public class SecondoOutputReader implements Runnable {
	private final ConsolePane editor;
	private final BufferedReader bufferedReader;

	/**
	 * The constructor initializes the input streams.
	 * @param editor The component which gets notified about the new text
	 * @param inputFile The output file of secondo
	 */
	public SecondoOutputReader(final ConsolePane editor, final Path inputFile) {
		this.editor = editor;
		try {
			final FileInputStream fileInputStream = new FileInputStream(inputFile.toFile());
			final InputStreamReader inputStreamReader = new InputStreamReader(fileInputStream, Charset.forName("CP1252"));
			bufferedReader = new BufferedReader(inputStreamReader);
		} catch (final FileNotFoundException e) {
			throw new IllegalArgumentException("The specified file couldn't be found", e);
		}
	}

	/**
	 * Implementation of {@link Runnable#run()}.
	 * Read lines of text from the input file and notifies the UI
	 */
	@Override
	public void run() {
		while (true) {
			try {
				String line = bufferedReader.readLine();
				if (line != null) {
					final StringBuilder buffer = new StringBuilder();

					while (line != null) {
						buffer.append(line).append("\n");
						line = bufferedReader.readLine();
					}

					SwingUtilities.invokeLater(new Runnable() {
						@Override
						public void run() {
							editor.appendTextBeforePrompt(buffer.toString());
						}
					});
				}

				Thread.sleep(50);
			} catch (final IOException e) {
				e.printStackTrace();
			} catch (final InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}