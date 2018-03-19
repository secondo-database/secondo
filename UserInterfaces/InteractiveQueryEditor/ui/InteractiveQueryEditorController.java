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

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.math.BigDecimal;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.regex.Pattern;

import javax.swing.text.AttributeSet;
import javax.swing.text.DocumentFilter.FilterBypass;

import sj.lang.ListExpr;
import ui.console.ConsoleDocumentFilterInterceptor;
import util.domain.EditorEvent;
import util.domain.EditorEventListener;
import util.domain.enums.EditorCommand;
import util.secondo.SecondoFacade;

/**
 * Handles all the events of the {@link InteractiveQueryEditorFrame} and the utilized components
 * @author D.Merle
 */
public class InteractiveQueryEditorController implements ConsoleDocumentFilterInterceptor {
	private final String historyFileName = ".iqe_secondo_history";
	private final int maxHistoryFileEntries = 200;
	private final ArrayList<EditorEventListener> listeners;
	private final InteractiveQueryEditorFrame frame;
	private final InteractiveQueryEditorModel model;

	public InteractiveQueryEditorController(final InteractiveQueryEditorFrame frame, final InteractiveQueryEditorModel model) {
		listeners = new ArrayList<>();
		this.frame = frame;
		this.model = model;
		initalizeHistory();
	}

	@Override
	public void replace(final FilterBypass fb, final int offset, final int length, final String string, final AttributeSet attr) {
		processDocumentEvent(string);
	}

	@Override
	public void remove(final FilterBypass fb, final int offset, final int length) {
		processDocumentEvent("");
	}

	private void processDocumentEvent(final String text) {
		if ("\n".equals(text)) {
			handleEnter();
		} else if ("\t".equals(text)) {
			handleTab();
		} else {
			final String currentCommandLine = frame.getEditor().getLineFromEndOfDocument(1);
			model.setCommandFragment(currentCommandLine);
			fireEditorEvent(EditorEvent.COMMAND_UPDATED);
		}
	}

	private void handleEnter() {
		String currentCommandLine;
		currentCommandLine = frame.getEditor().getLineFromEndOfDocument(2);//An Enter events is immediately processed by a JTextPane. Therefore we need to get the second to last line of text.
		model.setCommandFragment(currentCommandLine);
		if (!currentCommandLine.equals("")) {
			model.appendCurrentCommand(currentCommandLine);
		}

		String command = model.getCurrentCommand().trim();
		if (command.endsWith(";") || currentCommandLine.equals("")) {
			if (command.endsWith(";")) {
				command = removeCommandDelimiter(command);
			}
			if (!command.equals("")) {
				model.addCommandToHistory(command);
				writeToHistoryFile(command);
			}
			executeCommand(command);
			model.setPrompt("Secondo=>");
			model.resetCurrentCommand();
			fireEditorEvent(EditorEvent.NEW_COMMAND);
		} else {
			model.appendCurrentCommand(" ");
			model.setPrompt("Secondo->");
		}
	}

	private void handleTab() {
		String currentCommandLine = frame.getEditor().getLineFromEndOfDocument(1);
		String enteredText = "";
		int beginIndex = currentCommandLine.indexOf("\t");
		final Pattern onlyAplhabetic = Pattern.compile("\\w");

		if (beginIndex > 0) {
			String character = currentCommandLine.substring(beginIndex - 1, beginIndex);
			while (onlyAplhabetic.matcher(character).find()) {
				enteredText = character + enteredText;
				beginIndex--;
				if (beginIndex > 0) {
					character = currentCommandLine.substring(beginIndex - 1, beginIndex);
				} else {
					break;
				}
			}
		}

		final ArrayList<String> foundKeywords = new ArrayList<>();
		String[] keywords = null;
		keywords = model.getKeywords();
		for (int i = 0; i < keywords.length; i++) {
			if (keywords[i].startsWith(enteredText)) {
				foundKeywords.add(keywords[i]);
			}
		}

		if (foundKeywords.size() == 0) {
			frame.getEditor().replaceLastLine(currentCommandLine.replace("\t", ""));
		} else if (foundKeywords.size() == 1) {
			frame.getEditor().replaceLastLine(currentCommandLine.replace(enteredText + "\t", foundKeywords.get(0)));
		} else {
			String greatestCommonSubstring = "";
			final String firstFoundWord = foundKeywords.get(0);
			for (int i = 0; i < firstFoundWord.length(); i++) {
				final String part = firstFoundWord.substring(0, i +1);
				boolean fitsAll = true;
				for (int j = 0; j < foundKeywords.size(); j++) {
					if (!foundKeywords.get(j).startsWith(part)) {
						fitsAll = false;
						break;
					}
				}

				if (fitsAll) {
					greatestCommonSubstring = part;
				}
			}

			if (greatestCommonSubstring.equals(enteredText)) {
				for (int i = 0; i < foundKeywords.size(); i++) {
					if (i % 3 == 0) {
						System.out.printf("%n");
					}
					System.out.printf("%-19s", foundKeywords.get(i));
				}
				System.out.printf("%n");
			}
			frame.getEditor().replaceLastLine(currentCommandLine.replace(enteredText + "\t", greatestCommonSubstring));
		}

		currentCommandLine = frame.getEditor().getLineFromEndOfDocument(1);
		model.setCommandFragment(currentCommandLine);
		fireEditorEvent(EditorEvent.COMMAND_UPDATED);
	}

	public void setPreviousCommand() {
		frame.getEditor().replaceLastLine(model.getPreviousCommand());
	}

	public void setNextCommand() {
		frame.getEditor().replaceLastLine(model.getNextCommand());
	}

	private void executeCommand(String command) {
		final EditorCommand editorCommand = checkInputForEditorCommand(command);
		if (editorCommand != null) {
			handleEditorCommand(editorCommand, command);
			if (editorCommand == EditorCommand.REPEAT) {//Repeat ist eine Ausnahme, weil repeat ein query beinhaltet, das auch mehrzeilig angegeben werden kann
				command = removeRepeatFromCommand(command);
			} else {
				return;
			}
		}

		final int repeatCommandCounter = model.getRepeatCommandCounter();
		for (int i = 0; i < repeatCommandCounter; i++) {
			final StringTokenizer tokenizer = new StringTokenizer(command, "|");
			while (tokenizer.hasMoreTokens()) {
				final String singleCommand = tokenizer.nextToken().trim();
				final ListExpr result = processCommand(singleCommand);
				model.setResult(result);
				if (result != null) {
					if (singleCommand.startsWith("open database")) {
						fireEditorEvent(EditorEvent.INIT_DB_CONNECTION);
					} else if (command.startsWith("close database")) {
						fireEditorEvent(EditorEvent.CLOSE_DB_CONNECTION);
					}
				}
			}
		}
	}

	private String removeRepeatFromCommand(final String command) {
		final Pattern pattern = Pattern.compile("\\Arepeat \\d", Pattern.CASE_INSENSITIVE);
		return pattern.matcher(command).replaceFirst("");
	}

	private void handleEditorCommand(final EditorCommand command, final String input) {
		switch (command) {
		case HELP:
			showHelpMessage();
			break;
		case REPEAT:
			model.setRepeatCommandCounter(parseRepeatCounter(input));
			break;
		case DEBUG:
			setDebugLevel(parseDebugLevel(input));
			break;
		case QUIT:
			exitProgramm();
			break;
		default:
			break;
		}
	}

	private void showHelpMessage() {
		model.showHelpMessage();
	}

	private int parseRepeatCounter(final String input) {
		final StringTokenizer tokenizer = new StringTokenizer(input);
		tokenizer.nextToken();
		final String counter = tokenizer.nextToken();
		return new BigDecimal(counter).intValue();
	}

	private int parseDebugLevel(final String input) {
		final String purgedCommand = input.replaceAll("[^0-9]", "");
		final BigDecimal debugLevel = new BigDecimal(purgedCommand);
		return debugLevel.intValue();
	}

	private void setDebugLevel(final int debugLevel) {
		model.setDebugLevel(debugLevel);
		SecondoFacade.setDebugLevel(debugLevel);
	}

	private void exitProgramm() {
		System.exit(0);
	}

	/**
	 * Die Methode entfernt das letzte Zeichen eines Strings.
	 * Wird dazu genutzt ";" oder "\n" aus dem eingegebenen Befehl zu entfernen.
	 * @param command
	 * @return
	 */
	private String removeCommandDelimiter(final String command) {
		if(command.length() >= 1) {
			return command.substring(0, command.length()-1);
		}
		return command;
	}

	private ListExpr processCommand(final String command) {
		return SecondoFacade.query(command, true);
	}

	private EditorCommand checkInputForEditorCommand(final String input) {
		for (final EditorCommand editorCommand : EditorCommand.values()) {
			for (final String command : editorCommand.getCommandList()) {
				final StringTokenizer commandTokenizer = new StringTokenizer(command);
				if (commandTokenizer.countTokens() > 1) {
					if (input.toLowerCase().startsWith(commandTokenizer.nextToken().toLowerCase())) {
						return editorCommand;
					}
				} else if(commandTokenizer.countTokens() == 1) {
					final StringTokenizer inputTokenizer = new StringTokenizer(input);
					if (inputTokenizer.hasMoreTokens() && inputTokenizer.nextToken().equalsIgnoreCase(command.toLowerCase())) {
						return editorCommand;
					}
				}
			}
		}
		return null;
	}

	private void initalizeHistory() {
		try {
			final Path historyFile = Paths.get(historyFileName);
			if (!Files.exists(historyFile)) {
				Files.createFile(historyFile);
			}

			final ArrayList<String> historyCommands = new ArrayList<>();
			try (FileReader fileReader = new FileReader(historyFile.toFile())) {
				try (BufferedReader bufferedReader = new BufferedReader(fileReader)) {
					String line = bufferedReader.readLine();
					while (line != null) {
						historyCommands.add(line);
						model.addCommandToHistory(line);
						line = bufferedReader.readLine();
					}

				}
			}

			if (historyCommands.size() > maxHistoryFileEntries) {
				clearHistory(historyFile, historyCommands);
			}
		} catch (final IOException e) {
			e.printStackTrace();//TODO log?
		}
	}

	private void clearHistory(final Path historyFile, final ArrayList<String> historyCommands) {
		try {
			final Path newHistoryFile = Paths.get(".iqe_secondo_history_temp");
			if (!Files.exists(newHistoryFile)) {
				Files.createFile(newHistoryFile);
			}
			try (FileWriter fileWriter = new FileWriter(newHistoryFile.toFile())) {
				try (BufferedWriter bufferedWriter = new BufferedWriter(fileWriter)) {
					for (int i = historyCommands.size() - maxHistoryFileEntries; i < historyCommands.size(); i++) {
						bufferedWriter.write(historyCommands.get(i)  + "\n");
					}
				}
			}
			Files.move(newHistoryFile, newHistoryFile.resolveSibling(historyFile.getFileName()), StandardCopyOption.REPLACE_EXISTING);
		} catch (final IOException e) {
			e.printStackTrace();//TODO log?
		}
	}

	private void writeToHistoryFile(final String command) {
		try {
			final Path historyFile = Paths.get(historyFileName);
			if (!Files.exists(historyFile)) {
				Files.createFile(historyFile);
			}

			final FileWriter fileWriter = new FileWriter(historyFile.toFile(), true);
			fileWriter.write(command + "\n");
			fileWriter.close();
		} catch (final IOException e) {
			// TODO log?
			e.printStackTrace();
		}
	}

	public boolean addEditorEventListener(final EditorEventListener listener) {
		return listeners.add(listener);
	}

	/**
	 * Methode zum Dispatch von Ereignissen an alle Listener.
	 */
	private void fireEditorEvent(final String eventType) {
		final String command = model.getCurrentCommand() + model.getCommandFragment();
		final EditorEvent event = new EditorEvent(this, eventType, command);
		for (final EditorEventListener listener : listeners) {
			listener.handleEditorEvent(event);
		}
	}
}