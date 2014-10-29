//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
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

package unittests.mmdb.util;

import gui.CommandPanel;
import gui.ObjectList;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.List;

import sj.lang.ListExpr;
import viewer.QueryViewer;

/**
 * This class provides commonly used mock objects.
 *
 * @author Alexander Castor
 */
public class TestUtilMocks {

	@SuppressWarnings("serial")
	public static class ObjectListMock extends ObjectList {

		public List<SecondoObject> objects = new ArrayList<SecondoObject>();

		public ObjectListMock() {
			super(null, new QueryViewer());
		}

		@Override
		public void removeObject(SecondoObject SO) {
			objects.remove(SO);
		}

		@Override
		public void addEntry(SecondoObject SO) {
			objects.add(SO);
		}

	}

	@SuppressWarnings("serial")
	public static class CommandPanelMock extends CommandPanel {

		public List<String> history = new ArrayList<String>();

		public CommandPanelMock() {
			super(null, "test", "test");
		}

		@Override
		public ListExpr getCommandResult(String command) {
			if (command != null && !"query null".equals(command)) {
				return TestUtilRelation.getValidRelationList();
			}
			return null;
		}

		@Override
		public void addToHistory(String S) {
			history.add(S);
		}

	}

}
