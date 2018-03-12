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

package util.common;

import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;

import javax.swing.JTextField;

/**
 * This component is {@link JTextField} which manages a text which is shown when the
 * component doesn't have the focus and the user hasn't entered some text.
 * @author D.Merle
 */
public class SearchField extends JTextField implements FocusListener {
	private final String standardNote;

	/**
	 *
	 * @param standardNote
	 */
	public SearchField(final String standardNote) {
		super(standardNote);
		this.standardNote = standardNote;
		addFocusListener(this);
	}

	/**
	 *
	 */
	@Override
	public void focusGained(final FocusEvent e) {
		final String text = getText();
		if (text != null && text.trim().equals(standardNote)) {
			setText("");
		}
	}

	/**
	 *
	 */
	@Override
	public void focusLost(final FocusEvent e) {
		final String text = getText();
		if (text != null && text.trim().equals("")) {
			setText(standardNote);
		}
	}

	/**
	 *
	 * @return
	 */
	public String getStandardNote() {
		return standardNote;
	}
}