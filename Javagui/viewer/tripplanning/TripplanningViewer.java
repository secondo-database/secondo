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
import gui.ViewerControl;

import java.awt.BorderLayout;
import java.awt.GridLayout;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ScrollPaneConstants;

import viewer.tripplanning.TripQueryPanel;
import viewer.tripplanning.TripplanningViewerController;

/**
 * This viewer allows editing of multiple relations. It can be used to update,
 * insert and delete tuples. Tuple values are displayed sequentially in order to
 * faciliate viewing and editing of long text values.
 */
public class TripplanningViewer extends SecondoViewer {

    private static final long serialVersionUID = -1944940928017006815L;

    private String viewerName = "TripplanningViewer";

    private JPanel actionPanel;
    private TripQueryPanel queryPanel;

    // the controller decides which action shall be taken next and listens to
    // all buttons for user-input
    private TripplanningViewerController controller;

    /*
     * Constructor.
     */
    public TripplanningViewer() {
        this.controller = new TripplanningViewerController(this);

        this.setLayout(new BorderLayout());

        queryPanel = new TripQueryPanel("query");
        queryPanel.getSearchButton().addActionListener(controller);

        this.add(queryPanel);

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
        return false;

    }

    /**
     * Method of SecondoViewer
     */
    @Override
    public void removeObject(SecondoObject so) {

    }

    /*
     * Method of SecondoViewer. Remove all displayed relations from viewer.
     */
    @Override
    public void removeAll() {
        this.validate();
        this.repaint();
    }

    /*
     * Method of SecondoViewer: Returns true if specified SecondoObject is a
     * relation.
     */
    public boolean canDisplay(SecondoObject so) {

        return false;
    }

    /*
     * Method of SecondoViewer: Because this viewer shall not display objects
     * others than relations loaded by the viewer itself false is returned.
     */
    @Override
    public boolean isDisplayed(SecondoObject so) {

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

    public JPanel getActionPanel() {
        return actionPanel;
    }

    public TripQueryPanel getQueryPanel() {
        return queryPanel;
    }

    @Override
    /** set the Control for this viewer **/
    public void setViewerControl(ViewerControl VC) {
        this.VC = VC;
        this.controller.setViewerControl(VC);
    }

}
