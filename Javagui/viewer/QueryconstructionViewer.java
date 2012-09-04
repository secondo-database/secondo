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
import java.awt.Toolkit;
import java.awt.datatransfer.StringSelection;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import sj.lang.ListExpr;
import viewer.queryconstruction.*;

/**
 * this viewer class admits the construction of a query
 * 
 * @author Lisa Rentergent
 * @since 01.06.2012
 * @version 1.0
 * 
 */
public class QueryconstructionViewer extends SecondoViewer {
    
    protected ObjectPane objectPane;
    protected OperationsPane operationsPane;
    protected MainPane mainPane;
    
    private JScrollPane mainScrollpane;
    private JScrollPane objectScrollpane;
    private JScrollPane operationsScrollpane;
    
    private JPanel buttonPanel = new JPanel();
    protected JButton back = new JButton("back");
    private JButton run = new JButton("run");
    private JButton command = new JButton("copy command");
    
    private ListExpr objects;
    private ListExpr operators;
    
    public QueryconstructionViewer(){
        this.setLayout(new BorderLayout());
        
        mainPane = new MainPane(this);
        objectPane = new ObjectPane(this);
        operationsPane = new OperationsPane(this);
        
        mainScrollpane = new JScrollPane(mainPane);
        objectScrollpane = new JScrollPane(objectPane);
        objectScrollpane.setPreferredSize(new Dimension (600, 90));
        operationsScrollpane = new JScrollPane(operationsPane);
        operationsScrollpane.setPreferredSize(new Dimension (120, 30));
        
        this.add(objectScrollpane, BorderLayout.NORTH);
        this.add(operationsScrollpane, BorderLayout.EAST);
        this.add(mainScrollpane, BorderLayout.CENTER);
        
        JButton newQuery = new JButton("new");
        buttonPanel.add(newQuery);
        
        run.setEnabled(false);
        buttonPanel.add(run);
        buttonPanel.add(back);
        buttonPanel.add(command);
        
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
        
        ActionListener commandl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                copyCommand();
            }
        };
        command.addActionListener(commandl);
        
        ActionListener addObjl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if (VC != null) {
                    listObjects();
                }
            }
        };
        addObj.addActionListener(addObjl);
    }
    
    /**
     * adds an object to the main panel
     * @param object new object
     */
    public void addObject(ObjectView object){
        mainPane.addObject(object);
        update();
    }
    
    /**
     * adds an operation to the main panel
     * @param operation new operation
     */
    public void addOperation(Operation operation){
        mainPane.addOperation(operation);
        update();
    }
    
    public ListExpr getType(String query) {
        ListExpr getTypeNL = null;
        if (VC != null) {
            getTypeNL = VC.getCommandResult(query + " getTypeNL");
        }
        
        return getTypeNL;
    }
    
    public ViewerControl getViewerControl() {
        return VC;
    }
    
    public String getCount(String query) {
        if (VC != null) {
            ListExpr getTypeNL = VC.getCommandResult("query " + query + " count");
            
            if (getTypeNL != null)
                return getTypeNL.second().toString();
        }
        
        return "0";
    }
    
    public String[] getParameters() {
        return mainPane.getParameters();
    }
    
    public boolean checkAttributes() {
        return mainPane.checkAttributes();
    }
    
    private void copyCommand() {
        Toolkit.getDefaultToolkit().getSystemClipboard().setContents(
                        new StringSelection(mainPane.getStringsQuery()), null);
    }
    
    /**
     * Executes the constructed query.
     */
    private void runQuery() {
        if (VC.execCommand(mainPane.getStringsQuery()) == 0) {
            VC.execUserCommand(mainPane.getStringsQuery());
        }
        else {
            System.out.println(VC.getCommandResult(mainPane.getStringsQuery()));
            System.out.println("Kann nicht ausgeführt werden. Fehler: "+VC.execCommand(mainPane.getStringsQuery()));
        }
    }
    
    /**
     * Update the three panels.
     */
    public void update() {
        String state = mainPane.update();
        /* check if the query is runnable */
        if (!state.equals("") && !state.contains("stream")) {
            run.setEnabled(true);
        }
        
        objectPane.update();
        operationsPane.update();
    }
    
    
    public void back() {
        mainPane.removeLastObject();
        update();
    }
    
    /**
     * Set the ViewerControl object to communicate with the server.
     * @param VC ViewerControl object
     */
    public void setViewerControl(ViewerControl VC){
        super.setViewerControl(VC);
        if (VC != null) {
            if (objects == null) {
                listObjects();
            }
        }
    }
    
    public void setObjects(ListExpr ob) {
        if (ob != null) {
            this.objects = ob;
            objectPane.addObjects(this.objects);
        }
    }
    
    public void setOperators(ListExpr op) {
        if (op != null) {
            this.operators = op;
            operationsPane.addOperations(this.operators);
        }
    }
    
    /**
     * Add all database objects to the object panel
     * and check if the table with the operation objects exists.
     * If it does not exist restore it from the database.
     */
    protected void listObjects() {
        objects = VC.getCommandResult("list objects");
        if (objects != null) {
            operators = VC.getCommandResult("query QueryOperators feed sortby[OpName asc] consume");
            if (operators == null) {
                VC.execCommand("restore QueryOperators from '../bin/QueryOperators'");
                operators = VC.getCommandResult("query QueryOperators feed sortby[OpName asc] consume");
            }
            operationsPane.addOperations(operators);
            objectPane.addObjects(objects);
        }
    }
    
    public ArrayList<ObjectView> getObjects(){
        return objectPane.getObjects();
    }
    
    public ListExpr getObjectList(){
        return objects;
    }
    
    public ListExpr getOperatorList(){
        return operators;
    }
    
    /** returns QueryconstructionViewer */
    public String getName(){
        return "QueryconstructionViewer";
    }
    
    /** remove all containing objects */
    public void removeAll(){
        mainPane = new MainPane(this);
        mainPane.setPreferredSize(new Dimension (500, 400));
        mainScrollpane.setViewportView(mainPane);
        mainScrollpane.repaint();
        update();
    }
    
    public boolean addObject(SecondoObject o){
        return false;
    }
    
    public void removeObject(SecondoObject o){
        
    }
    
    public boolean selectObject (SecondoObject o) {
        return false;
    }    
    
    public boolean canDisplay(SecondoObject o){
        return false;
    }
    
    public boolean isDisplayed(SecondoObject o) {
        return false;
    }

    public void enableTestmode (boolean on) {
        //does nothing
    }
}
