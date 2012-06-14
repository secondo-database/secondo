package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Iterator;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/* ObjectsPane.java requires no other files. */
public class ObjectPane extends MainPane {
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    private QueryconstructionViewer viewer;
    private ListExpr objects;

    public ObjectPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        addMouseListener(this);
        
//        ObjectView Trains = new ObjectView(RELATION, "Trains");
//        addObject(Trains);
    }
    
    /** paints a Secondo Object into the ObjectsPane */
    public void paintComponent(Graphics g) {        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintComponent( g, x, y, null );
            x++;
        }
    }
    
    public void addObjects(ListExpr list){
        // the length must be two and the object element must be an symbol atom with content "inquiry"
        if(list.listLength()==2 && list.first().symbolValue().equals("inquiry")) { 
            objects = list.second().second();
            
            while (!objects.endOfList()) {
                ListExpr object = objects.second();
                ObjectType new_object = new ObjectType(object);
                ObjectView object_view = new ObjectView(new_object.getType(), new_object.getName());
                object_view.setOType(new_object);
                
                addObject(object_view);
                objects = objects.rest();
            }
        }
        this.update();
    }
    
    //    adds an operation or an object to the main panel
    public void addObject(ObjectView object){
        elements.add(object);
    }
    
    // updates the operations panel, only allowed operations shoul be viewed black
    public void update() {
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.setUnactive();
            if (object.getType().equals("rel") || object.getType().equals("trel")) {
                object.setActive();
            }
        }
        this.repaint();
    }

    //double click adds a copy of the selected object to the main panel
    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getClickCount () == 2) {
            int x = 0;
            if (10 < arg0.getY() && arg0.getY() < 80) {
                while (arg0.getX() > (10 + x*120)) { x++; }
                if (arg0.getX() < (10 + x*120)) {
                    if (x <= elements.size()) {
                        if (elements.get(x-1).isActive()) {
                            ObjectView element = elements.get(x-1);
                            ObjectView new_object = new ObjectView(element.getType(), element.getName());
                            new_object.setOType(element.getOType());
                            viewer.addObject(new_object);
                        }
                    }
                }
            }
        }
    }
}
