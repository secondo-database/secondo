package viewer.queryconstruction;

/*
 * This code is based on an example provided by John Vella,
 * a tutorial reader.
 */

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Iterator;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/* ObjectsPane.java requires no other files. */
public class ObjectPane extends MainPane {
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    private QueryconstructionViewer viewer;
    private ListExpr objects;

    public ObjectPane (QueryconstructionViewer viewer) {
        super(viewer);
        this.viewer = viewer;
    }
    
    /** paints a Secondo Object into the ObjectsPane */
    @Override
    public void paintComponent(Graphics g) {        
        int x = 0;
        int y = 0;
        
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.paintComponent( g, x, y, null );
            x++;
        }
    }
    
    //add all relations and objects of berlintest database to the viewer
    public ArrayList<ObjectView> addObjects(ListExpr list){
        // the length must be two and the object element must be an symbol atom with content "inquiry"
        if(list.listLength()==2 && list.first().symbolValue().equals("inquiry")) { 
            objects = list.second().second();
            setPreferredSize(new Dimension (objects.listLength()*120 - 120, 70));
            while (objects.listLength() > 1) {
                ListExpr object = objects.second();
                ObjectView object_view = new ObjectView(object);
                
                addObject(object_view);
                objects = objects.rest();
            }
        }
        this.update();
        return elements;
    }
    
    //    adds an operation or an object to the main panel
    @Override
    public void addObject(ObjectView object){
        elements.add(object);
    }
    
    // updates the operations panel, only allowed operations shoul be viewed black
    public void update() {
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.setUnactive();
            if ((object.getType().equals("rel") || object.getType().equals("trel")) && viewer.getState() < StreamView.TWOSTREAMS) {
                object.setActive();
            }
        }
        this.repaint();
    }

    //double click adds a copy of the selected object to the main panel
    @Override
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
        
        if (arg0.getButton() == 3) {
            int x = 0;
            if (10 < arg0.getY() && arg0.getY() < 80) {
                while (arg0.getX() > (10 + x*120)) { x++; }
                if (arg0.getX() < (10 + x*120)) {
                    if (x <= elements.size()) {
                        if (elements.get(x-1).isActive()) {
                            ObjectView element = elements.get(x-1);
                            new InfoDialog(element);
                        }
                    }
                }
            }
        }
    }
}
