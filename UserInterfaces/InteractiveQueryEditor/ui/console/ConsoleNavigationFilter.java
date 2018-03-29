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

package ui.console;

import javax.swing.text.BadLocationException;
import javax.swing.text.Element;
import javax.swing.text.NavigationFilter;
import javax.swing.text.Position.Bias;

/**
 * Restricts the placement of the cursor. This way the user can't
 * enter text at unwanted positions.
 * @author D.Merle
 */
public class ConsoleNavigationFilter extends NavigationFilter {
	private final ConsoleDocument document;

	public ConsoleNavigationFilter(final ConsoleDocument document) {
		this.document = document;
	}

	@Override
	public void setDot(final FilterBypass fb, final int dot, final Bias bias) {
		final Element root = document.getDefaultRootElement();
		final Element lastLine = root.getElement(root.getElementCount() -1);
		try {
			final String lastLineText = lastLine.getDocument().getText(lastLine.getStartOffset(), lastLine.getEndOffset() - lastLine.getStartOffset());
			int boundary = lastLine.getStartOffset();
			if (lastLineText.startsWith(document.getPrompt())) {
				boundary = lastLine.getStartOffset() + document.getPrompt().length();
			}
			if (dot >= boundary) {
				super.setDot(fb, dot, bias);
			} else {
				super.setDot(fb, boundary, bias);
			}
		} catch (final BadLocationException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void moveDot(final FilterBypass fb, final int dot, final Bias bias) {
		final Element root = document.getDefaultRootElement();
		final Element lastLine = root.getElement(root.getElementCount() -1);
		try {
			final String lastLineText = lastLine.getDocument().getText(lastLine.getStartOffset(), lastLine.getEndOffset() - lastLine.getStartOffset());
			int boundary = lastLine.getStartOffset();
			if (lastLineText.startsWith(document.getPrompt())) {
				boundary = lastLine.getStartOffset() + document.getPrompt().length();
			}
			if (dot >= boundary) {
				super.moveDot(fb, dot, bias);
			} else {
				super.moveDot(fb, boundary, bias);
			}
		} catch (final BadLocationException e) {
			e.printStackTrace();
		}
	}
}