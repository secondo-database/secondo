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

package viewer;

import gui.SecondoObject;
import gui.idmanager.ID;
import gui.idmanager.IDManager;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Image;
import java.io.BufferedReader;
import java.io.FileReader;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTabbedPane;
import javax.swing.JEditorPane;
import javax.swing.JTextField;
import javax.swing.ScrollPaneConstants;
import javax.swing.text.html.HTMLDocument;
import javax.swing.text.html.HTMLEditorKit;

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.tripplanning.TripplanningViewerController;
import viewer.update2.*;
import viewer.update2.gui.*;

/**
 * This viewer allows editing of multiple relations. It can be used to update,
 * insert and delete tuples. Tuple values are displayed sequentially in order to
 * faciliate viewing and editing of long text values.
 */
public class TripplanningViewer extends SecondoViewer {

    private static final long serialVersionUID = -1944940928017006815L;

    private String viewerName = "TripplanningViewer";

    private JPanel actionPanel;

    // Components to display the loaded relation set.
    private JTabbedPane tabbedPane;

    private List<RelationPanel> relationPanels;

    // the controller decides which action shall be taken next and listens to
    // all buttons
    // for user-input
    private TripplanningViewerController controller;
    
    private JTextField tfStreet;
    private JTextField tfPlz;
    private JTextField tfCity; // = new JTextField(50);
    private JTextField tfStreetDest;
    private JTextField tfPlzDest;
    private JTextField tfCityDest;
    private JTextField tfGradient;

    public String getViewerName() {
        return viewerName;
    }

    public void setViewerName(String viewerName) {
        this.viewerName = viewerName;
    }

    public JTextField getTfStreet() {
        return tfStreet;
    }

    public void setTfStreet(JTextField tfStreet) {
        this.tfStreet = tfStreet;
    }

    public JTextField getTfPlz() {
        return tfPlz;
    }

    public void setTfPlz(JTextField tfPlz) {
        this.tfPlz = tfPlz;
    }

    public JTextField getTfCity() {
        return tfCity;
    }

    public void setTfCity(JTextField tfCity) {
        this.tfCity = tfCity;
    }

    public JTextField getTfStreetDest() {
        return tfStreetDest;
    }

    public void setTfStreetDest(JTextField tfStreetDest) {
        this.tfStreetDest = tfStreetDest;
    }

    public JTextField getTfPlzDest() {
        return tfPlzDest;
    }

    public void setTfPlzDest(JTextField tfPlzDest) {
        this.tfPlzDest = tfPlzDest;
    }

    public JTextField getTfCityDest() {
        return tfCityDest;
    }

    public void setTfCityDest(JTextField tfCityDest) {
        this.tfCityDest = tfCityDest;
    }

    public JTextField getTfGradient() {
        return tfGradient;
    }

    public void setTfGradient(JTextField tfGradient) {
        this.tfGradient = tfGradient;
    }

    /*
     * Constructor.
     */
    public TripplanningViewer() {
        this.controller = new TripplanningViewerController(this);

        this.setLayout(new BorderLayout());

        // actionpanel
        this.actionPanel = new JPanel();
        this.actionPanel.setLayout(new GridLayout(1, 9));
        JPanel queryPanel = new JPanel();
        queryPanel.setLayout(new BoxLayout(queryPanel, BoxLayout.Y_AXIS));
        JPanel resultPanel = new JPanel();

        JLabel label = new JLabel("START");
        queryPanel.add(label);

        JLabel streetLabel = new JLabel("Street + no. ");
        queryPanel.add(streetLabel);
        tfStreet = new JTextField(50);
        queryPanel.add(tfStreet);
        JLabel plzLabel = new JLabel("postcode");
        queryPanel.add(plzLabel);
        tfPlz = new JTextField(10);
        queryPanel.add(tfPlz);
        JLabel cityLabel = new JLabel("town/city");
        queryPanel.add(cityLabel);
        tfCity = new JTextField(50);
        queryPanel.add(tfCity);

        JLabel labelDest = new JLabel("DESTINATION");
        queryPanel.add(labelDest);

        JLabel streetLabelDest = new JLabel("Street + no. ");
        queryPanel.add(streetLabelDest);
        tfStreetDest = new JTextField(50);
        queryPanel.add(tfStreetDest);
        JLabel plzLabelDest = new JLabel("postcode");
        queryPanel.add(plzLabelDest);
        tfPlzDest = new JTextField(10);
        queryPanel.add(tfPlzDest);
        JLabel cityLabelDest = new JLabel("town/city");
        queryPanel.add(cityLabelDest);
        tfCityDest = new JTextField(50);
        queryPanel.add(tfCityDest);
        
        JLabel gradientLabel = new JLabel("gradient weight (>=0)");
        queryPanel.add(gradientLabel);
        tfGradient = new JTextField("0",50);
        queryPanel.add(tfGradient);


        JButton searchButton = new JButton(TripplanningViewerController.SEARCH);
        searchButton.addActionListener(controller);
        queryPanel.add(searchButton);

        JScrollPane queryScroll = new JScrollPane(queryPanel,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        JScrollPane resultScroll = new JScrollPane(resultPanel,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        JSplitPane splitpane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
                queryScroll, resultScroll);
        splitpane.setOneTouchExpandable(true);
        splitpane.setResizeWeight(0.6);

        actionPanel.add(splitpane);

        this.add(actionPanel, BorderLayout.NORTH);

    }

    /**
     * Returns the currently active RelationPanel.
     */
    public RelationPanel getCurrentRelationPanel() {
        RelationPanel result = null;
        int index = this.tabbedPane.getSelectedIndex();
        if (index >= 0 && index < this.relationPanels.size()) {
            result = this.relationPanels.get(index);
        }
        return result;
    }

    /**
     * Returns the RelationPanel with the Relation of the specified name.
     */
    public RelationPanel getRelationPanel(String pRelName) {
        for (RelationPanel relpanel : this.relationPanels) {
            if (relpanel.getName().equals(pRelName)) {
                return relpanel;
            }
        }
        return null;
    }

    public boolean removeRelationPanel(int pIndex) {
        if (pIndex >= 0 && pIndex < this.relationPanels.size()) {
            this.relationPanels.remove(pIndex);
            this.tabbedPane.remove(pIndex);
            return true;
        }
        return false;
    }

    /**
     * Removes currently shown relation set from this viewer. All information
     * about these relations will be lost.
     */
    public void clear() {

        this.validate();
        this.repaint();
    }

    /**
     * Creates or overwrites RelationPanel data with specified data.
     * 
     * @param pRelName
     *            original name of SecondoObject
     * @param pRelationLE
     *            complete relation ListExpression
     * @param isDirectQuery
     *            TRUE if relation is opened read-only, especially if loaded by
     *            direct query in command window.
     */
    public boolean setRelationPanel(String pRelName, ListExpr pRelationLE,
            boolean pEditable) {
        RelationPanel rp = this.getRelationPanel(pRelName);

        return rp.createFromLE(pRelationLE, pEditable);
    }

    /*
     * For each mode and state the viewer is in only certain operations and
     * choices are possible. This method assures only the actually allowed
     * actions can be executed or chosen.
     */

    /*********************************************************
     * Methods of SecondoViewer
     *********************************************************/

    /**
     * Method of SecondoViewer: Get the name of this viewer. The name is used in
     * the menu of the MainWindow.
     */
    @Override
    public String getName() {
        return viewerName;
    }

    /**
     * Method of SecondoViewer: Add new relation panel to display specified
     * relation SecondoObject is SecondoObject is relation and not yet
     * displayed.
     */
    @Override
    public boolean addObject(SecondoObject so) {
        if (!this.canDisplay(so) || this.isDisplayed(so)) {
            return false;
        }

        if (!this.setRelationPanel(so.getName(), so.toListExpr(), false)) {
            return false;
        }

        boolean allReadOnly = true;
        for (RelationPanel rp : this.relationPanels) {
            if (rp.getState() == States.LOADED) {
                allReadOnly = false;
            }
        }

        RelationPanel rp = getRelationPanel(so.getName());
        if (rp != null) {
            int index = this.relationPanels.indexOf(rp);
            this.tabbedPane.setSelectedIndex(index);
        }
        return true;
    }

    /**
     * Method of SecondoViewer
     */
    @Override
    public void removeObject(SecondoObject so) {
        RelationPanel rp = getRelationPanel(so.getName());
        if (rp != null) {
            int index = this.relationPanels.indexOf(rp);
            this.removeRelationPanel(index);
        }

    }

    /*
     * Method of SecondoViewer. Remove all displayed relations from viewer.
     */
    @Override
    public void removeAll() {
        this.clear();
    }

    /*
     * Method of SecondoViewer: Returns true if specified SecondoObject is a
     * relation.
     */
    public boolean canDisplay(SecondoObject so) {
        ListExpr le = so.toListExpr();
        Reporter.debug("UpdateViewer2.canDisplay: full ListExpr is "
                + le.toString());

        if (le.listLength() >= 2 && !le.first().isAtom()) {
            le = le.first();
            // Reporter.debug("UpdateViewer2.canDisplay: type ListExpr is " +
            // le.toString());

            if (!le.isAtom() && !le.isEmpty() && le.first().isAtom()) {
                String objectType = le.first().symbolValue();
                return Head.isRelationType(objectType);
            }
        }
        return false;
    }

    /*
     * Method of SecondoViewer: Because this viewer shall not display objects
     * others than relations loaded by the viewer itself false is returned.
     */
    @Override
    public boolean isDisplayed(SecondoObject so) {
        RelationPanel rp = getRelationPanel(so.getName());
        if (rp != null) {
            return true;
        }
        return false;
    }

    /*
     * Method of SecondoViewer: Because this viewer shall not display objects
     * others than relations loaded by the viewer itself false is returned.
     */
    @Override
    public boolean selectObject(SecondoObject so) {
        return false;
    }

    /*
     * Method of SecondoViewer: Because all commands for this viewer can be
     * accessed from the actionPanel no MenuVector is built.
     */
    @Override
    public MenuVector getMenuVector() {
        return null;
    }

    /*
     * Method of SecondoViewer
     */
    @Override
    public double getDisplayQuality(SecondoObject so) {
        if (!this.canDisplay(so)) {
            return 0;
        }

        if (!so.toListExpr().toString().contains("text")) {
            // optimized for relation with (long) text attributes
            return 0.8;
        }

        // other relations are displayed as well
        // but may use too much space
        // as attributes are displayed sequentially
        return 0.4;
    }

}
