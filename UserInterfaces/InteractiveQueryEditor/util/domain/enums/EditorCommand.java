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

package util.domain.enums;

/**
 * Each entry representes build-in commands of the editor
 * @author D.Merle
 */
public enum EditorCommand {
	HELP("display this message", "?", "HELP"),
	DEBUG("set debug level to n where n is an integer where each\n" +
			"\tbit corresponds to one setting:\n" +
			"\tbit  0: debug mode (show annotated query and operator tree)\n" +
			"\tbit  1: trace (show recursive calls)\n" +
			"\tbit  2: trace nodes (construction of nodes of the op. tree,\n" +
			"\tand execution of the query processor's Eval() method)\n" +
			"\tbit  3: localInfo (prints a warning if an operator did not\n" +
			"\tdestroy its localinfo before the operator tree\n" +
			"\tis deconstructed)\n" +
			"\tbit  4: debug progress (after sending a REQUESTPROGRESS\n" +
			"\tmessage to an operator, the ranges in the\n" +
			"\tProgressInfo are checked for whether tey are\n" +
			"\treasonable. If not so, the according operator and\n" +
			"\tProgressInfo are reported) \n" +
			"\tbit  5: trace progress (prints the result of\n" +
			"\teach REQUESTPROGRESS message)\n" +
			"\tbit  6: show type mappings\n", "DEBUG n"),
	REPEAT("REPEAT n <query> - execute <query> n times", "REPEAT"),
	QUIT("exit the program", "Q" , "QUIT");

	private String description;
	private String[] commandList;

	/**
	 *
	 * @param description
	 * @param commands
	 */
	private EditorCommand( final String description, final String... commands) {
		this.description = description;
		this.commandList = commands;
	}

	/**
	 *
	 * @return
	 */
	public String getDescription() {
		return description;
	}

	/**
	 *
	 * @return
	 */
	public String[] getCommandList() {
		return commandList;
	}
}