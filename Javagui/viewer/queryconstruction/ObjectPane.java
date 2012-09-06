//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.queryconstruction;

import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import javax.swing.JTextField;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/* ObjectsPane.java requires no other files. */
public class ObjectPane  extends JComponent implements MouseListener {
    
    private ArrayList<ObjectView> elements = new ArrayList<ObjectView>();
    private ArrayList<ObjectView> attributeElements = new ArrayList<ObjectView>();
    private QueryconstructionViewer viewer;
    private ListExpr objects;
    private JTextField textfield = new JTextField();
    private InfoDialog dialog;

    public ObjectPane (QueryconstructionViewer viewer) {
        this.viewer = viewer;
        this.setLayout(new GridLayout(1, 0));
        
        ActionListener textL = new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                String objName = textfield.getText();
                updateObjects(objName);
            }
        };
        
        textfield.addActionListener(textL);
    }
    
    //add all relations and objects of berlintest database to the viewer
    public void addObjects(ListExpr list){
        this.removeAll();
        elements.clear();
        this.add(textfield);
        
        // the length must be two and the object element must be a symbol atom with content "inquiry"
        if(list.listLength()==2 && list.first().symbolValue().equals("inquiry")) { 
            objects = list.second().second();
            
            while (objects.listLength() > 1) {
                ListExpr object = objects.second();
                
                ObjectView objectView = new ObjectView(object);
                if (!objectView.getObjectName().startsWith("SEC")) {
                    addObject(objectView);
                }
                objects = objects.rest();
            }
        }
        this.update();
    }
    
    /**
     * Get a list of all objects in the panel.
     * @return 
     */
    public ArrayList<ObjectView> getObjects(){
        return this.elements;
    }
    
    /**
     * adds the object to the object panel if it does not exist already
     * @param object 
     */
    protected void addObject(ObjectView object){
        object.addMouseListener(this);
        object.setActive(true);
        add(object);
        this.elements.add(object);
    }
    
    /**
     * adds the object to the object panel if it does not exist already
     * @param object 
     */
    protected void addAttributeObject(ObjectView object){
        object.addMouseListener(this);
        object.setActive(true);
        add(object);
        this.attributeElements.add(object);
    }
    
    protected void clear() {
        this.attributeElements.clear();
    }
    
    // updates the panel, only active objects are shown
    public void update() {
        setPreferredSize(new Dimension (this.getComponentCount()*120, 70));
        this.revalidate();
    }
    
    /**
     * add all existing objects to the Component
     */
    protected void showAllObjects() {
        this.removeAll();
        this.add(textfield);
        for ( Iterator iter = attributeElements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            this.add(object);
            object.setActive(true);
        }
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            this.add(object);
            object.setActive(true);
        }
        update();
    }
    
    /**
     * updates the panel, only active objects that fit to the input of textfield are visible
     * @param type input of the textfield
     */
    private void updateObjects(String type) {
        String[] types = type.split(",");
        for ( Iterator iter = elements.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            object.setActive(false);
            for (String s: types) {
                if (s.equals("bool"))
                    return;
                if (object.getObjectName().toLowerCase().startsWith(s.toLowerCase()) || object.getType().equals(s)) {
                    object.setActive(true);
                }
            }
            if (object.isActive()) {
                this.add(object);
            }
            else {
                this.remove(object);
            }
        }
        this.update();
    }

    public void mouseClicked ( MouseEvent arg0 ) {
        if (arg0.getComponent() instanceof ObjectView) {
            ObjectView element = (ObjectView)arg0.getComponent();
            
            if (arg0.getButton() == 3) {
                // Java 1.4 compatible 
                int dx = 0;
                int dy = 0;
                Object src = arg0.getSource();
                if(src!=null){
                   if(src instanceof java.awt.Component){
                      java.awt.Point p = ((java.awt.Component) src).getLocationOnScreen();
                      if(p!=null){
                         dx = p.x;
                         dy = p.y;
                      }
                   }
                }
                dialog = new InfoDialog(dx + arg0.getX(), dy + arg0.getY());
                
                String elementCount = viewer.getCount(element.getObjectName()).trim();
                String elementName = element.getObjectName();
                if (!elementCount.equals("0"))
                    elementName += " ("+elementCount+")";
                /* generating the info dialog of the clicked object */
                if (element.getOType() == null) {
                    dialog.viewInfo(elementName, element.getType());
                }
                else {
                    dialog.viewInfo(elementName, element.getOType().getViewString());
                }
                dialog.view();
            }
            else {
                //a copy of the object is added to the main panel
                ObjectView new_object = new ObjectView(element.getObjectName(), element.getType());
                new_object.setLabel(element.getLabel());
                new_object.setOType(element.getOType());
                viewer.addObject(new_object);
            }
        }
    }
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}

}
