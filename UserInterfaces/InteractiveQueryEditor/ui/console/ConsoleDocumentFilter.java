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

import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.DocumentFilter;
import javax.swing.text.Element;

/**
 * Responsible to check if a KeyEvent should be processed by the {@link ConsolePane}.
 * Furthermore it manages the number of shown text in the {@link ConsolePane}
 * @author D.Merle
 */
public class ConsoleDocumentFilter extends DocumentFilter {
	private final int maximumDocumentSize;
	private ConsoleDocumentFilterInterceptor interceptor;

	public ConsoleDocumentFilter(final int maximumSize) {
		maximumDocumentSize = maximumSize;
	}

	/**
	 * The insertString method only gets called by the program itself.
	 * For example when text gets appended to the document.
	 */
	@Override
	public void insertString(final FilterBypass fb, final int offset, final String string, final AttributeSet attr) throws BadLocationException {
		final Element root = fb.getDocument().getDefaultRootElement();
		fb.insertString(offset, string, attr);
		checkAndAdjustDocumentSize(root, fb);
	}

	/**
	 * This method gets called when the user deletes text in the corresponding ui component
	 */
	@Override
	public void remove(final FilterBypass fb, final int offset, final int length) throws BadLocationException {
		final ConsoleDocument doc = (ConsoleDocument)fb.getDocument();
		final Element root = doc.getDefaultRootElement();
		final Element lastLine = root.getElement(root.getElementCount() -1);
		final String lastLineText = lastLine.getDocument().getText(lastLine.getStartOffset(), lastLine.getEndOffset() - lastLine.getStartOffset());
		int boundary = lastLine.getStartOffset();
		if (lastLineText.startsWith(doc.getPrompt())) {
			boundary = lastLine.getStartOffset() + doc.getPrompt().length();
		}
		if (offset >= boundary) {
			fb.remove(offset, length);
			if (interceptor != null) {
				interceptor.remove(fb, offset, length);
			}
		}
	}

	/**
	 * This method gets called whenever the user inserts or replaces text
	 */
	@Override
	public void replace(final FilterBypass fb, final int offset, final int length, final String text, final AttributeSet attrs) throws BadLocationException {
		final ConsoleDocument doc = (ConsoleDocument)fb.getDocument();
		final Element root = doc.getDefaultRootElement();
		final Element lastLine = root.getElement(root.getElementCount() -1);
		final String lastLineText = lastLine.getDocument().getText(lastLine.getStartOffset(), lastLine.getEndOffset() - lastLine.getStartOffset());
		int boundary = offset;
		if (lastLineText.startsWith(doc.getPrompt())) {
			boundary = lastLine.getStartOffset() + doc.getPrompt().length();
		}
		if (offset >= boundary) {
			if ("\n".equals(text)) {// if the user presses enter within the currently active command line it gets appended to the end
				fb.replace(doc.getLength(), 0, text, attrs);//this is done to reproduce the behaviour of a console
			} else {
				fb.replace(offset, length, text, attrs);
			}
			if (interceptor != null) {
				interceptor.replace(fb, offset, length, text, attrs);
			}
			checkAndAdjustDocumentSize(root, fb);
		}
	}

	private void checkAndAdjustDocumentSize(final Element root, final FilterBypass fb) throws BadLocationException {
		while (root.getElementCount() > maximumDocumentSize) {
			final Element firstLine = root.getElement(0);
			fb.remove(0, firstLine.getEndOffset());
		}
	}

	public void setInterceptor(final ConsoleDocumentFilterInterceptor interceptor) {
		this.interceptor = interceptor;
	}
}