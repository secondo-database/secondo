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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import sj.lang.ListExpr;
import util.domain.enums.EditorCommand;

/**
 * Manages the entered commands and the state of the {@link InteractiveQueryEditorFrame}
 * @author D.Merle
 */
public class InteractiveQueryEditorModel {
	public static final String SHOW_HELP = "1";
	public static final String PROMPT_UPDATED = "3";

	private final ArrayList<String> inputHistory;
	private int historyIndex = 0;
	private final ArrayList<ActionListener> actionListeners;
	private final String helpMessage;
	private int debugLevel = 0;
	private StringBuilder currentCommand;
	private String commandFragment;
	private ListExpr result;
	private final String[] keywords;

	private int repeatCommandCounter = 1;
	private String prompt;

	public InteractiveQueryEditorModel() {
		prompt = "Secondo=>";
		inputHistory = new ArrayList<>();
		historyIndex = inputHistory.size();
		actionListeners = new ArrayList<>();
		helpMessage = createHelpMessage();
		currentCommand = new StringBuilder();
		keywords = new String[] { "abort", "algebra", "algebras", "begin", "commit",
				"close", "constructors", "consume","count", "create",
				"database", "databases", "DEBUG", "delete", "derive",
				"else","endif","endwhile","extend", "feed", "filter",
				"from", "if", "kill", "let", "list","objects", "open",
				"operators", "query","restore", "save", "SHOW", "then",
				"transaction", "type","types", "update","while",
				"SEC2TYPEINFO","SEC2OPERATORUSAGE","SEC2OPERATORINFO",
				"SEC2FILEINFO","SEC2COUNTERS","SEC2COMMANDS","SEC2CACHEINFO"};
	}

	private String createHelpMessage() {
		final StringBuilder helpMessage = new StringBuilder();
		helpMessage.append("The following internal commands are available:\n");
		for (final EditorCommand editorCommand : EditorCommand.values()) {
			final String[] commands = editorCommand.getCommandList();
			for (int i = 0; i < commands.length; i++) {
				helpMessage.append(commands[i]);
				if (i < commands.length-1) {
					helpMessage.append(", ");
				}
			}
			helpMessage.append(" - ").append(editorCommand.getDescription()).append("\n");
		}
		helpMessage.append("Moreover, you may enter any valid SECONDO command introduced by the\n");
		helpMessage.append("keywords 'query', 'let', 'restore', etc. Refer to the \"User Manual\" for\n");
		helpMessage.append("details. Internal commands are restricted to ONE line, while SECONDO\n");
		helpMessage.append("commands may span several lines; a semicolon as the last character on\n");
		helpMessage.append("a line terminates a command, but is not part of the command.\n");
		helpMessage.append("Alternatively, you may enter an empty line.\n");
		return helpMessage.toString();
	}

	public int getRepeatCommandCounter() {
		return repeatCommandCounter;
	}

	public void setRepeatCommandCounter(final int repeatCommandCounter) {
		this.repeatCommandCounter = repeatCommandCounter;
	}

	public void addActionListener(final ActionListener listener) {
		actionListeners.add(listener);
	}

	private void fireActionEvent(final String command) {
		for (final ActionListener listener: actionListeners) {
			listener.actionPerformed(new ActionEvent(this, 1, command));
		}
	}

	public void showHelpMessage() {
		fireActionEvent(SHOW_HELP);
	}

	public String getHelpMessage() {
		return helpMessage;
	}

	public void setDebugLevel(final int debugLevel) {
		this.debugLevel = debugLevel;
	}

	public int getDebugLevel() {
		return debugLevel;
	}

	public void appendCurrentCommand(final String text) {
		currentCommand.append(text);
	}

	public String getCurrentCommand() {
		return currentCommand.toString();
	}

	public void resetCurrentCommand() {
		currentCommand = new StringBuilder();
	}

	public void setCommandFragment(final String line) {
		commandFragment = line;
	}

	public String getCommandFragment() {
		return commandFragment.toString();
	}

	public void setResult(final ListExpr result) {
		this.result = result;
	}

	public ListExpr getResult() {
		return result;
	}

	public String getPreviousCommand() {
		historyIndex--;
		if (historyIndex >= 0) {
			return inputHistory.get(historyIndex);
		} else {
			historyIndex = -1;
			return "";
		}
	}

	public String getNextCommand() {
		historyIndex++;
		if (historyIndex < inputHistory.size() - 1) {
			return inputHistory.get(historyIndex);
		} else {
			historyIndex = inputHistory.size();
			return "";
		}
	}

	public void addCommandToHistory(final String command) {
		inputHistory.add(command);
		historyIndex = inputHistory.size();
	}

	public void setPrompt(final String prompt) {
		this.prompt = prompt;
		fireActionEvent(PROMPT_UPDATED);
	}

	public String getPrompt() {
		return prompt;
	}

	public String[] getKeywords() {
		return keywords;
	}
}