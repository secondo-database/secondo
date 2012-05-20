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
        this.viewer = viewer;
        this.addMouseListener(this);
    }
    
    /** paints a Secondo Object into the RelationsPane */
    public void paintComponent(Graphics g) {
        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintComponent( g, x, y, null );
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
            if (lastComponent.getName() == "Trains") {
                if (object.getName() == "feed" || object.getName() == "count" || object.getName() == "filter") {
                    object.setActive();
                }
            }
            if (lastComponent.getName() == "feed") {
                if (object.getName() == "head[1]" || object.getName() == "tail[4]") {
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
            if (10 < arg0.getX() && arg0.getX() < 130) {
                while (arg0.getY() > (10 + y*70)) { y++; }
                if (arg0.getX() < (10 + y*70)) {
                    if (y <= elements.size()) {
                        if (elements.get(y-1).isActive())
                            viewer.addObject(elements.get(y-1));
                    }
                }
            }
            
        }
    }
}
