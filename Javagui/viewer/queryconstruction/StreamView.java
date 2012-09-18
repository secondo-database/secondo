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

import java.awt.Color;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * Stream of objects and operators.
 */
public class StreamView {

    //list of query objects
    private ArrayList<ObjectView> objects = new ArrayList<ObjectView>();
    //input streams as objects and parameters
    private ArrayList<StreamView> inputStreams = new ArrayList<StreamView>();
    private ArrayList<StreamView> paramStreams = new ArrayList<StreamView>();
    
    private String[] parts;
    private String[] attributes;
    private String[] attributesSorted;
    private String[] attrtypes;
    private char[] signature;
    private String name;
    private String type;
    private String state;
    private int[] line;
    private boolean active = true;
    private boolean hasNext = false;
    private int xpos;
    private int ypos;

    public StreamView(String name, String signature, int x, int y) {
        this.name = name;
        this.xpos = x;
        this.ypos = y;
        this.signature = signature.toCharArray();
    }

    /**
     * Add a stream as an input parameter for the stream object.
     * @param stream input stream
     */
    protected void addInputStream(StreamView stream) {
        inputStreams.add(stream);
    }
    
    /**
     * Add a line between an input stream and another stream.
     * @param x x-position of the next stream
     * @param y y-position of the next stream
     */
    private void addLine(int x, int y) {
        line = new int[2];
        line[0] = x;
        line[1] = y;
    }

    /**
     * Add an object to the stream.
     * @param object 
     */
    protected void addObject(ObjectView object) {
        objects.add(object);
    }
    
    /**
     * Add a parameter stream to the stream.
     * @param stream 
     */
    protected void addParamStream(StreamView stream) {
        paramStreams.add(stream);
    }
    
    /**
     * Add an attribute to the list of sorted attributes.
     * @param new_attr 
     */
    protected void addSortedAttribute(String new_attr) {
        int i = 0;
        for (String attr: this.attributesSorted){
            if (attr == null){
                this.attributesSorted[i] = new_attr;
                return;
            }
            if (attr.equals(new_attr))
                return;
            i++;
        }
    }

    /**
     * Change the active state of the stream.
     */
    protected void change() {
        if (objects.size() > 0) {
            this.active = !this.active;
        }
    }
    
    /**
     * 
     * @param dot
     * @return attributes as objects
     */
    protected ObjectView[] getAttrObjects(String dot) {
        int i = 0;
        ObjectView objectViews[] = new ObjectView[attributes.length];
        for (String objName : attributes) {
            objectViews[i] = new ObjectView(dot + objName, attrtypes[i]);
            i++;
        }
        return objectViews;
    }
    
    /**
     * 
     * @return all attributes
     */
    protected String[] getAttributes() {
        return attributes;
    }

    /**
     * 
     * @return only sorted attributes
     */
    protected String[] getAttributesSorted() {
        return attributesSorted;
    }
    
    /**
     * Returns the last object of a stream.
     * @return last object of type ObjetView
     */
    protected ObjectView getLastComponent() {
        if (this.objects.size() > 0) {
            return this.objects.get(objects.size() - 1);
        }
        return null;
    }
    
    /**
     * 
     * @return length of the object list
     */
    protected int getLength() {
        return this.objects.size();
    }
    
    protected String getName() {
        return this.name;
    }

    /**
     * Return a list of all objects of the stream.
     * @return ArrayList of all ObjectView objects
     */
    protected ArrayList<ObjectView> getObjects() {
        return this.objects;
    }
    
    protected String getState() {
        return state;
    }
    
    /**
    * Return the string of the query with attributes as constant objects.
    * @return 
    */
    protected String getString() {
        
        String result = "";
        int index = 0;
        int iS = 0;
        int pS = 0;
        
        for (Iterator iter = objects.iterator(); iter.hasNext();) {
            ObjectView object = (ObjectView) iter.next();
            if ((index == 0) && (signature.length != 0)) {
                for (char c : signature) {
                    switch(c) {
                        case OperationsDialog.obChar: 
                            if (iS < inputStreams.size()){
                                result += inputStreams.get(iS).getString();
                            }
                            iS++;
                            break;
                        case OperationsDialog.opChar:
                            result += object.getOnlyName().trim();
                            break;
                        case OperationsDialog.pChar:
                            if (pS < paramStreams.size()) {
                                for (Iterator iterParam = paramStreams.get(pS).getObjects().iterator(); iterParam.hasNext();) {
                                    ObjectView paramObject = (ObjectView) iterParam.next();
                                    result += paramObject.getObjectName().trim();
                                }
                                pS++;
                            }
                            break;
                        default:
                            if (iS <= inputStreams.size()) {
                                result += c;
                            }
                            break;
                    }
                }
                result += "";
            }
            else {
                if (object.getLabel().equals("group"))
                    result += "group ";
                else
                    result += object.getObjectName().trim() + "";
            }
            index++;
        }
        return result;
    }
    
    /**
     * Get result type of the query in the stream.
     * @return 
     */
    protected String getType() {
        return type;
    }
    
    /**
     * Return the string of the query with attributes as constant objects.
     * @return query
     */
    protected String getTypeString(){
        
        String result = "";
        int index = 0;
        int iS = 0;
        int pS = 0;
        
        for (Iterator iter = objects.iterator(); iter.hasNext();) {
            ObjectView object = (ObjectView) iter.next();
            if ((index == 0) && (signature.length != 0)) {
                for (char c : signature) {
                    switch(c) {
                        case OperationsDialog.obChar: 
                            if (iS < inputStreams.size()){
                                result += inputStreams.get(iS).getTypeString();
                            }
                            iS++;
                            break;
                        case OperationsDialog.opChar:
                            result += object.getConst(true);
                            break;
                        case OperationsDialog.pChar:
                            if (pS < paramStreams.size()) {
                                for (Iterator iterParam = 
                                        paramStreams.get(pS)
                                        .getObjects().iterator(); 
                                        iterParam.hasNext();) {
                                    ObjectView paramObject = 
                                            (ObjectView) iterParam.next();
                                    result += paramObject
                                            .getObjectName().trim();
                                }
                                pS++;
                            }
                            break;
                        default:
                            if (iS <= inputStreams.size()) {
                                result += c;
                            }
                            break;
                    }
                }
                result += " ";
            }
            else {
                /* Attribute objects should be treated as 
                 * constants in inner queries. */
                result += object.getConst(false) + "";
            }
            index++;
        }
        return result;
    }

    /**
     * @return y-position of this stream
     */
    protected int getX() {
        return xpos;
    }

    /**
     * @return x-position of this stream
     */
    protected int getY() {
        return ypos;
    }

    /**
     * @return stream is active
     */
    protected boolean isActive() {
        return this.active;
    }
    
    /**
     * Paints all objects and operations of the stream.
     * @param g 
     */
    protected void paintComponent(Graphics g) {
        int x = xpos;

        for (Iterator iter = objects.iterator(); iter.hasNext();) {
            ObjectView object = (ObjectView) iter.next();

            object.paintComponent(g, x, ypos);
            if (!iter.hasNext() && active && !hasNext) {
                g.setColor(Color.red);
            }
            g.drawLine(10 + x * 120 + 90, 10 + ypos * 80 + 25, 
                    10 + x * 120 + 120, 10 + ypos * 80 + 25);
            x++;

            if (!iter.hasNext() && (line != null)) {
                g.setColor(Color.black);
                g.drawLine(10 + x * 120, 10 + ypos * 80 + 25, 
                        10 + line[0] * 120, 10 + ypos * 80 + 25);
                g.drawLine(10 + line[0] * 120, 10 + ypos * 80 + 25, 
                        10 + line[0] * 120, 10 + line[1] * 80 + 25);
            }
        }

    }
    
    /**
     * Remove the stream and set the input streams active.
     */
    protected void remove(){
        for ( Iterator iter = inputStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            stream.removeNext();
        }
    }
    
    /**
     * Remove the following stream.
     */
    private void removeNext() {
        hasNext = false;
        line = null;
        active = true;
    }
    
    /**
     * Remove all saved sorted Attribute names of the list.
     */
    protected void removeSortedAttributes(){
        int i = 0;
        while (i < attributesSorted.length) {
            attributesSorted[i++] = null;
        }
    }
    
    /**
     * Set the attributes if the intermediate result is a relation or a stream of tuples.
     * @param attributes Array of attribute names and values;
     */
    private void setAttributes(String[] attributes) {
        int i = 4;
        int j = 0;

        this.attributes = new String[attributes.length - 4];
        if (attributesSorted == null)
            this.attributesSorted = new String[attributes.length - 4];
        this.attrtypes = new String[attributes.length - 4];
        
        while (i < attributes.length) {
            attributes[i] = attributes[i].replaceAll("\\)", "");
            String[] att = attributes[i].split(" ");
            this.attributes[j] = att[0];
            this.attrtypes[j] = att[1];
            i++;
            j++;
        }
    }
    
    /**
     * Set the following stream if this stream is an input stream.
     * @param nextStream 
     */
    protected void setNext(StreamView nextStream) {
        hasNext = true;
        this.active = false;
        this.addLine(nextStream.getX(), nextStream.getY());
    }
    
    /**
     * Set the actual state of the object stream.
     * @param str Result of the server communication.
     */
    protected void setState(String str) {
        this.state = str;
        this.parts = str.split("\\(");
        if (parts.length > 4) {
            this.setAttributes(str.split("\\("));
        }
        if (parts.length > 1) {
            this.type = parts[1].replace(")", "");
        } else {
            this.type = str.replace(")", "");
        }
    }
}
