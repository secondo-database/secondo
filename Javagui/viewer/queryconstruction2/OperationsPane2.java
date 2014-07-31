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

package viewer.queryconstruction2;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import javax.swing.JButton;
import javax.swing.ToolTipManager;
import javax.swing.JLabel;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer2;


/**
 * Panel for the allowed operations.
 */
public class OperationsPane2 extends JComponent implements MouseListener {
    
    //list of all operations
    private ArrayList<Operation> operations = new ArrayList<Operation>();
    private ArrayList<Operation> operationsTemp = new ArrayList<Operation>();
    private ArrayList<ListExpr> opMeanings = new ArrayList<ListExpr>();


    private QueryconstructionViewer2 mainViewer;
    private MainPane mainPane;
    private Operation rename;
    private String longestName = "";

    //graphic elements
    private JButton opSet = new JButton("Set OP-Selection");	 
    private JLabel opSetState = new JLabel("All possible Operators",
                                                            JLabel.CENTER);


    /**
     * Generate a new Panel for the Operations.
     * @param main MainPane
     * @param mainViewer QueryconstructionViewer2
     */
    public OperationsPane2(MainPane main,
                           QueryconstructionViewer2 mainViewer) {
        this.mainPane = main;
        this.mainViewer = mainViewer;
        this.setLayout(new GridLayout(0, 1));

        ToolTipManager.sharedInstance().setDismissDelay(600000); //10 minutes

        ActionListener opSetl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                new OperationsSelectView();
            }
        };
        opSet.setToolTipText("Set the Operations-Selection");
        opSet.addActionListener(opSetl);

    }

    /**
     * Add the operators meanings to the operators meanings list.
     * @param opsMean 
     */    
    public void addOperatorsMean(ListExpr opsMean) {
        opMeanings.clear();
        while (opsMean.listLength() > 1) {
            ListExpr objects = opsMean.second();
            while (objects.listLength() > 0) {
                opMeanings.add(objects);
                objects = objects.rest();
            }
            opsMean = opsMean.rest();
        }
    }

    /**
     * Add a new operation to the operation list.
     * @param op2 
     */
    private void addOperation2(Operation op2) {
        op2.addMouseListener(this);
        operations.add(op2);
    }

    /**
     * Add the "new" operations to the operation list.
     * 
     */
    private void addNewOperations() {
        String[] newOps = {"const", "int", "string"};
        for (String newop: newOps) {
            String opName2 = "new " + newop;
            String opObjects2 = "";
            String opParams2 = newop;
            String opSyntax2 = "";
            if (newop.equals("string")) { 
                opSyntax2 = "\"p\"";
            }
            else {
                opSyntax2 = "p";
            }
            String opResult2 = "";           
            Operation op2 = new Operation(opName2, 
                                    opObjects2.split(";"), opSyntax2, 
                                    opParams2.split(";"), opResult2);
            addOperation2(op2);
            op2.setToolTipText("Creates a new object");
        }
    }
    
    /**
     * Generate a list of operations of two nested list in files.
     * @param opSigs
     * @param opSpecs 
     */
    public void addOperations2(ListExpr opSigs, ListExpr opSpecs) {
        ListExpr opSpecsTemp = opSpecs;
        operations.clear();
        addNewOperations();
        //processing the nested lists
        while (opSigs.listLength() > 0) {
            String opName2 = opSigs.first().second().stringValue();
            String opSyntax2 = ""; // "opSignature2" is not correct
            //search for spec of an operator
            while (opSpecsTemp.listLength() > 0) {
                String opName2spec = opSpecsTemp.first().first().stringValue();
                if (opName2spec.equals(opName2)){
                    opSyntax2 = opSpecsTemp.first().second().stringValue();
                    opSpecsTemp = opSpecs;
                    break;
                }
                opSpecsTemp = opSpecsTemp.rest();
            }
            
            String opObjects2 = opSigs.first().third().stringValue();
            String opParams2 = opSigs.first().fourth().stringValue();
            String opResult2 =  "";
            Operation op2 = new Operation(opName2, 
                                        opObjects2.split(";"), opSyntax2, 
                                        opParams2.split(";"), opResult2);
            addOperation2(op2);
            //operator-tooltip
            for ( Iterator iterMean = opMeanings.iterator();
                                         iterMean.hasNext(); ) {
                ListExpr opMean = (ListExpr)iterMean.next();
                String opNameList = opMean.first().first().stringValue();
                String opMeaning = opMean.first().second().textValue();
                if (opName2.equals(opNameList)) {
                    String note = "...more info with rightclick";
                    if (opMeaning.length() > 80) {
                        op2.setToolTipText("<html>" + opMeaning.substring(0, 80)
                                                + "<font color=\"#FF55FF\">"
                                                + note + "</font></html>");
                    }
                    else {
                        op2.setToolTipText("<html>" + opMeaning
                                                + "<font color=\"#FF55FF\">  "
                                                + note + "</font></html>");
                    }
                }
            }
            // end tooltip

            //save the rename operator as variable
            if (opName2.equals("rename")){
                rename = op2;
            }

            opSigs = opSigs.rest();
        }
        this.repaint();
    }
    
    /**
     * Recursive method to find a combination array, with one 1 in
     * each row and each column.
     * @param level
     * @param typeMatrix
     * @param resultArray
     * @return 
     */
    private int[] checkComb(int level, int[][] typeMatrix, int[] resultArray){
        
        int n = resultArray.length;
        int index = 0;
        int[] typeArray = typeMatrix[level];
        for (int check: resultArray) {
            if (check == 0) {
                if (typeArray[index] == 1) {
                    resultArray[index] = 1;
                    if (level < n-1)
                        resultArray = checkComb(level + 1, typeMatrix, 
                                resultArray);
                    if (resultArray == null)
                        return null;
                }
            }
            if (resultArray != null) {
                boolean result = true;
                for (int i: resultArray) {
                    result = (result && (i == 1));
                }
                if (result && (level == n-1))
                    return resultArray;
            }
            index++;
        }
        if (level == n-1)
            return null;
        return resultArray;
    }
    
    /**
     * Search in the matrix for a right combination.
     * @param typeMatrix
     * @return 
     */
    private boolean checkMatrix(int[][] typeMatrix){
        int[] resultArray = new int[typeMatrix.length];
        for (int i: resultArray) {
            i = 0;
        }
        resultArray = checkComb(0, typeMatrix, resultArray);
        if (resultArray != null){
            boolean result = true;
            for (int i: resultArray) {
                result = (result && (i == 1));
            }
            return result;
        }
        else {
            return false;
        }
    }
    
    /**
     * Check if the type is in the array of usable types.
     * @param type
     * @param tArray array of types
     * @return type is part of the array
     */
    private boolean typeInArray(String type, String[] tArray){
        for (String s: tArray) {
            if (s.trim().equals(type))
                return true;
            //stream of data or one attribute is needed
            if (s.trim().startsWith("stream") 
                    && s.contains("data") 
                    && (type.startsWith("stream ") 
                    || this.mainPane.getAttributesCount() == 1))
                return true;
        }
        return false;
    }
    
    /**
     * Check if the type is in the array of usable types.
     * @param type
     * @param tArray array of types
     * @return type is part of the array
     */
    private int[] typeInArray2(String type, String[] tArray){
        int[] result = new int[tArray.length];
        int i = 0;
        for (String s1: tArray) {
            String[] sArray = s1.split(",");
            
            if (typeInArray(type, sArray))
                result[i] = 1;
            else
                result[i] = 0;
            i++;
        }
        return result;
    }
    
    /**
     * Paint all operations into the panel, 
     * only allowed operations should visible.
     * @param g 
     */
    public void paintComponent(Graphics g) {
        this.removeAll();
        operationsTemp.clear();
        //this.add(opSet);      //Button for OP-Selection
        this.add(opSetState); //State of OPSelection

        longestName = "";

        // objects, that are given by the actual query
        String[] viewerParam = mainPane.getParameters();
        
        for ( Iterator iter = operations.iterator(); iter.hasNext(); ) {
            Operation op = (Operation)iter.next();
            // objects, that are needed by the operation
            String[] operationObjects = op.getObjects();
            
            int viewerCount = viewerParam.length;
            String object0 = operationObjects[0].trim();
            if (object0.equals("")) {
                viewOperation(op);
            }
            int i = 0;
            if ((viewerParam.length > 1) && (viewerParam[0] == null)) {
                viewerCount--;
                i++;
            }
            
            /* 
             * If the count of active objects equals the count of objects, 
             * the operation needs, the viewer checks if the 
             * types are the same.
             */
            if (viewerCount == operationObjects.length) {
                int[][] typesIn = new int[viewerCount][viewerCount];
                
                int index = 0;
                // generating the matrix
                for (String viewerStr : viewerParam) {
                    typesIn[index] = this.typeInArray2(viewerStr.trim(), 
                            operationObjects);
                    index++;
                }
                
                boolean contains = false;
                boolean contains2 = contains;
                if (viewerCount == 1) {
                    if (typesIn[0][0] == 1)
                        contains = true;
                }
                if (viewerCount == 2) {
                    if ((typesIn[0][0] == 1) && (typesIn[1][1] == 1))
                        contains = true;
                    if ((typesIn[1][0] == 1) && (typesIn[0][1] == 1))
                        contains = true;
                }
                if (viewerCount > 2) {
                    contains = checkMatrix(typesIn);
                }
                if (contains) {
                    // Check if there are equivalent combinations
                    // e.g. (int real) (real int)
                    contains2 = contains;
                    for ( Iterator iter2 = operationsTemp.iterator();
                                                     iter2.hasNext(); ) {
                        Operation op2 = (Operation)iter2.next();
                        if (op2.getOperationName().equals
                            (op.getOperationName()) ) {
                            contains2 = false;
                            break;
                        }
                    }
                }
                if (contains2) {
                    viewOperation(op);
                    operationsTemp.add(op);
                }

            } // end if (viewerCount == operationObjects.length)

        } // end for (all operations)

        int width = g.getFontMetrics().stringWidth(longestName);
        this.setPreferredSize(new Dimension((width + 40), 
                (30 * this.getComponentCount())));
        
        this.revalidate();
    }
    
    /**
     * Add an operation to the panel.
     * @param op 
     */
    private void viewOperation(Operation op) {
        add(op);

        // fit the width of the panel to the longest operatorname
        if (longestName.length() < op.getOperationName().length()) {
            longestName = op.getOperationName();
        }
    }
    
    public void mouseClicked ( MouseEvent arg0 ) {
        if ((arg0.getClickCount () == 1) && (arg0.getComponent() != null)) {
            if (!arg0.getComponent().equals(this)) {
                Operation element = (Operation)arg0.getComponent();

                //left click adds a copy of the selected operation to
                // the main panel
                if (arg0.getButton() == 1) {
                    //check if objects have to be renamed
                    if ((element.getObjects().length == 2) && 
                            (element.getObjects()[0].equals("stream")) && 
                            (element.getObjects()[1].equals("stream")) && 
                            (element.getParameter().length > 0)) {
                       if (!mainPane.checkAttributes()) {
                            mainPane.addOperation(rename.copy());
                            return;
                       }
                    }
                    mainPane.addOperation(element.copy());
                }
                //right click shows operatorinfo
                if (arg0.getButton() == 3) {
                    try {
                        String opName = element.getOperationName();
                        ListExpr opInfo = mainViewer.getOperatorInfo(opName);
                        new OperationsInfoDialog(120, 40, opInfo);
                    }
                    catch ( NullPointerException e ) {
                        System.out.println("No Operator-Info available");
                    }
                }
            }
        }
    }
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}
}
