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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

/**
 * Responsible to manage all the information and the state of the {@link AnalyserPanel}
 * @author D.Merle
 */
public class AnalyserModel {
	public static final String DATA_UPDATED = "1";
	public static final String UPDATE_HIGHLIGHTING = "2";
	private final ArrayList<ActionListener> actionListeners;
	private String currentCommand;
	private String typeInformation;
	private String lastValidTypeInformation;
	private String analyserStatusMessage;
	private boolean dbConnectionInitialized = false;
	private final String startMessage = "SecondoTTY IQE has been initialized.\nType 'HELP' to get a list of available commands.\nNo database opened";
	private final String noTypeMessage = "No valid type found";


	public AnalyserModel() {
		actionListeners = new ArrayList<>();
		analyserStatusMessage = startMessage;
		typeInformation = null;
	}

	public String getTypeInformation() {
		return typeInformation;
	}

	public String getLastValidTypeInformation() {
		return lastValidTypeInformation;
	}

	public String getCurrentCommand() {
		return currentCommand;
	}

	public void setCurrentCommand(final String command) {
		this.currentCommand = command;
	}

	public void setTypeInformation(final String typeInformation) {
		this.typeInformation = typeInformation;
	}

	public void setLastValidTypeInformation(final String lastValidTypeInformation) {
		this.lastValidTypeInformation = lastValidTypeInformation;
	}

	public String getAnalyserStatusMessage() {
		return analyserStatusMessage;
	}

	public void setAnalyserStatusMessage(final String analyserStatusMessage) {
		this.analyserStatusMessage = analyserStatusMessage;
	}

	public boolean isDBConnectionInitialized() {
		return dbConnectionInitialized;
	}

	public void setDBConnectionStatus(final boolean status) {
		dbConnectionInitialized = status;
	}

	public String getStartMessage() {
		return startMessage;
	}

	public String getNoTypeMessage() {
		return noTypeMessage;
	}

	public void addActionListener(final ActionListener listener) {
		actionListeners.add(listener);
	}

	private void fireActionEvent(final String command) {
		for (final ActionListener listener: actionListeners) {
			listener.actionPerformed(new ActionEvent(this, 1, command));
		}
	}

	public void notifyDataUpdate() {
		fireActionEvent(DATA_UPDATED);
	}

	public void updateHighlighting() {
		fireActionEvent(UPDATE_HIGHLIGHTING);
	}
}