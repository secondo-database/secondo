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

package ui.analyser;

import javax.swing.JToolBar;
import javax.swing.event.DocumentListener;

import util.common.SearchField;

/**
 * This class encapsulates the whole functionality needed to administrate the SearchField component.
 * The original programm design incorporated several other buttons and functions.
 * @author D.Merle
 */
public class AnalyserToolBar extends JToolBar {
	private final SearchField searchField;

	/**
	 * Creates the AnalyserToolBar as a component.
	 * @param documentListener A class which is intrested in documentevents which are triggered when the user enters text in the search field
	 */
	public AnalyserToolBar(final DocumentListener documentListener) {
		setFloatable(false);

		searchField = new SearchField("Search attribute");
		searchField.setEnabled(false);
		searchField.getDocument().addDocumentListener(documentListener);

		add(searchField);
	}

	/**
	 * Enables or disables the SearchField
	 * @param enabled true to activate to allow user input
	 */
	public void setSearchFieldEnabled(final boolean enabled) {
		searchField.setEnabled(enabled);
	}

	/**
	 * Returns the currently entered text
	 */
	public String getSearchString() {
		final String text = searchField.getText();
		if (text != null && (text.equals(searchField.getStandardNote()) || text.equals(""))) {
			return null;
		}
		return text;
	}
}