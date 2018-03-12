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

package ui;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowEvent;
import java.nio.file.Paths;

import javax.swing.AbstractAction;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.KeyStroke;
import javax.swing.WindowConstants;

import ui.analyser.AnalyserModel;
import ui.analyser.AnalyserPanel;
import ui.console.ConsolePane;
import util.secondo.SecondoOutputReader;

/**
 * The main frame of the UI
 * @author D.Merle
 */
public class InteractiveQueryEditorFrame extends JFrame implements ActionListener {
	private final InteractiveQueryEditorModel model;
	private final InteractiveQueryEditorController controller;
	private ConsolePane editor;
	private AnalyserPanel analyser;
	private JSplitPane frameDivider;

	public InteractiveQueryEditorFrame (final InteractiveQueryEditorModel model) {
		super("SecondoTTYIQE - Interactive Query Editor");
		this.model = model;
		controller = new InteractiveQueryEditorController(this, model);
		initComponents();
		updateEditorPrompt();
		model.addActionListener(this);
		defineClosingBehaviour();
	}

	private void initComponents() {
		frameDivider = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
		add(frameDivider, BorderLayout.CENTER);

		editor = new ConsolePane();
		editor.getDocument().addDocumentListener(controller);
		analyser = new AnalyserPanel(this, new AnalyserModel());
		addKeyBindings();
		controller.addEditorEventListener(analyser.getController());
		frameDivider.setLeftComponent(new JScrollPane(editor));
		frameDivider.setRightComponent(new JScrollPane(analyser));

		final SecondoOutputReader readerRunnable = new SecondoOutputReader(editor, Paths.get("secondoOut.txt"));
		final Thread continuouslyReadSecondoOutput = new Thread(readerRunnable);
		continuouslyReadSecondoOutput.start();

		editor.requestFocus();
	}

	private void addKeyBindings() {
		final KeyStroke arrowUp = KeyStroke.getKeyStroke(KeyEvent.VK_UP, 0);
		editor.getInputMap(JComponent.WHEN_FOCUSED).put(arrowUp, "PREVIOUS_COMMAND");
		final AbstractAction previousCommandAction = new AbstractAction() {
			@Override
			public void actionPerformed(final ActionEvent e) {
				controller.setPreviousCommand();
			}
		};
		editor.getActionMap().put("PREVIOUS_COMMAND", previousCommandAction);

		final KeyStroke arrowDown = KeyStroke.getKeyStroke(KeyEvent.VK_DOWN, 0);
		editor.getInputMap(JComponent.WHEN_FOCUSED).put(arrowDown, "NEXT_COMMAND");
		final AbstractAction nextCommandAction = new AbstractAction() {
			@Override
			public void actionPerformed(final ActionEvent e) {
				controller.setNextCommand();
			}
		};
		editor.getActionMap().put("NEXT_COMMAND", nextCommandAction);
	}

	/**
	 * Die Methode registriert VK_ESCAPE in der InputMap des Dialogs, so dass bei Betätigung
	 * von Escape ein HIDE_ON_CLOSE Event ausgelöst wird.<br>
	 * Der Dialog wird minimiert
	 */
	private void defineClosingBehaviour() {
		final KeyStroke escapeKeyStroke = KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0);
		final AbstractAction closeAction = new AbstractAction() {
			@Override
			public void actionPerformed(final ActionEvent e) {
				dispatchEvent(new WindowEvent(InteractiveQueryEditorFrame.this, WindowEvent.WINDOW_CLOSING));
			}
		};
		getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(escapeKeyStroke, "CLOSEDIALOG");
		getRootPane().getActionMap().put("CLOSEDIALOG", closeAction);

		setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
	}

	public void setDividerLocation(final double proportionalLocation) {
		frameDivider.setDividerLocation(proportionalLocation);
	}

	@Override
	public void actionPerformed(final ActionEvent e) {
		switch (e.getActionCommand()) {
		case InteractiveQueryEditorModel.SHOW_HELP:
			showHelpMessage();
			break;
		case InteractiveQueryEditorModel.PROMPT_UPDATED:
			updateEditorPrompt();
			break;
		}
	}

	private void showHelpMessage() {
		editor.appendTextBeforePrompt(model.getHelpMessage());
	}

	private void updateEditorPrompt() {
		editor.setPrompt(model.getPrompt());
	}

	public ConsolePane getEditor() {
		return editor;
	}
}