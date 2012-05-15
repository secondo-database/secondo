package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

/* RelationsPane.java requires no other files. */
public class OperationsPane extends JComponent {
    
    private Collection<ObjectComponent> elements = new ArrayList<ObjectComponent>();

    public OperationsPane() {
        
    }
    
    /** paints a Secondo Object into the RelationsPane */
    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectComponent object = (ObjectComponent)iter.next();
            object.paintComponent( g, x, y );
            y++;
        }
        
    }
    
    //    adds an operation or an object to the operations panel
    public void addObject(ObjectComponent object){
        elements.add(object);
    }

    
}
