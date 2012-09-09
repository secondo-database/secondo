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
 *
 * @author lrentergent
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
    
    //adds an operation or an object to the main panel
    public void addObject(ObjectView object){
        activeStream = new StreamView(object.getLabel(), "", 0, lastY);
        fullStream.add(activeStream);
        lastY++;
        activeStream.addObject(object);
        lastObject = object;
        object.addMouseListener(this);
    }
    
    /**
     * add the operation to the query
     * @param operation 
     */
    public void addOperation(Operation operation) {
        
        if (!operation.getParameter()[0].equals("")) {
            this.dialog = new OperationsDialog(this, this.viewer, operation);
            String dot = ".";
            if (operation.getObjects().length > 0) {
                for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
                    StreamView stream = (StreamView)iter.next();
                    
                    if (stream.getAttributes() != null) {
                        dialog.addAttributes(stream.getAttrObjects(dot));
                        for (String param: operation.getParameter()) {
                            if (param.equals("attr,attr")) {
                                dialog.addRadiobuttons(stream.getName(), stream.getAttributes());
                            }
                            if (param.startsWith("attrlist")) {
                                if (param.endsWith("dir"))
                                    dialog.addCheckboxes(stream.getName(), stream.getAttributes(), new String[]{"asc", "desc"});
                                else
                                    dialog.addCheckboxes(stream.getName(), stream.getAttributes(), null);
                            }
                        }
                    }
                    if (operation.getOperationName().startsWith("group")) {
                        dialog.addTuple(stream.getObjects().get(0).copy("group"));
                    }
                    dot += ".";
                }
                
            }
            dialog.activate();
        }
        //System.out.println(operation.getOperationName()+ " " +operation.getSignature());
        if ((operation.countObjects() > 1) || ((operation.countObjects() == 1) && operation.getSignature().contains("o") && !operation.getSignature().startsWith("o"))) {
            
            lastX = getLastX();
            
            activeStream = new StreamView(operation.getOperationName(), operation.getSignature(), lastX, activeStream.getY());
            for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
                StreamView stream = (StreamView)iter.next();
                activeStream.addInputStream(stream);
                stream.setNext(activeStream);
            }
            fullStream.add(activeStream); 
        }
        lastObject = operation.getView();
        if (operation.getObjects()[0].equals("")) {
            ObjectView new_object = new ObjectView(operation.getOperationName(), operation.getResultType());
            addObject(new_object);
            new_object.setSignature(operation.getSignature());
        }
        else {
            activeStream.addObject(operation.getView());
        }
    }
    
    protected void addParamStream(StreamView stream){
        lastObject.addParamStream(stream);
        activeStream.addParamStream(stream);
    }
    
    /**
     * deletes the last added object of the query
     */
    public void removeLastObject(){
        if (fullStream.size() > 0) {
            StreamView lastStream = fullStream.get(fullStream.size()-1);
            lastStream.getObjects().remove(lastStream.getLastComponent());
            if (lastStream.getLength() < 1) {
                if (lastStream.getY() > 0) {
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
    
    public StreamView getStream(int x , int y){
        StreamView returnStream = null;
        for ( Iterator iter = fullStream.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            if (stream.getY() == y) {
                if ((stream.getX()-1 < x ) && (x < stream.getX() + stream.getLength() + 1))
                    returnStream = stream;
            }
        }
        return returnStream;
    }
    
    /**
     * check for attributes with the same name
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
    
    public void setActiveStreams(){
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
    
    private void getInfo() {
        //infoDialog.addInfo("Query", getStringsQuery());
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            String result = stream.getState();
            String name = stream.getName();
            if (result != null) {
                infoDialog.addInfo(name, result);
                infoDialog.view();
            }
        }
    }
    
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
    
    private String getTypeString(){
        String query = "query ";
        
        for ( Iterator iter = activeStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            query += stream.getTypeString()+" ";
        }
        
        return query;
    }
    
    private void setType(ListExpr obj) {
        if (obj != null) {
            result = obj.second().textValue();
            if (result.equals("undefined"))
                result = obj.first().textValue();
        }
    }
    
    /**
     * update the panel and the stream information
     */
    public String update() {
        setActiveStreams();
        System.out.println(activeStreams.size());
        ListExpr type = updateStream(activeStream);
        if (activeStreams.size() > 1)
            type = viewer.getType(this.getTypeString());
        
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
    
    protected String getType(){
        return result;
    }
    
    public void paintComponent(Graphics g) {
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
