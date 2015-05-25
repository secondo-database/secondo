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

package viewer.hoese2.hoese;

import gui.SecondoObject;
import gui.idmanager.ID;

import java.awt.Color;
import java.awt.Component;
import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Vector;

import javax.swing.DefaultListCellRenderer;
import javax.swing.DefaultListModel;
import javax.swing.JFileChooser;
import javax.swing.JList;
import javax.swing.ListModel;
import javax.swing.ListSelectionModel;
import javax.swing.SwingUtilities;

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.HoeseViewer;
import viewer.hoese.algebras.continuousupdate.RunnableUpdate;

/**
 * This class enhances JList. A list is generated dependant to the types given
 * back and formatted by the query.
 */
public class QueryResult extends JList {
	/** The query command */
	public String command;
	/** The result given back by the query command as a Listexpr */
	public ListExpr LEResult;
	/** A list of the ggraphic objects of this query */
	private Vector GraphObjects;
	/** No. of tuples, no. of attributes of a tuple */
	private int TupelCount, AttrCount;
	/** FileChosser for storing a file **/
	private JFileChooser filechooser = new JFileChooser();
	/** The id of the stored Secondo-Object */
	private ID id;
	/** Last element of LEResult; */
	private ListExpr lastElement;

	/** return the Id for this QueryResult **/
	public ID getId() {
		return id;
	}

	/** the QueryRepresentations for this QueryResult */
	// private ViewConfig myViewConfig = null;
	private Vector ViewConfigs = new Vector(2);

	/**
	 * stores the interval where Time dependent objects are defined
	 **/
	private Interval interval;
	private Layer resultLayer;

	private void setLastElement() {
		ListExpr last = LEResult.second();
		while (!last.endOfList()) {
			last = last.rest();
		}
		lastElement = last;
	}

	/**
	 * Add tuples which are received by a continuous update
	 * 
	 * @param hoese
	 *            The Viewer this QR belongs to
	 * @param tuple
	 *            The Tuple which was received
	 * @param modelLimit
	 *            The maximum Size of the used ListModel
	 */
	public void addTuples(HoeseViewer hoese, ListExpr tuple, Integer tupleLimit) {

		// For performance reasons, save the last element of the nested-list
		if (lastElement == null) {
			setLastElement();
		}

		// Create a temporary QueryResult
		QueryResult tmp_qr = new QueryResult(new SecondoObject("dummy",
				LEResult));

		// Loop through the provided nested List, and add all tuples
		while (tuple != null & !tuple.isEmpty()) {
			ListExpr current = tuple.first();
			tuple = tuple.rest();

			lastElement = ListExpr.append(lastElement, current);
			TupelCount++;
			tmp_qr.addEntry("---------");
			LEUtils.analyse(LEResult.first().toString(), 0, 0,
					LEResult.first(), ListExpr.oneElemList(current), tmp_qr);
		}

		// Reduce the NestedList
		if (tupleLimit > 0) {
			ListExpr tuples = LEResult.second();
			for (int i = tuples.listLength(); i > tupleLimit; i--) {
				tuples = tuples.rest();
				TupelCount--;
			}
			LEResult.second().setValueTo(tuples);
		}

		// Update the ListModel on the GUI thread
		SwingUtilities.invokeLater(new RunnableUpdate(this, tmp_qr, hoese,
				tupleLimit));
	}

	/**
	 * Creates a QueryResult with a command and a result of a query
	 * 
	 * @param String
	 *            acommand
	 * @param ListExpr
	 *            aLEResult
	 */
	public QueryResult(SecondoObject so) {
		super();
		String acommand = so.getName();
		ListExpr aLEResult = so.toListExpr();
		this.id = so.getID();
		interval = null;
		setFont(new Font("Monospaced", Font.PLAIN, 12));

		// processing double clicks
		addMouseListener(new MouseAdapter() {
			public void mouseClicked(MouseEvent e) {
				if (e.getButton() != MouseEvent.BUTTON1
						&& e.getClickCount() == 1) {
					int index = QueryResult.this.locationToIndex(e.getPoint());
					if (index < 0) {
						return;
					}
					Object o = QueryResult.this.getModel().getElementAt(index);
					if (o != null && (o instanceof Writable)) {
						if (filechooser.showSaveDialog(null) == JFileChooser.APPROVE_OPTION) {
							File F = filechooser.getSelectedFile();
							if (F.exists()) {
								if (Reporter.showQuestion("File " + F
										+ " already exists,\n Overwrite It?") != Reporter.YES) {
									return;
								}
							}
							boolean ok = false;
							try {
								ok = ((Writable) o).writeTo(F);
							} catch (Exception e4) {
								Reporter.debug(e4);
							}
							if (ok) {
								Reporter.showInfo("File " + F
										+ " has been written");
							} else {
								Reporter.showError("error in writing file " + F);
							}
						}
					}
				}
				if (e.getClickCount() != 2) {
					return;
				}
				Object o = QueryResult.this.getSelectedValue();
				if ((o instanceof DsplBase)
						&& (((DsplBase) o).getFrame() != null)) {
					((DsplBase) o).getFrame().select(o);
					((DsplBase) o).getFrame().show(true);
				}
				if ((o instanceof ExternDisplay)) {
					ExternDisplay BG = (ExternDisplay) o;
					if (!BG.isExternDisplayed()) {
						BG.displayExtern();
					}
				}
			}
		});

		setModel(new DefaultListModel());
		setCellRenderer(new QueryRenderer());
		setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		setBackground(Color.lightGray);
		command = acommand;
		LEResult = aLEResult;
		TupelCount = LEResult.second().listLength();
		if (LEResult.first().isAtom()) {
			AttrCount = 0;
		} else {
			try {
				AttrCount = LEResult.first().second().second().listLength();
			} catch (Exception e) {
				AttrCount = 0;
			}
		}
		GraphObjects = new Vector(50);
	}

	/**
	 * Creates a QueryResult for Collections with a command and a result of a
	 * query
	 * 
	 * @param String
	 *            acommand
	 * @param ListExpr
	 *            aLEResult
	 * @param boolean isColl
	 */
	public QueryResult(SecondoObject so, boolean isColl) {
		super();
		interval = null;
		setFont(new Font("Monospaced", Font.PLAIN, 12));
		String acommand = so.getName();
		ListExpr aLEResult = so.toListExpr();
		this.id = so.getID();
		// processing double clicks
		addMouseListener(new MouseAdapter() {
			public void mouseClicked(MouseEvent e) {
				if (e.getButton() != MouseEvent.BUTTON1
						&& e.getClickCount() == 1) {
					int index = QueryResult.this.locationToIndex(e.getPoint());
					if (index < 0) {
						return;
					}
					Object o = QueryResult.this.getModel().getElementAt(index);
					if (o != null && (o instanceof Writable)) {
						if (filechooser.showSaveDialog(null) == JFileChooser.APPROVE_OPTION) {
							File F = filechooser.getSelectedFile();
							if (F.exists()) {
								if (Reporter.showQuestion("File " + F
										+ " already exists,\n Overwrite It?") != Reporter.YES) {
									return;
								}
							}
							boolean ok = false;
							try {
								ok = ((Writable) o).writeTo(F);
							} catch (Exception e4) {
								Reporter.debug(e4);
							}
							if (ok) {
								Reporter.showInfo("File " + F
										+ " has been written");
							} else {
								Reporter.showError("error in writing file " + F);
							}
						}
					}
				}
				if (e.getClickCount() != 2) {
					return;
				}
				Object o = QueryResult.this.getSelectedValue();
				if ((o instanceof DsplBase)
						&& (((DsplBase) o).getFrame() != null)) {
					((DsplBase) o).getFrame().select(o);
					((DsplBase) o).getFrame().show(true);
				}
				if ((o instanceof ExternDisplay)) {
					ExternDisplay BG = (ExternDisplay) o;
					if (!BG.isExternDisplayed()) {
						BG.displayExtern();
					}
				}
			}
		});

		setModel(new DefaultListModel());
		setCellRenderer(new QueryRenderer());
		setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		setBackground(Color.lightGray);
		command = acommand;
		LEResult = aLEResult;
		GraphObjects = new Vector(50);
	}

	public boolean hasId(ID id) {
		return this.id.equals(id);
	}

	/** get the ViewConfigs for this query */
	public ViewConfig[] getViewConfigs() {
		ViewConfig[] Cfgs = new ViewConfig[ViewConfigs.size()];
		for (int i = 0; i < Cfgs.length; i++)
			Cfgs[i] = (ViewConfig) ViewConfigs.get(i);
		return Cfgs;
	}

	/** get the ViewConfig with specific index */
	public ViewConfig getViewConfigAt(int index) {
		if (index < 0 || index >= ViewConfigs.size())
			return null;
		else
			return (ViewConfig) ViewConfigs.get(index);
	}

	/** set the ViewConfig for this query */
	public void addViewConfig(ViewConfig VCfg) {
		ViewConfigs.add(VCfg);
	}

	/**
	 * get the Pos of the ViewConfig with specific AttrName if not exists a
	 * ViewConfig with AttrName -1 is returned
	 */
	public int getViewConfigIndex(String AttrName) {
		int pos = -1;
		boolean found = false;
		for (int i = 0; i < ViewConfigs.size() && !found; i++)
			if (((ViewConfig) ViewConfigs.get(i)).AttrName.equals(AttrName)) {
				pos = i;
				found = true;
			}
		return pos;
	}

	/**
	 * 
	 * @return The ListExpr of the picked (selected) list-entry
	 */
	public ListExpr getPick() {
		if (AttrCount == 0) {
			return LEResult;
		}
		int selind = getSelectedIndex();
		int TupelNr = (selind / (AttrCount + 1));
		int AttrNr = (selind % (AttrCount + 1));
		if (AttrNr == AttrCount)
			return null; // Separator
		ListExpr TupelList = LEResult.second();
		for (int i = 0; i < TupelNr; i++)
			TupelList = TupelList.rest();
		ListExpr AttrList = TupelList.first();
		for (int i = 0; i < AttrNr; i++)
			AttrList = AttrList.rest();
		ListExpr TypeList = LEResult.first().second().second();
		for (int i = 0; i < AttrNr; i++)
			TypeList = TypeList.rest();
		return ListExpr
				.twoElemList(TypeList.first().second(), AttrList.first());
	}

	/**
	 * 
	 * @return graphical objects of this query-result
	 */
	public Vector getGraphObjects() {
		return GraphObjects;
	}

	/**
	 * Adds an object to the end of the result list
	 * 
	 * @param entry
	 *            The entry object
	 */
	public void addEntry(Object entry) {
		if (entry != null) {
			if (entry instanceof DsplBase) {
				if (entry instanceof DsplGraph) {
					GraphObjects.add(entry);
				}
				if (((DsplBase) entry).getFrame() != null) {
					((DsplBase) entry).getFrame().addObject(entry);
				}
			}
		}

		((DefaultListModel) getModel()).addElement(entry);
	}

	/**
	 * Adds a ListModel to the end of the result list
	 * 
	 * @param entries
	 *            Collection
	 */
	public void addEntries(ListModel entries) {
		if (entries.getSize() == 0) {
			return;
		}

		for (int i = 0; i < entries.getSize(); i++) {
			Object entry = entries.getElementAt(i);
			if (entry != null) {
				if (entry instanceof DsplBase) {
					if (entry instanceof DsplGraph) {
						GraphObjects.add(entry);
					}
					if (((DsplBase) entry).getFrame() != null) {
						((DsplBase) entry).getFrame().addObject(entry);
					}
				}
			}
			((DefaultListModel) getModel()).addElement(entry);
		}

	}

	/**
	 * Removes entries from the used models to retain the maximum size Entries
	 * will be removed from the front, it maxSize is greater than zero
	 * 
	 * @param maxSize
	 *            Maximum Size of the Model
	 * @param hoese
	 *            Hoese Viewer
	 */
	public void reduceModels(Integer tupleLimit, HoeseViewer hoese) {
		
		DefaultListModel model = ((DefaultListModel) getModel());

		while (tupleLimit > 0 && model.size() > (tupleLimit * (AttrCount + 1))) {
			for (int i = 0; i < AttrCount + 1; i++) {
				Object o = model.remove(0);
				if (o instanceof DsplGraph) {
					GraphObjects.remove(o);
					resultLayer.removeGO((DsplGraph)o);
				}
			}
		}
	}

	/**
	 * search the given String in this list and returns the index, the search is
	 * started with offset and go to the end of the list. if the given string is
	 * not containing between offset and end -1 is returned
	 */

	public int find(String S, boolean CaseSensitiv, int Offset) {
		ListModel LM = getModel();
		if (LM == null)
			return -1;
		String UCS = S.toUpperCase();
		boolean found = false;
		int pos = -1;
		for (int i = Offset; i < LM.getSize() && !found; i++) {
			if (CaseSensitiv && LM.getElementAt(i).toString().indexOf(S) >= 0) {
				pos = i;
				found = true;
			}
			if (!CaseSensitiv
					&& LM.getElementAt(i).toString().toUpperCase().indexOf(UCS) >= 0) {
				pos = i;
				found = true;
			}
		}
		return pos;
	}

	public boolean equals(Object o) {
		if (!(o instanceof QueryResult))
			return false;
		else {
			QueryResult qr = (QueryResult) o;
			return this.id == qr.id;
		}
	}

	/**
	 * return the command
	 */
	public String getCommand() {
		return command;
	}

	/**
	 * Sets the command
	 **/
	public void setCommand(String command) {
		this.command = command;
	}

	/** return the ListExpr */
	public ListExpr getListExpr() {
		return LEResult;
	}

	/**
	 * computes the TimeBounds from the contained objects.
	 **/
	public void computeTimeBounds() {
		ListModel listModel = getModel();
		int size = listModel.getSize();
		this.interval = null;
		for (int i = 0; i < size; i++) {
			Object o = listModel.getElementAt(i);
			if (o instanceof Timed) {
				Interval oInterval = ((Timed) o).getBoundingInterval();
				if (oInterval != null) {
					if (this.interval == null) {
						this.interval = oInterval.copy();
					} else {
						this.interval.unionInternal(oInterval);
					}
				}
			}
		}
	}

	/**
	 * Returns the interval containing all definition times of object instances
	 * of Timed. If no such time exist, the result is null.
	 **/
	public Interval getBoundingInterval() {
		return interval;
	}

	/**
	 * 
	 * @return The command of the query as string representation for the query
	 *         combobox
	 */
	public String toString() {
		return command;
	}

	/**
	 * A class for special rendering of datatypes in the list
	 */

	private class QueryRenderer extends DefaultListCellRenderer {
		public Component getListCellRendererComponent(JList list, Object value,
				int index, boolean isSelected, boolean cellHasFocus) {
			super.getListCellRendererComponent(list, value, index, isSelected,
					cellHasFocus);
			// if (value instanceof DsplGraph)
			// setIcon(new ImageIcon("images/labdlg.gif"));
			setForeground(Color.BLACK);
			if ((value instanceof DsplGraph) && (value instanceof Timed))
				setForeground(Color.magenta);
			else if (value instanceof DsplGraph)
				setForeground(Color.red);
			else if (value instanceof Timed)
				setForeground(Color.blue);
			else if (value instanceof DsplBase)
				setForeground(new Color(0, 100, 0));
			if (value instanceof DsplBase)
				if (!((DsplBase) value).getVisible())
					setForeground(Color.gray);
			if (value instanceof ExternDisplay) {
				setForeground(new Color(255, 0, 0));
			}
			return this;
		}
	}

	public void setResultLayer(Layer lay) {
		resultLayer = lay;
	}

	public Layer getResultLayer() {
		return resultLayer;
	}
}
