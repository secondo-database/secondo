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
import java.awt.event.*;
import gui.SecondoObject;
import sj.lang.*;
import viewer.queryconstruction.*;

/**
 *
 * @author lrentergent
 */
public class QueryconstructionViewer extends SecondoViewer {
    
    private ObjectsPane ObjectsPane;
    private OperationsPane OperationsPane = new OperationsPane(this);
    private MainPane MainPane;
    
    // define supported types
    protected static final String RELATION = "rel";
    protected static final String OPERATION="operation";
    protected static final String MPOINT = "mpoint";
    protected static final String POINT = "point";
    protected static final String REGION = "region";
    protected static final String MREGION = "mregion";
    
    private MenuVector MV = new MenuVector();
    private String result;
    private int streamCounter = 0;
    private ListExpr objects;
    
    public QueryconstructionViewer(){
        this.setLayout(new BorderLayout());
        
        MainPane = new MainPane();
        MainPane.setPreferredSize(new Dimension (500, 400));
        ObjectsPane = new ObjectsPane(this, objects);
        ObjectsPane.setPreferredSize(new Dimension (600, 80));
        OperationsPane.setPreferredSize(new Dimension (120, 400));
        
        OperationsPane.update();
        ObjectsPane.update();
        
        JScrollPane MainScrollPane = new JScrollPane(MainPane);
        JScrollPane ObjectsScrollPane = new JScrollPane(ObjectsPane);
        JScrollPane OperationsScrollPane = new JScrollPane(OperationsPane);
        OperationsScrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        
        this.add(ObjectsScrollPane, BorderLayout.NORTH);
        this.add(OperationsScrollPane, BorderLayout.EAST);
        this.add(MainScrollPane, BorderLayout.CENTER);
        
        JPanel buttonPanel = new JPanel();
        JButton run = new JButton("run");
        buttonPanel.add(run);
        JButton back = new JButton("back");
        buttonPanel.add(back);
        
        this.add(buttonPanel, BorderLayout.SOUTH);
        
        ActionListener runl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                runQuery();
            }
        };
        run.addActionListener(runl);
        
        ActionListener backl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                back();
            }
        };
        back.addActionListener(backl);
        
        //VC.execCommand("open database berlintest");
    }
    
    //adds an object to the main panel
    public void addObject(ObjectView object){
        if (object.getType().equals("rel"))
            streamCounter++;
        MainPane.addObject(object);
        MainPane.setToolTipText(getType());
        MainPane.repaint();
        OperationsPane.update();
        ObjectsPane.update();
    }
    
    public String getType () {
        String getTypeNL = VC.getCommandResult(MainPane.getStrings() + " getTypeNL").second().textValue();
        if (getTypeNL.startsWith("(stream"))
            this.result = "stream";
        else
            this.result = "relation";
        return getTypeNL;
    }
    
    //executes the constructed query
    public void runQuery() {
        System.out.println(MainPane.getStrings());
        if (VC.execCommand(MainPane.getStrings()) == 0) {
            System.out.println(VC.getCommandResult(MainPane.getStrings()));
            VC.execUserCommand(MainPane.getStrings());
        }
    }
    
    public void back() {
        MainPane.removeLastObject();
        MainPane.repaint();
        OperationsPane.update();
        ObjectsPane.update();
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
//        ListExpr LE = o.toListExpr(); // get the nested list of o
//        if(LE.listLength()!=2) // the length must be two
//            return false;
//        // the first element must be an symbol atom with content "inquiry"
//        if(LE.first().atomType()!=ListExpr.SYMBOL_ATOM || !LE.first().symbolValue().equals("inquiry"))
//            return false;
//        ListExpr VL = LE.second();
//        // the length of the second element must again be two
//        if(VL.listLength()!=2)
//            return false;
//        ListExpr SubTypeList = VL.first();
//        // the first element of this list must be a symbol atom
//        if(SubTypeList.atomType()!=ListExpr.SYMBOL_ATOM)
//            return false;
//        String SubType = SubTypeList.symbolValue();
//        // check for supported "sub types"
//        // the used constants just contain the appropriate String
//        if(SubType.equals(DATABASES) || SubType.equals(CONSTRUCTORS) || SubType.equals(OPERATORS) || SubType.equals(ALGEBRA) || SubType.equals(ALGEBRAS) || SubType.equals(OBJECTS) || SubType.equals(TYPES))
//            return true;
//        return false;
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
