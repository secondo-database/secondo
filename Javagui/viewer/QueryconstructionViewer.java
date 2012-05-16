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
import sj.lang.*;
import tools.Reporter;
import viewer.queryconstruction.*;

/**
 *
 * @author lrentergent
 */
public class QueryconstructionViewer extends SecondoViewer {
    
    private ObjectsPane ObjectsPane = new ObjectsPane(this);
    private OperationsPane OperationsPane = new OperationsPane(this);
    private MainPane MainPane;
    
    // define supported subtypes
    private static final String DATABASES = "databases";
    private static final String CONSTRUCTORS="constructors";
    private static final String OPERATORS = "operators";
    private static final String ALGEBRAS = "algebras";
    private static final String ALGEBRA = "algebra";
    private static final String TYPES = "types";
    private static final String OBJECTS ="objects";
    
    private MenuVector MV = new MenuVector();
    
    public QueryconstructionViewer(){
        this.setLayout(new BorderLayout());
        
        MainPane = new MainPane();
               
        ObjectsPane.setPreferredSize(new Dimension (500, 100));
        OperationsPane.setPreferredSize(new Dimension (110, 500));
        MainPane.setPreferredSize(new Dimension (600, 400));
        
        ObjectView query = new ObjectView("operation", "query");
        MainPane.addObject(query);
        
        ObjectView Trains = new ObjectView("rel", "Trains");
        ObjectsPane.addObject(Trains);
        ObjectView strassen = new ObjectView("rel", "strassen");
        //ObjectsPane.addObject(strassen);
        ObjectView Kinos = new ObjectView("rel", "Kinos");
        //ObjectsPane.addObject(Kinos);
        
        ObjectView train7 = new ObjectView("mpoint", "train7");
        //ObjectsPane.addObject(train7);
        ObjectView mehringdamm = new ObjectView("point", "mehringdamm");
        //ObjectsPane.addObject(mehringdamm);
        ObjectView tiergarten = new ObjectView("region", "tiergarten");
        //ObjectsPane.addObject(tiergarten);
        
        ObjectView feed = new ObjectView("operation", "feed");
        OperationsPane.addObject(feed);
        ObjectView head = new ObjectView("operation", "head");
        OperationsPane.addObject(head);
        ObjectView tail = new ObjectView("operation", "tail");
        OperationsPane.addObject(tail);
        ObjectView consume = new ObjectView("operation", "consume");
        OperationsPane.addObject(consume);
        
        OperationsPane.update();
        ObjectsPane.update();
        
        JScrollPane ScrollPane = new JScrollPane(MainPane);
        this.add(ObjectsPane, BorderLayout.NORTH);
        this.add(OperationsPane, BorderLayout.EAST);
        this.add(ScrollPane, BorderLayout.CENTER);
    }
    
    public void addObject(ObjectView object){
        MainPane.addObject(object);
        MainPane.repaint();
        OperationsPane.update();
        ObjectsPane.update();
        
        if (VC.execUserCommand(MainPane.getStrings())) {
            System.out.println(VC.getCommandResult(MainPane.getStrings()));
        }
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
    
    public boolean canDisplay(SecondoObject o){
        ListExpr LE = o.toListExpr(); // get the nested list of o
        if(LE.listLength()!=2) // the length must be two
            return false;
        // the first element must be an symbol atom with content "inquiry"
        if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM || !LE.first().symbolValue().equals("inquiry"))
            return false;
        ListExpr VL = LE.second();
        // the length of the second element must again be two
        if(VL.listLength()!=2)
            return false;
        ListExpr SubTypeList = VL.first();
        // the first element of this list must be a symbol atom
        if(SubTypeList.atomType()!=ListExpr.SYMBOL_ATOM)
            return false;
        String SubType = SubTypeList.symbolValue();
        // check for supported "sub types"
        // the used constants just contain the appropriate String
        if(SubType.equals(DATABASES) || SubType.equals(CONSTRUCTORS) || SubType.equals(OPERATORS) || SubType.equals(ALGEBRA) || SubType.equals(ALGEBRAS) || SubType.equals(OBJECTS) || SubType.equals(TYPES))
            return true;
        return false;
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
