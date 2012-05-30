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
        ObjectView feed = new ObjectView(OPERATION, "feed");
        addObject(feed);
        ObjectView filter = new ObjectView(OPERATION, "project");
        addObject(filter);
        ObjectView count = new ObjectView(OPERATION, "count");
        addObject(count);
        ObjectView head = new ObjectView(OPERATION, "head[1]");
        addObject(head);
        ObjectView tail = new ObjectView(OPERATION, "tail[4]");
        addObject(tail);
        ObjectView consume = new ObjectView(OPERATION, "consume");
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
            if (lastComponent.getName().equals("Trains")) {
                if ("feed".equals(object.getName()) || "count".equals(object.getName())) {
                    object.setActive();
                }
            }
            if (lastComponent.getName() == "feed") {
                if (object.getName() == "head[1]" || object.getName() == "tail[4]" || object.getName() == "project") {
                    object.setActive();
                }
            }
            if (lastComponent.getName() == "feed" || lastComponent.getName() == "head[1]" || lastComponent.getName() == "tail[4]") {
                if (object.getName() == "consume") {
                    object.setActive();
                }
            }
        }
        this.repaint();
    }

    //double click adds the selected operation to the main panel
    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getClickCount () == 2) {
            int y = 0;
            while (arg0.getY() > (y*30)) { y++; }
            if (y <= elements.size()) {
                if (elements.get(y-1).isActive())
                    viewer.addObject(elements.get(y-1));
            }
        }
    }
}
