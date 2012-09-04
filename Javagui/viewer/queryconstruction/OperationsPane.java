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
    
    private ArrayList<Operation> operations = new ArrayList<Operation>();
    private QueryconstructionViewer viewer;
    
    private Operation rename;

    public OperationsPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        this.setLayout(new GridLayout(0, 1));
    }
    
    /**
     * Paint all usable operations into the panel.
     * @param g 
     */
    public void paintComponent(Graphics g) {
        this.removeAll();
        String[] viewerParam = viewer.getParameters();
        
        for ( Iterator iter = operations.iterator(); iter.hasNext(); ) {
            Operation op = (Operation)iter.next();
            boolean view = true;
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

                for (String opObject : operationObjects) {

                    if ((!opObject.equals("")) && (viewerCount > i) && (viewerParam[i] != null)) {

                        String viewerStr = viewerParam[i].trim();
                        //some operations can be used on different object types
                        String[] oPs = opObject.split(",");
                        boolean oneright = false;
                        if (opObject.equals("typ")) {
                            operationObjects[i] = viewerParam[i];
                            operationObjects[i+1] = viewerParam[i];
                        }
                        for (String oP: oPs) {
                            if (oP.equals(viewerStr)) {
                                oneright = true;
                            }
                        }
                        view = oneright;
                    }
                    i++;
                }
                if (view) {
                    viewOperation(op);
                }
            }
            
        }
        this.setPreferredSize(new Dimension(110, (30 * this.getComponentCount())));
        
        this.revalidate();
    }
    
    private void viewOperation(Operation op) {
        add(op);
    }
    
    private void addOperation(Operation op){
        op.addMouseListener(this);
        operations.add(op);
    }
    
    public void addOperations(ListExpr operators) {
        operations.clear();
        while (operators.listLength() > 1) {
            ListExpr objects = operators.second();
            while (objects.listLength() > 0) {
                String opName = objects.first().first().stringValue();
                String opObjects = objects.first().second().textValue();
                String opParams = objects.first().third().textValue();
                String opSignature = objects.first().fourth().textValue();
                String opResult = objects.first().fifth().stringValue();
                
                Operation op = new Operation(opName, opObjects.split(";"), opSignature, opParams.split(";"), opResult);
                addOperation(op);
                
                if (opName.equals("rename")){
                    rename = new Operation(opName, opObjects.split(";"), opSignature, opParams.split(";"), opResult);
                }
                objects = objects.rest();
            }
            operators = operators.rest();
        }
        update();
    }
    
    /**
     * updates the operations panel, only allowed operations should visible
     */
    public void update() {
        this.repaint();
    }
    
    public void mouseClicked ( MouseEvent arg0 ) {
        //double click adds a copy of the selected operation to the main panel
        if ((arg0.getClickCount () == 1) && (arg0.getComponent() != null)) {
            if (!arg0.getComponent().equals(this)) {
                Operation element = (Operation)arg0.getComponent();
                
                //check if objects have to be renamed
                if (element.getParameter()[0].equals("attr,attr")) {
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
