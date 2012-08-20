package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import viewer.QueryconstructionViewer;

/* RelationsPane.java requires no other files. */
public class OperationsPane extends JComponent implements MouseListener {
    
    private ArrayList<Operation> operations = new ArrayList<Operation>();
    private QueryconstructionViewer viewer;
    
    private Operation rename;

    public OperationsPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        this.setLayout(new GridLayout(0, 1));
        
        
        String rel[] = {"rel"};
        //operation(name, objects, brackets, parameter, result
        Operation feedrel = new Operation("feed", rel, "#", null, "stream");
        addOperation(feedrel);
        Operation countrel = new Operation("count", rel, "#", null, "int");
        addOperation(countrel);
        
        String stream[] = {"stream"};
        Operation filter = new Operation("filter", stream, "#[p]", "bool", "stream");
        addOperation(filter);
        rename = new Operation("rename", stream, "{p}", "String", "stream");
        addOperation(rename);
        Operation extend = new Operation("extend", stream, "#[p: p]", "String;new", "stream");
        addOperation(extend);
        Operation project = new Operation("project", stream, "#[p]", "attrlist", "stream");
        addOperation(project);
        Operation head = new Operation("head", stream, "#[p]", "int", "stream");
        addOperation(head);
        Operation tail = new Operation("tail", stream, "#[p]", "int", "stream");
        addOperation(tail);
        
        Operation count = new Operation("count", stream, "#", null, "int");
        addOperation(count);
        Operation consume = new Operation("consume", stream, "#", null, "rel");
        addOperation(consume);
        
        String twoStreams[] = {"stream", "stream"};
        Operation symmjoin = new Operation("symmjoin", twoStreams, "#[p]", "bool", "stream");
        addOperation(symmjoin);
        Operation sortmergejoin = new Operation("sortmergejoin", twoStreams, "#[p]", "attr,attr", "stream");
        addOperation(sortmergejoin);
        Operation mergejoin = new Operation("mergejoin", twoStreams, "#[p]", "attr,attr", "stream");
        addOperation(mergejoin);
        Operation hashjoin = new Operation("hashjoin", twoStreams, "#[p]", "attr,attr", "stream");
        addOperation(hashjoin);
        
        String empty[] = {null};
        Operation units = new Operation("units", empty, "#(p)", "mpoint", "stream upoint");
        addOperation(units);
        Operation distance = new Operation("distance", empty, "#(p, p)", "spatial;spatial", "int");
        addOperation(distance);
        Operation string = new Operation("new string", empty, "p", "String", "bool");
        addOperation(string);
        
        String compare[] = {"int"};
        Operation bigger = new Operation(">", compare, "# p", "int", "bool");
        addOperation(bigger);
        Operation smaller = new Operation("<", compare, "# p", "int", "bool");
        addOperation(smaller);
        
        String compareTyp[] = {"typ"};
        Operation equalString = new Operation("=", compareTyp, "# p", "typ", "bool");
        addOperation(equalString);
        
        String mpoint[] = {"mpoint,mregion"};
        Operation passes = new Operation("passes", mpoint, "# p", "point", "bool");
        addOperation(passes);
    }
    
    /** paints a Secondo Object into the relations panel */
    public void paintComponent(Graphics g) {
        this.removeAll();
        String[] viewerParam = viewer.getParameters();
        
        for ( Iterator iter = operations.iterator(); iter.hasNext(); ) {
            Operation op = (Operation)iter.next();
            boolean view = true;
            String[] operationParam = op.getObjects();
            
            //TODO
            int viewerCount = viewerParam.length;
            
            if (operationParam[0] == null) {
                viewOperation(op);
            }
            int i = 0;
            if ((viewerParam.length > 1) && (viewerParam[0] == null)) {
                viewerCount--;
                i++;
            }
            
            if (viewerCount == operationParam.length) {

                String viewerStr = "";
                for (String s : operationParam) {

                    if ((s != null) && (viewerCount > i) && (viewerParam[i] != null)) {

                        viewerStr = viewerParam[i].trim();
                        //some operations can be used on different object types
                        String[] oPs = s.split(",");
                        boolean oneright = false;
                        for (String oP: oPs) {
                            if (s.equals("typ")) {
                                op.setParameter(viewerStr);
                                oP = viewerStr;
                            }
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
        this.setPreferredSize(new Dimension(120, (30 * this.getComponentCount())));
        
        this.revalidate();
    }
    
    public void viewOperation(Operation op) {
        add(op);
    }
    
    private void addOperation(Operation op){
        op.addMouseListener(this);
        operations.add(op);
    }
    
    //updates the operations panel, only allowed operations should visible
    public void update() {
        this.repaint();
    }
    
    public void mouseClicked ( MouseEvent arg0 ) {
        //double click adds a copy of the selected operation to the main panel
        if ((arg0.getClickCount () == 1) && (arg0.getComponent() != null)) {
            if (!arg0.getComponent().equals(this)) {
                Operation element = (Operation)arg0.getComponent();
                
                //check if objects have to be renamed
                if (element.countObjects() > 1) {
                    if (!viewer.checkAttributes()) {
                        //dialog.setMessage("Two attributes have the same name. Please rename the object "+activeStream.getName()+".");
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
