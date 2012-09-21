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

package viewer.queryconstruction;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import sj.lang.ListExpr;

/**
 * Panel for the allowed operations.
 */
public class OperationsPane extends JComponent implements MouseListener {
    
    //list of all operations
    private ArrayList<Operation> operations = new ArrayList<Operation>();
    
    private MainPane mainPane;
    private Operation rename;
    private String longestName = "";

    /**
     * Generate a new Panel for the Operations.
     * @param viewer main viewer
     */
    public OperationsPane(MainPane main) {
        this.mainPane = main;
        this.setLayout(new GridLayout(0, 1));
    }
    
    /**
     * Add a new operation to the operation list.
     * @param op 
     */
    private void addOperation(Operation op){
        op.addMouseListener(this);
        operations.add(op);
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
        longestName = "";
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
            }
        }
        
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
        //double click adds a copy of the selected operation to the main panel
        if ((arg0.getClickCount () == 1) && (arg0.getComponent() != null)) {
            if (!arg0.getComponent().equals(this)) {
                Operation element = (Operation)arg0.getComponent();
                
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
        }
    }
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}
}
