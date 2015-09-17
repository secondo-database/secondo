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

import javax.swing.JCheckBoxMenuItem;
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
		LOAD_QUERY(1, "Load Object from Query", false, false), LOAD_EXPLORER(2,
				"Load Object from Explorer", false, false), LOAD_DATABASE(3,
				"Load Objects from Database", true, false), CONVERT_ONE(4,
				"Convert selected Object to NL", false, false), CONVERT_ALL(5,
				"Convert all Objects to NL", true, false), RESULT_AUTOCONVERT(
				6, "Autoconvert query results to NL format", false, true), EXPORT(
				7, "Export MMObject", false, false), IMPORT(8,
				"Import MMObject", true, false), INDEX(9, "Generate Index",
				false, false), QUERY(10, "Execute Query", false, false), MEMORY(
				11, "Manage Memory", true, false), TYPES(120,
				"Supported Types", false, false), HELP(13, "Help", false, false);

		final int position;
		final boolean followedBySeparator;
		final boolean isCheckbox;
		final JMenuItem item;

		private MenuEntry(int position, String name,
				boolean followedBySeparator, boolean isCheckbox) {
			this.position = position;
			this.isCheckbox = isCheckbox;
			this.followedBySeparator = followedBySeparator;
			if (isCheckbox) {
				this.item = new JCheckBoxMenuItem(name, true);
			} else {
				this.item = new JMenuItem(name);
			}
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