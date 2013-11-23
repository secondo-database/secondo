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
import java.awt.event.ActionEvent;///
import java.awt.event.ActionListener;///
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import javax.swing.JButton;///
import javax.swing.ToolTipManager;///
import javax.swing.JLabel;///
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer2;///


/**
 * Panel for the allowed operations.
 */
public class OperationsPane23 extends JComponent implements MouseListener {
    
    //list of all operations
    private ArrayList<Operation> operations = new ArrayList<Operation>();
    private ArrayList<Operation> mOperations = new ArrayList<Operation>();
    private ArrayList<Operation> mOperationsTemp = new ArrayList<Operation>();
 
    private QueryconstructionViewer2 mainViewer;///
    private MainPane mainPane;
    private Operation rename;
    private String longestName = "";

    private JButton opSet = new JButton("Set OP-Selection");	/// 
    private JLabel opSetState = new JLabel("All possible Operators");///



    /**
     * Generate a new Panel for the Operations.
     * @param main MainPane
     * @param mainViewer QueryconstructionViewer2
     */
    public OperationsPane23(MainPane main,
			    QueryconstructionViewer2 mainViewer) {
        this.mainPane = main;
	this.mainViewer = mainViewer;
        this.setLayout(new GridLayout(0, 1));

	ToolTipManager.sharedInstance().setDismissDelay(600000); //10 minutes
///
        ActionListener opSetl = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
		new OperationsSelectView();
            }
        };
        opSet.setToolTipText("choose the Operations-Selection");
        opSet.addActionListener(opSetl);

///  


    }
    
/// Neue Methoden

    public void mOperators(ListExpr ops2, ListExpr opsMean){
	addOperations2(ops2, opsMean);	///
    }


///

    /**
     * Add a new operation to the operation list.
     * @param op 
     */
    private void addOperation(Operation op){
        op.addMouseListener(this);
        operations.add(op);
    }

    /**
     * Add a new operation to the operation list.
     * @param op2 
     */
    private void addOperation2(Operation op2){
	op2.addMouseListener(this);
        mOperations.add(op2);
        mOperationsTemp.add(op2);
    }

    
    /**
     * Generate a list of operations of a nested list.
     * @param operators 
     */
    public void addOperations(ListExpr operators) {
        operations.clear();
        while (operators.listLength() > 1) {
            ListExpr objects = operators.second();
            while (objects.listLength() > 0) {
                //processing the nested list
                String opName = objects.first().first().stringValue();
                String opObjects = objects.first().second().textValue();
                String opParams = objects.first().third().textValue();
                String opSignature = objects.first().fourth().textValue();
                String opResult = objects.first().fifth().stringValue();

                String[] possible = opObjects.split("\\|");
                for (String objectStrings: possible) {
                    Operation op = new Operation(opName, 
                            objectStrings.split(";"), opSignature, 
                            opParams.split(";"), opResult);
                    addOperation(op);
                    
                    //save the rename operator as variable
                    if (opName.equals("rename")){
                        rename = op;
                    }
                }
                
                objects = objects.rest();
            }
            operators = operators.rest();
        }
        this.repaint();
    }

    /**
     * Generate a list of operations of a nested list. Zum Test ///
     * @param mOperators 
     */
    public void addOperations2(ListExpr mOperators, ListExpr opsMean) {
        mOperations.clear();
	mOperationsTemp.clear();
        while (mOperators.listLength() > 1) {
            ListExpr objects2 = mOperators.second();
            ListExpr mObjects2 = opsMean.second();
            while (objects2.listLength() > 0) {
                //processing the nested list

                String opName2 = objects2.first().first().stringValue();
                String opObjects2 = " ";
                String opParams2 = " ";
                String opSignature2 =  " ";
                String opResult2 =  " ";
                
                String[] possible2 = opObjects2.split("\\|");
                for (String objectStrings2: possible2) {
                    Operation op2 = new Operation(opName2, 
                            objectStrings2.split(";"), opSignature2, 
                            opParams2.split(";"), opResult2);
		    op2.setInactive();
		    addOperation2(op2);

		    String opMean = mObjects2.first().first().textValue();
		    String note = "...more info with rightclick";
		    if (opMean.length() > 80) {
			op2.setToolTipText("<html>" + opMean.substring(0, 80)
			    + "<font color=\"#FF55FF\">" + note + "</font></html>");
		    }
		    else {
			op2.setToolTipText("<html>" + opMean
			    + "<font color=\"#FF55FF\">  " + note + "</font></html>");
		    }
		//System.out.println(op2.getToolTipText().length());
		//System.out.println(op2.getOperationName().length());
		}

/*
                    //save the rename operator as variable
            //        if (opName.equals("rename")){
            //            rename = op;
            //        }
            //    }

*/

                objects2 = objects2.rest();
                mObjects2 = mObjects2.rest();
            }
            mOperators = mOperators.rest();
            opsMean = opsMean.rest();
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
	this.add(opSet);///Button for OP-Selection
	this.add(opSetState);///State of OPSelection

        longestName = "";
	boolean contains2 = false; ///

        // objects, that are given by the actual query
        String[] viewerParam = mainPane.getParameters();
        
        for ( Iterator iter = operations.iterator(); iter.hasNext(); ) {
            Operation op = (Operation)iter.next();
            // objects, tha are needed by the operation
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
                    viewOperation(op);
                }
		contains2 = contains; 
            }

/// add mOperations
	    if (contains2) {
		for ( Iterator iter2 = mOperations.iterator(); iter2.hasNext(); ) {
		    Operation op2 = (Operation)iter2.next();
		    if (op2.getOperationName().equals(op.getOperationName())) {
		    //System.out.println(op2.getOperationName());
		    mOperationsTemp.remove(op2);
		    }
		}
	    }	    
	} // end for

	for ( Iterator iter3 = mOperationsTemp.iterator(); iter3.hasNext(); ) {
	    Operation opRest = (Operation)iter3.next();
	    viewOperation2(opRest);
	    //System.out.println(opRest.getOperationName());
	}

///
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

    /**
     * Add an operation to the panel.
     * @param op2 
     */
    private void viewOperation2(Operation op2) {
        this.add(op2);

        // fit the width of the panel to the longest operatorname
        if (longestName.length() < op2.getOperationName().length()) {	///
            longestName = op2.getOperationName();			///
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
		    String opName = element.getOperationName();
		    String opObject = mainViewer.getOpObject();//zum Test
		    ListExpr opInfo = mainViewer.getOperatorInfo(opName, opObject);
		    new OperationsInfoDialog(120, 40, opInfo);
		}
            }
        }
    }
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}
}
