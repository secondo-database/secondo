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

/* ObjectsPane.java requires no other files. */
public class ObjectsPane extends MainPane {
    
    private ArrayList<ObjectComponent> elements = new ArrayList<ObjectComponent>();
    private QueryconstructionViewer viewer;

    public ObjectsPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        this.addMouseListener(this);
    }
    
    /** paints a Secondo Object into the ObjectsPane */
    public void paintComponent(Graphics g) {        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectComponent object = (ObjectComponent)iter.next();
            object.paintComponent( g, x, y );
            x++;
        }
    }
    
    //    adds an operation or an object to the main panel
    public void addObject(ObjectComponent object){
        elements.add(object);
    }

    //double click adds the selected object to the main panel
    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getClickCount () == 2) {
            int x = 0;
            if (10 < arg0.getY() && arg0.getY() < 80) {
                while (arg0.getX() > (10 + x*120)) { x++; }
                if (arg0.getX() < (10 + x*120)) {
                    if (x <= elements.size()) {
                        viewer.addObject(elements.get(x-1));
                    }
                }
            }
        }
    }
}
