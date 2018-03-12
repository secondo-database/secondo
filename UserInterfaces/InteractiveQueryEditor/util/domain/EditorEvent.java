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

package util.domain;

/**
 * Event to notify other components about relevant status changes of the Editor
 * @author D.Merle
 */
public class EditorEvent {
	public static final String INIT_DB_CONNECTION = "1";
	public static final String COMMAND_UPDATED = "2";
	public static final String CLOSE_DB_CONNECTION = "3";
	public static final String NEW_COMMAND = "4";
	private final Object source;
	private final String eventType;
	private final String command;

	public EditorEvent(final Object source, final String eventType, final String command) {
		this.source = source;
		this.eventType = eventType;
		this.command = command;
	}

	public Object getSource() {
		return source;
	}

	public String getEventType() {
		return eventType;
	}

	public String getCommand() {
		return command;
	}
}