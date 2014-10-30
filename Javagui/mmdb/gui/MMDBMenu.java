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

package mmdb.gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;

import mmdb.MMDBUserInterfaceController;

/**
 * This class represents the menu entry "MMDB" in the main menu bar.
 *
 * @author Alexander Castor
 */
public class MMDBMenu extends JMenu {

	private static final long serialVersionUID = 1973048546180993452L;

	/**
	 * Enum for collecting all menu entries.
	 */
	public enum MenuEntry {
		LOAD_QUERY(1, "Load Object from Query", false), LOAD_EXPLORER(2,
				"Load Object from Explorer", false), LOAD_DATABASE(3,
				"Load Objects from Database", true), CONVERT_ONE(4,
				"Convert selected Object to NL", false), CONVERT_ALL(5,
				"Convert all Objects to NL", true), INDEX(6, "Generate Index",
				false), QUERY(7, "Execute Query", false), MEMORY(8,
				"Manage Memory", true), TYPES(9, "Supported Types", false), HELP(
				10, "Help", false);

		final int position;
		final boolean followedBySeparator;
		final JMenuItem item;

		private MenuEntry(int position, String name, boolean followedBySeparator) {
			this.position = position;
			this.item = new JMenuItem(name);
			this.followedBySeparator = followedBySeparator;
		}
	}

	/**
	 * Generates the new JMenu.
	 */
	public MMDBMenu() {
		super("MMDB");
		addActionListeners();
		addItemsToMenu();
	}

	/**
	 * Adds action listeners to menu items.
	 */
	private void addActionListeners() {
		for (final MenuEntry entry : MenuEntry.values()) {
			entry.item.addActionListener(new ActionListener() {
				@Override
				public void actionPerformed(ActionEvent e) {
					MMDBUserInterfaceController.getInstance()
							.dispatchMenuEvent(entry);
				}
			});
		}
	}

	/**
	 * Adds items and separators to menu in sorted order.
	 */
	private void addItemsToMenu() {
		for (int i = 1; i <= MenuEntry.values().length; i++) {
			for (MenuEntry entry : MenuEntry.values()) {
				if (entry.position == i) {
					this.add(entry.item);
					if (entry.followedBySeparator) {
						this.addSeparator();
					}
					break;
				}
			}
		}
	}

}