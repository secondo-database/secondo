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
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    private QueryconstructionViewer viewer;

    public OperationsPane(QueryconstructionViewer viewer) {
        ObjectView feed = new ObjectView(ObjectType.OPERATION, "feed");
        addObject(feed);
        ObjectView filter = new ObjectView(ObjectType.OPERATION, "project");
        addObject(filter);
        ObjectView count = new ObjectView(ObjectType.OPERATION, "count");
        addObject(count);
        ObjectView head = new ObjectView(ObjectType.OPERATION, "head");
        addObject(head);
        ObjectView tail = new ObjectView(ObjectType.OPERATION, "tail");
        addObject(tail);
        ObjectView consume = new ObjectView(ObjectType.OPERATION, "consume");
        addObject(consume);
        
        this.viewer = viewer;
        this.addMouseListener(this);
    }
    
    /** paints a Secondo Object into the RelationsPane */
    public void paintComponent(Graphics g) {
        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintTable( g, x, y, null );
            y++;
        }
        
    }
    
    //    adds an operation or an object to the operations panel
    public void addObject(ObjectView object){
        elements.add(object);
    }
    
    // updates the operations panel, only allowed operations shoul be viewed black
    public void update() {
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.setUnactive();
            if (lastComponent.getType().equals(ObjectType.RELATION) || lastComponent.getType().equals(ObjectType.TRELATION)) {
                if ("feed".equals(object.getName()) || "count".equals(object.getName())) {
                    object.setActive();
                }
            }
            if (lastComponent.getName().equals("feed")) {
                if (object.getName().startsWith("head") || object.getName().startsWith("tail") || object.getName().startsWith("project")) {
                    object.setActive();
                }
            }
            if (object.getName().equals("consume") && (viewer.getState() == MainPane.STREAM || viewer.getState() == MainPane.TWOSTREAMS)) {
                object.setActive();
            }
        }
        this.repaint();
    }

    //double click adds a copy of the selected operation to the main panel
    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getClickCount () == 2) {
            int y = 0;
            while (arg0.getY() > (y*30)) { y++; }
            if (y <= elements.size()) {
                if (elements.get(y-1).isActive())
                    viewer.addObject(new ObjectView(elements.get(y-1).getType(), elements.get(y-1).getName()));
            }
        }
    }
}
