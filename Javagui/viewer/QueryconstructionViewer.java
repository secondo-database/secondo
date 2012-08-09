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
import java.util.ArrayList;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import sj.lang.ListExpr;
import viewer.queryconstruction.MainPane;
import viewer.queryconstruction.ObjectPane;
import viewer.queryconstruction.ObjectView;
import viewer.queryconstruction.OperationsPane;
import viewer.queryconstruction.Operation;

/**
 * this viewer class admits the construction of a query
 * 
 * @author Lisa Rentergent
 * @since 01.06.2012
 * @version 1.0
 * 
 */
public class QueryconstructionViewer extends SecondoViewer {
    
    protected ObjectPane ObjectPane = new ObjectPane(this);
    protected OperationsPane OperationsPane = new OperationsPane(this);
    protected MainPane MainPane;
    
    private JScrollPane MainScrollPane;
    private JScrollPane ObjectsScrollPane;
    private JScrollPane OperationsScrollPane;
    
    private ArrayList<ObjectView> objectList = new ArrayList<ObjectView>();
    
    private MenuVector MV = new MenuVector();
    private static ListExpr objects;
    
    public QueryconstructionViewer(){
        this.setLayout(new BorderLayout());
        
        MainPane = new MainPane(this);
        MainPane.setPreferredSize(new Dimension (500, 400));
        OperationsPane.update();
        
        MainScrollPane = new JScrollPane(MainPane);
        ObjectsScrollPane = new JScrollPane(ObjectPane);
        ObjectsScrollPane.setPreferredSize(new Dimension (600, 90));
        OperationsScrollPane = new JScrollPane(OperationsPane);
        OperationsScrollPane.setPreferredSize(new Dimension (120, 300));
        
        this.add(ObjectsScrollPane, BorderLayout.NORTH);
        this.add(OperationsScrollPane, BorderLayout.EAST);
        this.add(MainScrollPane, BorderLayout.CENTER);
        
        JPanel buttonPanel = new JPanel();
        JButton newQuery = new JButton("new");
        buttonPanel.add(newQuery);
        JButton run = new JButton("run");
        buttonPanel.add(run);
        JButton back = new JButton("back");
        buttonPanel.add(back);
        JButton addObj = new JButton("add objects");
        buttonPanel.add(addObj);
        
        this.add(buttonPanel, BorderLayout.SOUTH);
        
        ActionListener newl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                removeAll();
            }
        };
        newQuery.addActionListener(newl);
        
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
        
        ActionListener addObjl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if (VC != null) {
                    listObjects();
                }
            }
        };
        addObj.addActionListener(addObjl);
    }
    
    //adds an object to the main panel
    public void addObject(ObjectView object){
        //TODO prüfen ob das Objekt einen neuen Stream erzeugt
        MainPane.addObject(object);
        update();
    }
    
    //adds an object to the main panel
    public void addOperation(Operation operation){
        //TODO prüfen ob das Objekt einen neuen Stream erzeugt
        MainPane.addOperation(operation);
        update();
    }
    
    public ListExpr getType(String query) {
        ListExpr getTypeNL = null;
        if ((VC != null) && (VC.execCommand(query + " getTypeNL") == 0)) {
            getTypeNL = VC.getCommandResult(query + " getTypeNL");
        }
        
        return getTypeNL;
    }
    
    //returns the state of the actual query
    public int getState() {
        return MainPane.getState();
    }
    
    public String[] getParameters() {
        return MainPane.getParameters();
    }
    
    //executes the constructed query
    public void runQuery() {
        System.out.println(MainPane.getStrings());
        if (VC.execCommand(MainPane.getStrings()) == 0) {
            VC.execUserCommand(MainPane.getStrings());
        }
        else {
            System.out.println(VC.getCommandResult(MainPane.getStrings()));
            System.out.println("Kann nicht ausgeführt werden.");
        }
    }
    
    //sets the panels up to date and repaints them
    public void update() {
        MainPane.update();
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
            objects = VC.getCommandResult("list objects");
            
            if (objects != null)
                objectList = ObjectPane.addObjects(objects);
        }
    }
    
    public void listObjects() {
        if (VC != null) {
            objects = VC.getCommandResult("list objects");
            
            if (objects != null)
                objectList = ObjectPane.addObjects(objects);
            
            update();
        }
        else {
            System.out.println("Fehler: noch keine Datenbank geöffnet");
        }
    }
    
    public ArrayList<ObjectView> getObjects(){
        return objectList;
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
        MainPane = new MainPane(this);
        MainPane.setPreferredSize(new Dimension (500, 400));
        MainScrollPane.setViewportView(MainPane);
        MainScrollPane.repaint();
        this.update();
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
