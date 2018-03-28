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

import java.awt.Color;

import javax.swing.JTextPane;
import javax.swing.text.AbstractDocument;
import javax.swing.text.BadLocationException;
import javax.swing.text.Element;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;

import util.common.UITools;
import util.common.UpdateHandler;

public class ConsolePane extends JTextPane implements UpdateHandler {
	private final SimpleAttributeSet standardTextAttribute;

	public ConsolePane() {
		super();
		final ConsoleDocument document = new ConsoleDocument();
		setStyledDocument(document);
		standardTextAttribute = new SimpleAttributeSet();
		StyleConstants.setBackground(standardTextAttribute, Color.WHITE);
		StyleConstants.setForeground(standardTextAttribute, Color.BLACK);
		StyleConstants.setFontFamily(standardTextAttribute, "Monospaced");
		StyleConstants.setFontSize(standardTextAttribute, (int)(12*UITools.calculateUIScaling()));

		setBackground(Color.WHITE);
		setForeground(Color.BLACK);
		setCaretColor(Color.BLACK);
		document.setDocumentFilter(new ConsoleDocumentFilter(1000));
		setNavigationFilter(new ConsoleNavigationFilter(document));
	}

	public void replaceLastLine(final String text) {
		final ConsoleDocument doc = (ConsoleDocument)getDocument();
		final Element root = getDocument().getDefaultRootElement();
		final Element lastLine = root.getElement(root.getElementCount() -1);
		try {
			doc.remove(lastLine.getStartOffset() + doc.getPrompt().length(), lastLine.getEndOffset() - lastLine.getStartOffset() - doc.getPrompt().length() - 1);
			doc.insertString(lastLine.getStartOffset() + doc.getPrompt().length(), text, standardTextAttribute);
		} catch (final BadLocationException e) {
			e.printStackTrace();
		}
	}

	public void appendString(final String text) {
		final ConsoleDocument doc = (ConsoleDocument)getDocument();
		final Element root = getDocument().getDefaultRootElement();
		final Element lastLine = root.getElement(root.getElementCount() -1);
		try {
			doc.insertString(lastLine.getEndOffset() - 1, doc.getPrompt() + text, standardTextAttribute);
		} catch (final BadLocationException e) {
			e.printStackTrace();
		}
		setCaretPosition(getDocument().getLength());
	}

	public void appendTextBeforePrompt(final String text) {
		final ConsoleDocument doc = (ConsoleDocument)getDocument();
		final Element root = getDocument().getDefaultRootElement();
		final Element insertAt = root.getElement(root.getElementCount() -1);
		try {
			doc.insertString(insertAt.getStartOffset(), text, standardTextAttribute);
		} catch (final BadLocationException e) {
			e.printStackTrace();
		}
	}

	public void setPrompt(final String prompt) {
		((ConsoleDocument)getDocument()).setPrompt(prompt);
		appendString("");
	}

	public String getLineFromEndOfDocument(final int subtract) {
		final ConsoleDocument doc = (ConsoleDocument)getDocument();
		final Element root = getDocument().getDefaultRootElement();
		final Element lastLine = root.getElement(root.getElementCount() - subtract);
		try {
			return doc.getText(lastLine.getStartOffset() + doc.getPrompt().length(), lastLine.getEndOffset() - lastLine.getStartOffset() - doc.getPrompt().length() - 1);
		} catch (final BadLocationException e) {
			e.printStackTrace();
			return "";
		}
	}

	public void addDocumentFilterInterceptor(final ConsoleDocumentFilterInterceptor interceptor) {
		((ConsoleDocumentFilter)((AbstractDocument)getDocument()).getDocumentFilter()).setInterceptor(interceptor);
	}

	@Override
	public void handleUpdate(final String text) {
		appendTextBeforePrompt(text);
	}
}