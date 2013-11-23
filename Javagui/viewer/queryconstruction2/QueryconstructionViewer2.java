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
import javax.swing.JTextField;	///
import sj.lang.ListExpr;
import viewer.queryconstruction2.*;
import java.lang.String;		///
import java.io.*;	///

/**
 * this viewer class admits the visual construction of a query
 * 
 * @author Thomas Alber
 * @since 01.06.2012
 * @version 1.0
 * 
 */
public class QueryconstructionViewer2 extends SecondoViewer {
   
   //scrollpanels in the viewer
    protected ObjectPane objectPane;
    protected OperationsPane23 operationsPane;
    protected MainPane mainPane;
    private JScrollPane mainScrollpane;
    private JScrollPane objectScrollpane;
    private JScrollPane operationsScrollpane;
    
    //buttons
    private JTextField mOpText = new JTextField("10,3", 20);	/// 
    private String fieldtext; ///

    private JPanel buttonPanel = new JPanel();
    protected JButton back = new JButton("back");
    private JButton run = new JButton("run");
    private JButton command = new JButton("copy command");
           
    //lists of all objects and operators in nested list format
    private ListExpr objects;
    private ListExpr operators;

    
    /**
     * Construct the viewer window.
     */
    public QueryconstructionViewer2(){
        this.setLayout(new BorderLayout());
        
        mainPane = new MainPane(this);
        objectPane = new ObjectPane(this, mainPane);
        operationsPane = new OperationsPane23(mainPane, this);
        
        mainScrollpane = new JScrollPane(mainPane);
        objectScrollpane = new JScrollPane(objectPane);
        objectScrollpane.setPreferredSize(new Dimension (600, 90));
        operationsScrollpane = new JScrollPane(operationsPane);
        operationsScrollpane.setPreferredSize(new Dimension (230, 30));

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

        JButton testObj = new JButton("mOp");	/// 
        buttonPanel.add(testObj);		///

	buttonPanel.add(mOpText);		///
          
        this.add(buttonPanel, BorderLayout.SOUTH);

        
        ActionListener newl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                removeAllObjects();
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
        command.setToolTipText("copy command to clipboard");
        command.addActionListener(commandl);
        
        ActionListener addObjl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                if (VC != null) {
                    listObjects();
                }
            }
        };
        addObj.addActionListener(addObjl);
///
        ActionListener testObjl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
		opsList(mOpText.getText().trim()); //mOpText aus Eingabefeld
//		parse();
            }
        };
        testObj.addActionListener(testObjl);
///        
    }





// Neue Methoden //



    //Test ListExpr-Ausgabe
    public void opsList(String mOpin) {

//	ListExpr ops = null;
//	ops = getOperatorList();
//	System.out.println(ops);	//Operatoren aus Tabelle QueryOperators

	ListExpr ops2 = null;
	ListExpr opsMean = null;



        if (VC != null) {
            ops2 = VC.getCommandResult("query matchingOperators("
			+ mOpin + ") project[OperatorName] consume");

	    opsMean =  VC.getCommandResult("query matchingOperators("
			+ mOpin + ") project[Meaning] consume");


	}
// 	System.out.println(ops2);	//anwendbare Operatoren
// 	System.out.println(opsMean);	//Bedeutung Operatoren


        operationsPane.mOperators(ops2, opsMean);

	System.out.println(mainPane.getStringsQuery());	//aktuelle Query

    }


    //zum Test mit mOps-Eingabe
    public String getOpObject() {
	String mOpin = mOpText.getText().trim(); 
	return mOpin;
    }


///

    /**
     * Get the properties list expression of a possible operator  
     *  from secondo server.
     * @param opName
     * @param opObject
     * @return Nested List of OperatorInfo
     */
    public ListExpr getOperatorInfo(String opName, String opObject) {
	ListExpr opInfo = null;
        if (VC != null) {
	    //query matchingOperators(10,3) filter[.OperatorName="+"] consume
	    // (Example)
     	    opInfo = VC.getCommandResult("query matchingOperators(" + opObject + ")"
				+ "filter[.OperatorName="+"\""+opName+"\"]"
				+ "consume");
	}
        return opInfo;
    }








    public void parse() {
    
	try {
/*
    String line;
    Process process = Runtime.getRuntime().exec(command);
    Reader r = new InputStreamReader(process.getInputStream());
    BufferedReader in = new BufferedReader(r);
    while((line = in.readLine()) != null) System.out.println(line);
    in.close();
*/


	String[] command = {"Parser2", "specs", "2>specAusgabe", ">specErgebnis"};
	Runtime.getRuntime().exec(command);

	}
	catch (Exception e) {
	    System.err.println(e.toString());
	}

    }




// Neue Methoden Ende //




    
    /**
     * adds an object to the main panel
     * @param object new object
//     */
//    public void addObject(ObjectView object){
//        mainPane.addObject(object);
//        update();
//    }
    
//    /**
//     * adds an operation to the main panel
//     * @param operation new operation
//     */
//    public void addOperation(Operation operation){
//        mainPane.addOperation(operation);
//        update();
//    }
    
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
     * Get the types of all active objects in the main panel.
     * @return array of types
//     */
//    public String[] getParameters() {
//        return mainPane.getParameters();
//    }
    
    /**
     * Send a request to the main panel to check the attributes 
     * for duplicate names.
     * @return 
     */
//    public boolean checkAttributes() {
//        return mainPane.checkAttributes();
//    }
    
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
     * @param op list of the operators to add
     */
    public void setOperators(ListExpr op) {
        if (op != null) {
            this.operators = op;
            operationsPane.addOperations(this.operators);///
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
            operators = VC.getCommandResult(
                    "query QueryOperators feed sortby[OpName asc] consume");
            if (operators == null) {
                VC.execCommand(
                        "restore QueryOperators from '../Javagui/viewer/"
                        + "queryconstruction2/QueryOperators'");
                operators = VC.getCommandResult(
                        "query QueryOperators feed sortby[OpName asc] consume");
            }
            operationsPane.addOperations(operators);///
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
    public ListExpr getOperatorList(){
        return operators;
    }
    
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
