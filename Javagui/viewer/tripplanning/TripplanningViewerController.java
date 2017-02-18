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

import gui.ViewerControl;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.List;

import tools.Reporter;
import viewer.TripplanningViewer;

/*
 This class controls the actionflow of the 'TripplanningViewer'.

 */
public class TripplanningViewerController implements ActionListener,
        MouseListener {
    private TripplanningViewer viewer;
    private ViewerControl vc;
    private long start=0;
    private long end=0;

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
        System.out.println("City:"
                + viewer.getQueryPanel().getTfCity().getText());
        String sourceStreet = viewer.getQueryPanel().getTfStreet().getText();
        String sourceNo = viewer.getQueryPanel().getTfNo().getText();
        String sourcePostcode = viewer.getQueryPanel().getTfPlz().getText();
        String sourceCity = viewer.getQueryPanel().getTfCity().getText();
        String targetStreet = viewer.getQueryPanel().getTfStreetDest()
                .getText();
        String targetNo = viewer.getQueryPanel().getTfNoDest().getText();
        String targetPostcode = viewer.getQueryPanel().getTfPlzDest().getText();
        String targetCity = viewer.getQueryPanel().getTfCityDest().getText();
        String gradientWeight = viewer.getQueryPanel().getTfGradient().getText();
        double gradientWeightDoub =  Double.parseDouble(gradientWeight);

        TripplanningSecondoCommand tsc = new TripplanningSecondoCommand(
                sourceStreet, sourceNo, sourcePostcode, sourceCity,
                targetStreet, targetNo, targetPostcode, targetCity, gradientWeightDoub);
        this.executeSecondoCommand(tsc.getCommands());
        String mainQuery="query EdgesHeight oshortestpatha[-1,0,0;distanceWithGradient(.SourcePos,.TargetPos,[const real value "+gradientWeight+" ],.Heightfunction), distance(.TargetPos, targetPos)] feed extend[Gradient: (lfResult(size(.Curve), .Heightfunction) -  lfResult(0.0, .Heightfunction)) * 100 / size(gk(.Curve))] feed extend[GradientCategory:(ifthenelse(.Gradient<-20,\"lower-20\",ifthenelse(.Gradient<-10,\"lower-10\",ifthenelse(.Gradient<-6,\"lower-6\",ifthenelse(.Gradient<-2,\"lower-2\",ifthenelse(.Gradient<2,\"lower2\",ifthenelse(.Gradient<6,\"lower6\",ifthenelse(.Gradient<10,\"lower10\",ifthenelse(.Gradient<20,\"lower20\",\"bigger20\")))))))))]  consume;";
        logStart(mainQuery);
        vc.execUserCommand(mainQuery);
        logEnd();
    }

    /**
     * Executes given commands.
     */
    private void executeSecondoCommand(List<String> pCommands) {
        for (String command : pCommands) {
            // Executes the remote command.
            logStart(command);
            int errorCode=vc.execCommand(command);
            logEnd();

                if (errorCode != 0) {
                    //errorCode=12 on a delete statement means that the object didn't exist
                    if (!(errorCode == 12 && command.contains("delete"))) {
                        System.out.print("\tErrorCode:" + errorCode);
                        System.out
                                .println("### Command execution is aborted. Please see error code above. ###");
                        break;
                    }
                }
        }
    }

    private void logStart(String command) {
        System.out.print("Command: " + command);
        start=System.currentTimeMillis();
    }

    private void logEnd() {
        end=System.currentTimeMillis();
        System.out.println("\tduration: " + (end - start)+" ms");
    }

    public void mouseClicked(MouseEvent arg0) {

    }

    public void mouseEntered(MouseEvent arg0) {
    }

    public void mouseExited(MouseEvent arg0) {
    }

    public void mousePressed(MouseEvent arg0) {
    }

    public void mouseReleased(MouseEvent arg0) {
    }

    public void setViewerControl(ViewerControl vc) {
        this.vc=vc;
        
    }

}

