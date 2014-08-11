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
import viewer.queryconstruction2.*;
import java.lang.String;
import java.io.*;

/**
 * this viewer class admits the visual construction of a query
 * 
 * @author Thomas Alber
 * @since 01.12.2013
 * @version 2.0
 * 
 */
public class QueryconstructionViewer2 extends SecondoViewer {
   
   //scrollpanels in the viewer
    protected ObjectPane objectPane;
    protected OperationsPane2 operationsPane;
    protected MainPane mainPane;
    private JScrollPane mainScrollpane;
    private JScrollPane objectScrollpane;
    private JScrollPane operationsScrollpane;
    
    //buttons
    private JPanel buttonPanel = new JPanel();
    protected JButton back = new JButton("back");
    private JButton run = new JButton("send Query");
    private JButton command = new JButton("copy command");
           
    //lists of all objects and operators in nested list format
    private ListExpr objects;
    //private ListExpr operators;
    private ListExpr opsigs = new ListExpr();
    private ListExpr opspecs = new ListExpr();

    
    /**
     * Construct the viewer window and Init files. 
     */
    public QueryconstructionViewer2(){
        this.setLayout(new BorderLayout());
        
        mainPane = new MainPane(this);
        objectPane = new ObjectPane(this, mainPane);
        operationsPane = new OperationsPane2(mainPane, this);
        
        mainScrollpane = new JScrollPane(mainPane);
        objectScrollpane = new JScrollPane(objectPane);
        objectScrollpane.setPreferredSize(new Dimension (600, 90));
        operationsScrollpane = new JScrollPane(operationsPane);
        operationsScrollpane.setPreferredSize(new Dimension (230, 28));

        this.add(objectScrollpane, BorderLayout.NORTH);
        this.add(operationsScrollpane, BorderLayout.EAST);
        this.add(mainScrollpane, BorderLayout.CENTER);

        JButton newQuery = new JButton("new Query");
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
                removeAllObjects();
            }
        };
        newQuery.setToolTipText("make the mainpanel empty for new query");
        newQuery.addActionListener(newl);
        
        ActionListener runl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                runQuery();
            }
        };
        run.setToolTipText("send the query to SECONDO system");
        run.addActionListener(runl);
        
        ActionListener backl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                back();
            }
        };
        back.setToolTipText("remove the last added object");
        back.addActionListener(backl);
        
        ActionListener commandl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                copyCommand();
            }
        };
        command.setToolTipText("copy query command to clipboard");
        command.addActionListener(commandl);
        
        ActionListener addObjl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if (VC != null) {
                    listObjects();
                }
            }
        };
        addObj.setToolTipText("add objects from database to objectpanel");
        addObj.addActionListener(addObjl);

        //Init
        createfiles();

    }


    /**
     * Create the files from sigs and specs for 
     *  the operator signatures and the operator syntaxes.
     * 
     */
    public void createfiles() {

      try {
        String sigscommand = "../Tools/TypeMap/OpSigParser/OpSig";
        Runtime.getRuntime().exec(sigscommand);
        String specscommand = "../Tools/TypeMap/OpSpecParser/OpSpec";
        Runtime.getRuntime().exec(specscommand);
      }  
      catch (IOException e) {  
            e.printStackTrace();  
      }

    }

    /**
     * Set the list expressions of the operator signatures  
     *  and the operator syntaxes from their files.
     * 
     */
    public void setfileLists() {

        String sigsPath = "../Tools/TypeMap/OpSigParser/OpSigsArgs.tmp";
        //ListExpr opsigs = new ListExpr();
        if(opsigs.readFromFile(sigsPath)!=0) {
            System.out.println("I can't load the file");
        }
        else {
            System.out.println("opsigs loaded");
        }
        String specsPath = "../Tools/TypeMap/OpSpecParser/OpSpecs.tmp";
        //ListExpr opspecs = new ListExpr();
        if(opspecs.readFromFile(specsPath)!=0) {
            System.out.println("I can't load the file");
        }
        else {
            //Example: (+ "(o # o)")
            System.out.println("opspecs loaded");
        }

    }

    /**
     * Get the list expression of the operator signatures.
     * @return this.opsigs
     */
    public ListExpr getOpSigs(){
        return opsigs;
    }

    /**
     * Get the list expression of the operator syntaxes.
     * @return this.opspecs
     */
    public ListExpr getOpSpecs(){
        return opspecs;
    }

    /**
     * Get the properties list expression of a possible operator  
     *  from secondo server.
     * @param opName
     * @return Nested List of OperatorInfo
     */
    public ListExpr getOperatorInfo(String opName) {
        ListExpr opInfo = null;
        if (VC != null) {
            //query SEC2OPERATORINFO feed filter[.Name="+"] consume
            // (Example)
            opInfo = VC.getCommandResult("query SEC2OPERATORINFO feed "
                                + "filter[.Name="+"\""+opName+"\"]"
                                + "consume");
        }
        return opInfo;
    }

    public ListExpr getOperatorInfo2Mean(String opName) {
        ListExpr opInfo2 = null;
        if (VC != null) {
            //query SEC2OPERATORINFO feed filter[.Name="+"]
            //			     project[Meaning] consume
            // (Example)
            opInfo2 = VC.getCommandResult("query SEC2OPERATORINFO feed "
                                + "filter[.Name="+"\""+opName+"\"]"
                                + "project[Meaning] consume");
        }
        return opInfo2;
    }
    
    /**
     * Send the query to the secondo server and return the result.
     * @param query
     * @return query result
     */
    public ListExpr getType(String query) {
        ListExpr getTypeNL = null;
        if (VC != null) {
            getTypeNL = VC.getCommandResult(query + " getTypeNL");
        }
        
        return getTypeNL;
    }
    
    /**
     * Get the object for the communication with the server.
     * @return 
     */
    public ViewerControl getViewerControl() {
        return VC;
    }
    
    /**
     * Get the amount of tuples in a relation, if the object is a relation.
     * @param query
     * @return 0, if the query result is not countable
     */
    public String getCount(String query) {
        if (VC != null) {
            ListExpr getTypeNL = 
                    VC.getCommandResult("query " + query + " count");
            
            if (getTypeNL != null)
                return getTypeNL.second().toString();
        }
        
        return "0";
    }
        
    /**
     * Copy the active query in the main panel to the system clipboard.
     */
    private void copyCommand() {
        Toolkit.getDefaultToolkit().getSystemClipboard().setContents(
                        new StringSelection(mainPane.getStringsQuery()), null);
    }
    
    /**
     * Executes the constructed query.
     */
    private void runQuery() {
        if (!VC.execUserCommand(mainPane.getStringsQuery())) {
            System.out.println("Kann nicht ausgeführt werden. Fehler: "
                    +VC.execCommand(mainPane.getStringsQuery()));
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
        operationsPane.repaint();
    }
    
    /**
     * Set the main panel one step back.
     */
    private void back() {
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
    
    /**
     * Add the objects of the database to the object panel.
     * Used in the nested window.
     * @param ob list of the objects to add
     */
    public void setObjects(ListExpr ob) {
        if (ob != null) {
            this.objects = ob;
            objectPane.addObjects(this.objects);
        }
    }
    
    /**
     * Add the operators of the database to the operators panel.
     * Used in the nested window.
     * @param opsigs list of the operator signatures to add
     * @param opspecs list of the operator syntaxes to add

     */
    public void setOperators2(ListExpr opsigs, ListExpr opspecs) {
        if (opsigs != null && opspecs != null) {
            operationsPane.addOperations2(opsigs, opspecs);
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
            //get the operatornames and their meanings
            ListExpr opsMean = null;
            opsMean = VC.getCommandResult("query SEC2OPERATORINFO feed "
                                        + "project[Name, Meaning] consume");
            operationsPane.addOperatorsMean(opsMean);
            setfileLists();
            operationsPane.addOperations2(opsigs, opspecs);
            objectPane.addObjects(objects);
        }
    }
    
    /**
     * Get a list of all attribute objects in the object panel.
     * @return 
     */
    public ArrayList<ObjectView> getAttributes(){
        return objectPane.getAttributes();
    }
    
    /**
     * Get a list of all objects in the object panel.
     * @return 
     */
    public ArrayList<ObjectView> getObjects(){
        return objectPane.getObjects();
    }
    
    /**
     * Get the list expression of the objects.
     * @return this.objects
     */
    public ListExpr getObjectList(){
        return objects;
    }
    
    /**
     * Get the list expression of the operators.
     * @return this.operators
     */
    //public ListExpr getOperatorList(){
    //    return operators;
    //}
    
    /**
     * @return "QueryconstructionViewer2"
     */
    public String getName(){
        return "QueryconstructionViewer2";
    }
    
    /**
     * Remove all containing objects and renew the main panel.
     */
    private void removeAllObjects(){
        mainPane.removeAllObjects();
        update();
    }
    
    public boolean addObject(SecondoObject o){
        return false;
    }
    
    public void removeObject(SecondoObject o){
        //does nothing
    }
    
    public void removeAll(){
        //does nothing
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
