package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

public class OperationsPane extends JComponent implements MouseListener {
    
    //list of all operations
    private ArrayList<Operation> operations = new ArrayList<Operation>();
    private QueryconstructionViewer viewer;
    private Operation rename;
    private String longestName = "";

    /**
     * Generate a new Panel for the Operations.
     * @param viewer main viewer
     */
    public OperationsPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
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
                
                Operation op = new Operation(opName, opObjects.split(";"), opSignature, opParams.split(";"), opResult);
                addOperation(op);
                
                //save the rename operator as variable
                if (opName.equals("rename")){
                    rename = op;
                }
                objects = objects.rest();
            }
            operators = operators.rest();
        }
        update();
    }
    
    /**
     * Check if the type is in the array of usable types.
     * @param type
     * @param tArray array of types
     * @return type is part of the array
     */
    private boolean typeInArray(String type, String[] tArray){
        for (String s: tArray) {
            if (s.equals(type))
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
     * Repaint the operations panel.
     */
    public void update() {
        this.repaint();
    }
    
    /**
     * Paint all operations into the panel, 
     * only allowed operations should visible.
     * @param g 
     */
    public void paintComponent(Graphics g) {
        this.removeAll();
        longestName = "";
        String[] viewerParam = viewer.getParameters();
        
        for ( Iterator iter = operations.iterator(); iter.hasNext(); ) {
            Operation op = (Operation)iter.next();
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
             * If the count of active objects equals the count of objects, the operation needs, 
             * the viewer checks if the types are the same.
             */
            if (viewerCount == operationObjects.length) {
                int[][] typesIn = new int[viewerCount][viewerCount];
                
                int index = 0;
                for (String viewerStr : viewerParam) {
                    typesIn[index] = this.typeInArray2(viewerStr.trim(), operationObjects);
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
                if (contains) {
                    viewOperation(op);
                }
            }
        }
        
        int width = g.getFontMetrics().stringWidth(longestName);
        this.setPreferredSize(new Dimension((width + 40), (30 * this.getComponentCount())));
        
        this.revalidate();
    }
    
    /**
     * Add an operation to the panel.
     * @param op 
     */
    private void viewOperation(Operation op) {
        add(op);
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
                if ((element.getObjects().length == 2) && (element.getObjects()[0].equals("stream")) && (element.getObjects()[1].equals("stream")) && (element.getParameter().length > 0)) {
                    if (!viewer.checkAttributes()) {
                        viewer.addOperation(rename.copy());
                        return;
                    }
                }
                viewer.addOperation(element.copy());
            }
        }
    }
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}
}
