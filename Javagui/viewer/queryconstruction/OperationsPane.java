package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Iterator;
import viewer.QueryconstructionViewer;

/* RelationsPane.java requires no other files. */
public class OperationsPane extends MainPane {
    
    private ArrayList<Operation> operations = new ArrayList<Operation>();
    private QueryconstructionViewer viewer;
    private String result;

    public OperationsPane(QueryconstructionViewer viewer) {
        super(viewer);
        this.viewer = viewer;
        this.setLayout(new GridLayout(0, 1));
        
        String rel[] = {"rel"};
        //operation(name, objects, brackets, parameter, result
        Operation feedrel = new Operation("feed", rel, null, null, "stream");
        addOperation(feedrel);
        Operation countrel = new Operation("count", rel, null, null, "int");
        addOperation(countrel);
        
        String stream[] = {"stream"};
        Operation filter = new Operation("filter", stream, "[]", "bool", "stream");
        addOperation(filter);
        Operation rename = new Operation("rename", stream, "{}", "String", "stream");
        addOperation(rename);
        
        Operation project = new Operation("project", stream, "[]", "arraylist", "stream");
        addOperation(project);
        Operation count = new Operation("count", stream, null, null, "int");
        addOperation(count);
        Operation head = new Operation("head", stream, "[]", "int", "stream");
        addOperation(head);
        Operation tail = new Operation("tail", stream, "[]", "int", "stream");
        addOperation(tail);
        Operation consume = new Operation("consume", stream, null, null, "rel");
        addOperation(consume);
        
        String twoStreams[] = {"stream", "stream"};
        Operation symmjoin = new Operation("symmjoin", twoStreams, "[]", "arraylist", "stream");
        addOperation(symmjoin);
        Operation sortmergejoin = new Operation("sortmergejoin", twoStreams, "[]", "arraylist", "stream");
        addOperation(sortmergejoin);
        Operation mergejoin = new Operation("mergejoin", twoStreams, "[]", "arraylist", "stream");
        addOperation(mergejoin);
        Operation hashjoin = new Operation("hashjoin", twoStreams, "[]", "arraylist", "stream");
        addOperation(hashjoin);
        
        String empty[] = {null};
        Operation units = new Operation("units", empty, "()", "mpoint", "stream upoint");
        addOperation(units);
        
        String compare[] = {"int"};
        Operation equal = new Operation("=", compare, null, "int", "bool");
        addOperation(equal);
        Operation bigger = new Operation(">", compare, null, "int", "bool");
        addOperation(bigger);
        Operation smaller = new Operation("<", compare, null, "int", "bool");
        addOperation(smaller);
//        Operation text = new Operation("text", null, null, "String", "bool");
//        addOperation(text);
        
        String compareString[] = {"String"};
        Operation equalString = new Operation("=", compareString, null, "String", "bool");
        addOperation(equalString);
    }
    
    /** paints a Secondo Object into the relations panel */
    public void paintComponent(Graphics g) {
        this.removeAll();
        String[] viewerParam = viewer.getParameters();
        
        for ( Iterator iter = operations.iterator(); iter.hasNext(); ) {
            Operation op = (Operation)iter.next();
            
            boolean view = true;
            String[] operationParam = op.getObjects();
            
            if ((this.result != null) && (op.getResultType() != null) && (!op.getResultType().equals(result))) {
                view = false;
            }
            
            if ((operationParam[0] == null) && (viewerParam.length == 1)) {
                viewOperation(op);
            }
            
            if (viewerParam.length == (operationParam.length + 1)) {

                if ((viewerParam.length > 0) && (viewerParam[0] == null)) {
                    int i = 1;

                    String viewerStr = "";
                    for (String s : operationParam) {

                        if ((s != null) && (viewerParam.length > i) && (viewerParam[i] != null)) {
                            viewerStr = viewerParam[i].trim();
                            
                            if (!s.equals(viewerStr)) {
                                view = false;
                            }
                        }
                        i++;
                    }
                
                }
                else {
                    view = false;
                }
                if (view) {
                    viewOperation(op); 
                }
            }
            
        }
        this.setPreferredSize(new Dimension(120, 30*this.getComponentCount()));
        this.revalidate();
    }
    
    public void setResult(String result) {
        this.result = result;
    }
    
    public void viewOperation(Operation op) {
        add(op);
    }
    
    public void addOperation(Operation op){
        op.addMouseListener(this);
        operations.add(op);
    }
    
    //updates the operations panel, only allowed operations should visible
    public void update() {
        this.repaint();
    }
    
    @Override
    public void mouseClicked ( MouseEvent arg0 ) {
        //double click adds a copy of the selected operation to the main panel
        if ((arg0.getClickCount () == 1) && (arg0.getComponent() != null)) {
            Operation element = (Operation)arg0.getComponent();
            
            viewer.addOperation(element);
        }
    }
}
