/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
---
 */

package viewer.tripplanning;

import gui.MainWindow;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.List;

import sj.lang.IntByReference;
import sj.lang.ListExpr;
import sj.lang.UpdateInterface;
import tools.Reporter;
import viewer.TripplanningViewer;

/*
 This class controls the actionflow of the 'TripplanningViewer'.

 */
public class TripplanningViewerController implements ActionListener,
        MouseListener {
    private UpdateInterface updateInterface = null;
    private TripplanningViewer viewer;
    private ListExpr resultList = new ListExpr();
    private IntByReference errorCode = new IntByReference(0);
    private IntByReference errorPos = new IntByReference(0);
    private StringBuffer errorMessage = new StringBuffer();

    public final static String SEARCH = "Search";

    /**
     * Constructor
     */
    public TripplanningViewerController(TripplanningViewer viewer) {
        this.viewer = viewer;
    }

    /**
     * Reacts on user actions in TripplanningViewer
     * 
     */
    public void actionPerformed(ActionEvent e) {
        if (e.getActionCommand() == SEARCH) {
            this.processSearchAction();
            return;
        }

        // This point should never be reached
        Reporter.showError("Command not known");
    }

    private void processSearchAction() {
        System.out.println("Da sind wa");
        System.out.println("City:" + viewer.getTfCity().getText());

        // hier geocode in der TrajectoryAnnotationAlgebra aufrufen. das ist die
        // google funktion
        // oder gleich ein Skript was alles macht
        searchAddressInGoogle();
    }

    private boolean searchAddressInGoogle() {
        List<String> commands = new ArrayList<String>();

        // TODO: Hier das Statement eingeben, mit dem man google abfragen kann
        StringBuffer sb = new StringBuffer("let ");
        sb.append("keks");
        sb.append(" = [const rel (tuple (");
        sb.append(" [ProfileName: string, FormatType: string, FormatAliases: text, ");
        sb.append(" FormatQuery: text, FormatScript: text, OutputDir: text, ");
        sb.append(" FormatTemplateHead: text, FormatTemplateBody: text, FormatTemplateTail: text]");
        sb.append(" )) value ()]");
        commands.add(sb.toString());

        ListExpr result = this.executeSecondoCommand(commands);
        if (!result.isEmpty()) {
            parseSecondoResponse(result);
        } else {
            return false;
        }
        return true;
    }

    private void parseSecondoResponse(ListExpr result) {
        // TODO Auto-generated method stub
        //ListExpr auseinander nehmen
    }

    /**
     * Executes given commands.
     */
    private ListExpr executeSecondoCommand(List<String> pCommands) {
        String errorMsg;

        updateInterface = MainWindow.getUpdateInterface();

        for (String command : pCommands) {
            // Executes the remote command.
            if (updateInterface.isInitialized()) {
                updateInterface.secondo(command, // Command to execute.
                        resultList, errorCode, errorPos, errorMessage);

            } else {
                errorMsg = "Connection to SECONDO lost!";
                Reporter.showError("TripplanningViewerController.executeSecondoCommand: Error on executing command "
                        + command + ": " + errorMsg);
            }
        }

        return resultList;
    }

    public void mouseClicked(MouseEvent arg0) {
        // TODO Auto-generated method stub

    }

    public void mouseEntered(MouseEvent arg0) {
        // TODO Auto-generated method stub

    }

    public void mouseExited(MouseEvent arg0) {
        // TODO Auto-generated method stub

    }

    public void mousePressed(MouseEvent arg0) {
        // TODO Auto-generated method stub

    }

    public void mouseReleased(MouseEvent arg0) {
        // TODO Auto-generated method stub

    }

}
