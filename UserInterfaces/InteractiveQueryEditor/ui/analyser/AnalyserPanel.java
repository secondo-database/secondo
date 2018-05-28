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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextPane;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;

/**
 * The UI representation of the analyser
 * @author D.Merle
 */
public class AnalyserPanel extends JPanel implements ActionListener {
	private final AnalyserController controller;
	private final AnalyserModel model;
	private AnalyserToolBar toolbar;
	private JTextPane typePane;
	private final SimpleAttributeSet highlightingTextAttribute;
	private final SimpleAttributeSet standardTextAttribute;
	private final SimpleAttributeSet corruptedCommandTextAttribute;

	public AnalyserPanel(final JFrame parent, final AnalyserModel frameModel) {
		this.model = frameModel;
		model.addActionListener(this);
		controller = new AnalyserController(parent, model);

		highlightingTextAttribute = new SimpleAttributeSet();
		StyleConstants.setBackground(highlightingTextAttribute, Color.YELLOW);
		standardTextAttribute = new SimpleAttributeSet();
		StyleConstants.setBackground(standardTextAttribute, Color.WHITE);
		corruptedCommandTextAttribute = new SimpleAttributeSet();
		StyleConstants.setForeground(corruptedCommandTextAttribute, Color.RED);

		initComponents();
	}

	private void initComponents() {
		setLayout(new BorderLayout());
		toolbar = new AnalyserToolBar(controller);
		add(toolbar, BorderLayout.NORTH);
		typePane = new JTextPane();
		typePane.setText(model.getAnalyserStatusMessage());
		typePane.setEditable(false);
		add(typePane, BorderLayout.CENTER);
	}

	/**
	 * This method acitvates or deactivates components showing in the UI
	 * considering the results of the analysis.
	 * Currently the analyser does not notify the UI about occurred errors.
	 * If the analyser would set model.getTypeInformation() == null then the
	 * panel would display the type information in red.
	 */
	@Override
	public void actionPerformed(final ActionEvent e) {
		final String actionCommand = e.getActionCommand();
		if (actionCommand.equals(AnalyserModel.DATA_UPDATED)) {
			if (model.getTypeInformation() != null) {
				toolbar.setSearchFieldEnabled(true);
				typePane.setText(model.getTypeInformation());
				highlightSearchedText();
			} else {
				if (model.getLastValidTypeInformation() != null) {
					toolbar.setSearchFieldEnabled(true);
					typePane.setText(model.getLastValidTypeInformation());
					signalCorruptCommand();
				} else {
					toolbar.setSearchFieldEnabled(false);
					typePane.setText(model.getAnalyserStatusMessage());
				}
			}
		} else if (actionCommand.equals(AnalyserModel.UPDATE_HIGHLIGHTING)) {
			highlightSearchedText();
		}
	}

	private void highlightSearchedText() {
		final String searchString = toolbar.getSearchString();
		final StyledDocument document = typePane.getStyledDocument();
		final String text = typePane.getText();
		document.setCharacterAttributes(0, text.length(), standardTextAttribute, false);
		if (searchString != null) {
			final Pattern pattern = Pattern.compile("(" + searchString + ")", Pattern.CASE_INSENSITIVE);
			final Matcher matcher = pattern.matcher(text);
			while (matcher.find()) {
				document.setCharacterAttributes(matcher.start(1), matcher.end(1) - matcher.start(1), highlightingTextAttribute, false);
			}
		}
	}

	private void signalCorruptCommand() {
		final StyledDocument document = typePane.getStyledDocument();
		final String text = typePane.getText();
		document.setCharacterAttributes(0, text.length(), corruptedCommandTextAttribute, false);
	}

	public AnalyserController getController() {
		return controller;
	}
}