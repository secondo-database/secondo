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

import javax.swing.*;
import java.awt.*;
import gui.SecondoObject;
import viewer.queryconstruction.*;

/**
 *
 * @author lrentergent
 */
public class QueryconstructionViewer extends SecondoViewer {
    
    private ObjectsPane ObjectsPane = new ObjectsPane(this);
    private OperationsPane OperationsPane = new OperationsPane(this);
    private MainPane MainPane;
    
    private MenuVector MV = new MenuVector();
    
    public QueryconstructionViewer(){
        this.setLayout(new BorderLayout());
        
        MainPane = new MainPane();
               
        ObjectsPane.setPreferredSize(new Dimension (500, 100));
        OperationsPane.setPreferredSize(new Dimension (110, 500));
        MainPane.setPreferredSize(new Dimension (600, 400));
        
        ObjectComponent query = new ObjectComponent("operation", "query");
        MainPane.addObject(query);
        
        ObjectComponent Trains = new ObjectComponent("rel", "Trains");
        ObjectsPane.addObject(Trains);
        ObjectComponent strassen = new ObjectComponent("rel", "strassen");
        ObjectsPane.addObject(strassen);
        ObjectComponent Kinos = new ObjectComponent("rel", "Kinos");
        ObjectsPane.addObject(Kinos);
        
        ObjectComponent train7 = new ObjectComponent("mpoint", "train7");
        ObjectsPane.addObject(train7);
        ObjectComponent mehringdamm = new ObjectComponent("point", "mehringdamm");
        ObjectsPane.addObject(mehringdamm);
        ObjectComponent tiergarten = new ObjectComponent("region", "tiergarten");
        ObjectsPane.addObject(tiergarten);
        
        ObjectComponent neu3 = new ObjectComponent("operation", "feed");
        OperationsPane.addObject(neu3);
        
        JScrollPane ScrollPane = new JScrollPane(MainPane);
       
        this.add(ObjectsPane, BorderLayout.NORTH);
        this.add(OperationsPane, BorderLayout.EAST);
        this.add(ScrollPane, BorderLayout.CENTER);
    }
    
    public void addObject(ObjectComponent object){
        MainPane.addObject(object);
        MainPane.repaint();
        VC.execCommand("query Trains feed consume");
        VC.getCommandResult("query Trains feed consume");
    }
    
    public boolean addObject(SecondoObject o){
        return true;
    }
    
    public void removeObject(SecondoObject o){
        
    }
    
    public boolean selectObject (SecondoObject o) {
        return true;
    }
    
    /** remove all containing objects */
    public void removeAll(){
    
    }
    
    /** returns InquiryViewer */
    public String getName(){
        return "QueryconstructionViewer";
    }
    
    public boolean canDisplay(SecondoObject o) {
        return true;
    }

    /** check if o displayed in the moment **/
    public boolean isDisplayed(SecondoObject o) {
        return true;
    }
    
    public MenuVector getMenuVector() {
        return MV;
    }
    
    public double getDisplayQuality(SecondoObject SO) {
        return 1.0;
    }

    public void enableTestmode (boolean on) {
        on = true;
    }
}
