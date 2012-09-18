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
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JComponent;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 * Main panel for the query objects and operators.
 */
public class MainPane extends JComponent implements MouseListener {
    
    private StreamView activeStream = new StreamView("", "", 0, 0);
    private ArrayList<StreamView> fullStream = new ArrayList<StreamView>();
    private ArrayList<StreamView> activeStreams = new ArrayList<StreamView>();
    
    private QueryconstructionViewer viewer;
    private OperationsDialog dialog;
    private InfoDialog infoDialog;
    
    private ObjectView lastObject;
    private int lastY = 0;
    private int lastX = 0;
    
    private String result;
    
    public MainPane(QueryconstructionViewer viewer) {
        this.viewer = viewer;
        this.addMouseListener(this);
    }
    
    /**
     * Add an object to the main panel in a new row.
     * @param object 
     */
    public void addObject(ObjectView object){
        activeStream.change();
        //generate a new stream for the object
        activeStream = new StreamView(object.getLabel(), "", 0, lastY);
        fullStream.add(activeStream);
        lastY++;
        
        activeStream.addObject(object);
        lastObject = object;
        object.addMouseListener(this);
        
        viewer.update();
    }
    
    /**
     * Add an operation to the main panel.
     * @param operation 
     */
    public void addOperation(Operation operation) {
        
        if (!operation.getParameter()[0].equals("")) {
            //generate the communication dialog
            this.dialog = new OperationsDialog(this, this.viewer, operation);
            
            String dot = ".";
            if (operation.getObjects().length > 0) {
                
                /* treat attributes for each active stream */
                for ( Iterator iter = activeStreams.iterator(); 
                        iter.hasNext(); ) {
                    StreamView stream = (StreamView)iter.next();
                    
                    if (stream.getAttributes() != null) {
                        //add attributes for nested queries
                        dialog.addAttributes(stream.getAttrObjects(dot));
                        for (String param: operation.getParameter()) {
                            //add attributes for radio buttons
                            if (param.equals("attr,attr")) {
                                dialog.addRadiobuttons(stream.getName(), 
                                        stream.getAttributes());
                            }
                            //add attributes for checkboxes
                            if (param.startsWith("attrlist")) {
                                if (param.endsWith("dir"))
                                    dialog.addCheckboxes(stream.getName(), 
                                            stream.getAttributes(), 
                                            new String[]{"asc", "desc"});
                                else if (param.contains("sort"))
                                    dialog.addCheckboxes(stream.getName(), 
                                            stream.getAttributesSorted(), 
                                            null);
                                else
                                    dialog.addCheckboxes(stream.getName(), 
                                            stream.getAttributes(), 
                                            null);
                            }
                        }
                    }
                    //add the group object if needed
                    if (operation.getOperationName().startsWith("group")) {
                        dialog.addTuple(stream
                                .getObjects().get(0).copy("group"));
                    }
                    dot += ".";
                }
            }
            //set the dialog window visible
            dialog.activate();
        }
        
        if ((operation.countObjects() > 1) || 
                ((operation.countObjects() == 1) && 
                operation.getSignature().contains("o") &&
                !operation.getSignature().startsWith("o"))) {
            
            lastX = getLastX();
            
            //generate a new stream
            activeStream = new StreamView(operation.getOperationName(), 
                    operation.getSignature(), lastX, activeStream.getY());
            
            /* turn the active streams as input streams 
             * of the new operation stream */
            for ( Iterator iter = activeStreams.iterator();
                    iter.hasNext(); ) {
                StreamView stream = (StreamView)iter.next();
                activeStream.addInputStream(stream);
                stream.setNext(activeStream);
            }
            fullStream.add(activeStream); 
        }
        
        lastObject = operation.getView();
        
        /* if the operation has no input objects, it has to be turned into
         * an object */
        if (operation.getObjects()[0].equals("")) {
            ObjectView new_object = new ObjectView(
                    operation.getOperationName(), 
                    operation.getResultType());
            addObject(new_object);
            new_object.setSignature(operation.getSignature());
        }
        else {
            activeStream.addObject(operation.getView());
        }
        
        viewer.update();
    }
    
    /**
     * Add streams of parameters to the active stream.
     * @param stream
     * @param label new label of the last component
     */
    protected void addParamStream(StreamView stream, String label){
        lastObject.addParamStream(stream);
        if (label != null)
            lastObject.setLabel(label);
        
        activeStream.addParamStream(stream);
    }
    
    /**
     * Add the name of a sorted attribute to the active stream.
     * @param attr 
     */
    protected void addSortedAttribute(String attr) {
        activeStream.addSortedAttribute(attr);
    }
    
    /**
     * Search for two attributes with the same name.
     * @return true, if no double attribute names exist
     */
    public boolean checkAttributes() {
        String[][] attributes = new String[activeStreams.size()][];
        int i = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            attributes[i] = stream.getAttributes();
            if (attributes[i] != null)
                i++;
        }
        int j = 0;
        while (j < i-1) {
            for (String attr1 : attributes[j]) {
                for (String attr2 : attributes[j+1]) {
                    if (attr1.equals(attr2)) {
                        return false;
                    }
                }
            }
            j++;
        }
        return true;
    }
    
    /**
     * Get the quantity of attributes of the active stream.
     * @return 
     */
    protected int getAttributesCount() {
        if (activeStream.getAttributes() != null)
            return activeStream.getAttributes().length;
        else
            return 0;
    }
    
    /**
     * Get the info for the InfoDialog and show it.
     */
    private void getInfo() {
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            String state = stream.getState();
            String name = stream.getName();
            if (state != null) {
                infoDialog.addInfo(name, state);
                infoDialog.view();
            }
        }
    }
    
    /**
     * Get the last position of an object in x dimension.
     * @return 
     */
    private int getLastX(){
        int length = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (stream.getX() + stream.getLength() > length) {
                length = stream.getX() + stream.getLength();
            }
        }
        return length;
    }
    
    /**
     * Get the types of the active streams.
     * @return 
     */
    public String[] getParameters() {
        String parameters[] = new String[activeStreams.size()];
        int i = 0;
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            parameters[i] = stream.getType();
            
            if (parameters[i] == null && stream.getObjects().size() > 0) {
                parameters[i] = stream.getObjects().get(0).getType();
            }
            i++;
        }
        return parameters;
    }
    
    /**
     * Get the stream of the selected position.
     * @param x
     * @param y
     * @return 
     */
    private StreamView getStream(int x , int y){
        StreamView returnStream = null;
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (stream.getY() == y) {
                if ((stream.getX()-1 < x ) &&
                        (x < stream.getX() + stream.getLength() + 1))
                    returnStream = stream;
            }
        }
        return returnStream;
    }
    
    /**
     * Get the names of all objects.
     * @return 
     */
    protected String getStrings(){
        String query = "";
        
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            query += stream.getString()+" ";
        }
        
        return query;
    }
    
    /**
     * Get the active Query.
     * @return query string
     */
    public String getStringsQuery(){
        return "query " + getStrings();
    }
    
    /**
     * Get the result type of the active query.
     * @return 
     */
    protected String getType(){
        return result;
    }
    
    /**
     * Get the names of all objects, attributes turned to constants.
     * @return 
     */
    private String getTypeString(){
        String query = "query ";
        
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            query += stream.getTypeString()+" ";
        }
        
        return query;
    }
    
    /**
     * Remove the active object in the query.
     */
    public void removeActiveComponent(){
        if (activeStream.getObjects().size() > 0) {
            activeStream.getObjects().remove(activeStream.getLastComponent());
            lastObject = activeStream.getLastComponent();
        }
        viewer.update();
    }
    
    public void removeAllObjects(){
        this.removeAll();
        
        lastObject = null;
        lastX = 0;
        lastY = 0;
        
        activeStream = new StreamView("", "", 0, 0);
        fullStream = new ArrayList<StreamView>();
        activeStreams = new ArrayList<StreamView>();
    }
    
    /**
     * Delete the last added object of the query.
     */
    public void removeLastObject(){
        if (fullStream.size() > 0) {
            StreamView lastStream = fullStream.get(fullStream.size()-1);
            lastStream.getObjects().remove(lastStream.getLastComponent());
            if (lastStream.getLength() < 1) {
                if (lastY > 0) {
                    lastY--;
                }
                
                /* remove the stream and set the input streams active */
                lastStream.remove();
                fullStream.remove(lastStream);
                
                /* set the new active stream */
                if (fullStream.size() > 0) {
                    activeStream = fullStream.get(fullStream.size()-1);
                }
                else {
                    activeStream = new StreamView("new", "", 0, 0);
                }
            }
        }
        else {
            viewer.removeAll();
        }
    }
    
    /**
     * Find all active streams.
     */
    private void setActiveStreams(){
        activeStreams = (ArrayList<StreamView>)fullStream.clone();
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (!stream.isActive()) {
                activeStreams.remove(stream);
            }
            else {
                activeStream = stream;
            }
        }
    }
    
//    private void setType(ListExpr obj) {
//        if (obj != null) {
//            result = obj.second().textValue();
//            if (result.equals("undefined"))
//                result = obj.first().textValue();
//        }
//    }
    
    /**
     * Update the panel and the stream information.
     */
    public String update() {
        setActiveStreams();
        ListExpr type = updateStream(activeStream);
        if (activeStreams.size() > 1)
            type = viewer.getType(this.getTypeString());
        
        /* Update the result type. */
        if ((type != null) && (type.second().textValue() != null)) {
            result = type.second().textValue();
            if (result.equals("undefined"))
                    result = type.first().textValue();
            this.setToolTipText(result);
        }
        else {
            this.setToolTipText(getStringsQuery());
            result = getStringsQuery();
        }
        
        this.setPreferredSize(new Dimension(getLastX()*120, this.lastY * 80));
        this.repaint();
        this.revalidate();
        
        return result;
    }
    
    /**
     * Update the stream result types.
     * @param stream
     * @return 
     */
    private ListExpr updateStream(StreamView stream) {
        ListExpr obj = viewer.getType("query " + stream.getTypeString());

        if (obj != null) {
            String streamResult = obj.second().textValue();
            if (streamResult.equals("undefined"))
                streamResult = obj.first().textValue();
            if (streamResult != null)
                stream.setState(streamResult);
        }
        return obj;
    }
    
    public void paintComponent(Graphics g) {
        //paint all streams
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            stream.paintComponent(g);
        }
    }
    
    /**
     * Handle mouse event.
     * @param e mouse event
     */
    public void mouseClicked ( MouseEvent e ) {
        
        //get the position of the click
        int y = 0;
        while (e.getY() > (10 + (y+1)*80)) { y++; }
        int x = 0;
        while (e.getX() > (10 + x*120)) { x++; }
        StreamView stream = getStream(x,y);
        
        /* right click on an object shows more information about the query */
        if (e.getButton() == 3) {

          // java 1.4 compatible code
            int dx = 0;
            int dy = 0;
            Object src = e.getSource();
            if(src!=null){
               if(src instanceof java.awt.Component){
                  java.awt.Point p = ((java.awt.Component) src).getLocationOnScreen();
                  if(p!=null){
                     dx = p.x;
                     dy = p.y;
                  }
               }
            }
            
            infoDialog = new InfoDialog(dx + e.getX(), dy + e.getY());
            infoDialog.setTitle(activeStream.getName());
            if (stream != null)
                infoDialog.addInfo("Query: "+stream.getName(), stream.getString());
            getInfo();
        }
        
        //double click sets the last object of the selected stream active
        if ((e.getClickCount () == 1) && (fullStream.size() > y) && (e.getButton() == 1)) {
            
            if (stream != null) {
                stream.change();
                viewer.update();
            }        
        }
    }
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e) {}
}
