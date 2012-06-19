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
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ScrollPaneConstants;
import sj.lang.ListExpr;
import viewer.queryconstruction.MainPane;
import viewer.queryconstruction.ObjectPane;
import viewer.queryconstruction.ObjectView;
import viewer.queryconstruction.OperationsPane;

/**
 * this viewer class admits the construction of a query
 * 
 * @author Lisa Rentergent
 * @since 01.06.2012
 * @version 1.0
 * 
 */
public class QueryconstructionViewer extends SecondoViewer {
    
    private ObjectPane ObjectPane = new ObjectPane(this);
    private OperationsPane OperationsPane = new OperationsPane(this);
    private MainPane MainPane;
    
    private MenuVector MV = new MenuVector();
    private String result;
    private int streamCounter = 0;
    private static ListExpr objects;
    
    public QueryconstructionViewer(){
        this.setLayout(new BorderLayout());
        
        MainPane = new MainPane();
        MainPane.setPreferredSize(new Dimension (500, 400));
        
        OperationsPane.update();
        
        JScrollPane MainScrollPane = new JScrollPane(MainPane);
        JScrollPane ObjectsScrollPane = new JScrollPane(ObjectPane);
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
    }
    
    //adds an object to the main panel
    public void addObject(ObjectView object){
        if (object.getType().equals("rel"))
            streamCounter++;
        MainPane.addObject(object);
        update();
    }
    
    public String getType() {
        String getTypeNL = "no result";
        if (VC != null) { 
            
            if (VC.execCommand(MainPane.getStrings() + " getTypeNL") == 0) {
                getTypeNL = VC.getCommandResult(MainPane.getStrings() + " getTypeNL").second().textValue();
                if (getTypeNL.startsWith("(stream")) {
                    this.result = "stream";
                }
                else {
                    this.result = "relation";
                }
            }
            else {
                getTypeNL += MainPane.getState();
            }
        }
        
        return getTypeNL;
    }
    
    public int getState() {
        return MainPane.getState();
    }
    
    //executes the constructed query
    public void runQuery() {
        System.out.println(MainPane.getStrings());
        if (VC.execCommand(MainPane.getStrings()) == 0) {
            System.out.println(VC.getCommandResult(MainPane.getStrings()));
            VC.execUserCommand(MainPane.getStrings());
        }
        else {
            System.out.println("Fehler!");
        }
    }
    
    //sets the panels up to date and repaints them
    public void update() {
        MainPane.setToolTipText(getType());
        MainPane.repaint();
        OperationsPane.update();
        ObjectPane.update();
    }
    
    public void back() {
        MainPane.removeLastObject();
        update();
    }
    
    @Override
    public void setViewerControl(ViewerControl VC){
        //super.setViewerControl(VC);
        if (VC != null) {
            this.VC = VC;
            VC.execCommand("open database berlintest");
            objects = VC.getCommandResult("list objects");
            ObjectPane.addObjects(objects);
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
