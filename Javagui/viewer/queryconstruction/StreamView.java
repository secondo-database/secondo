/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 *
 * @author lrentergent
 */
public class StreamView {
    
    private ArrayList<ObjectView> objects = new ArrayList<ObjectView>();
    private String[] parts;
    private String[] attributes;
    private String[] attrtypes;
    private String name;
    private String state;
    private int[] line;
    private boolean active = true;
    private boolean hasNext = false;
    private StreamView nextStream;
    private int xpos;
    private int ypos;
    
    protected static final int EMPTY = 0;
    protected static final int TUPEL = 1;
    protected static final int STREAM = 2;
    protected static final int TWOSTREAMS = 3;
    protected static final int TWORELATIONS = 4;
    
    public StreamView(String name, int x, int y) {
        this.name = name;
        this.xpos = x;
        this.ypos = y;
    }
    
    public void setState(String str) {
        this.parts = str.split("\\(");
        if (parts.length > 4) {
            this.setAttributes(str.split("\\("));
        }
        if (parts.length > 1) {
            this.state = parts[1];
        }
        else {
            this.state = str;
        }
    }
    
    public void addStream(ArrayList<ObjectView> list){
        objects.addAll(list);
    }
    
    public void addObject(ObjectView object){
        objects.add(object);
    }
    
    public void change() {
        if (objects.size() > 0) {
            this.active = !this.active;
        }
    }
    
    public boolean isActive() {
        return this.active;
    }
    
    public boolean hasNext() {
        return hasNext;
    }
    
    public void setNext(StreamView nextStream){
        hasNext = true;
        this.addLine(nextStream.getX(), nextStream.getY());
    }
    
    public void removeNext(){
        hasNext = false;
        line = null;
    }
    
    public StreamView next() {
        return nextStream;
    }
    
    public ObjectView getLastComponent() {
        return this.objects.get(objects.size()-1);
    }
    
    public ArrayList<ObjectView> getObjects() {
        return this.objects;
    }
    
    public String getName() {
        return this.name;
    }
    
    public int getLength() {
        return this.objects.size();
    }
    
    public final void setAttributes(String[] attributes) {
        int i = 4;
        int j = 0;
        
        this.attributes = new String[attributes.length - 4];
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
    
    public String[] getAttributes() {
        return attributes;
    }
    
    public ObjectView[] getAttrObjects() {
        int i = 0;
        ObjectView objectViews[] = new ObjectView[attributes.length];
        for (String name : attributes) {
            objectViews [i] = new ObjectView(attrtypes[i], name);
            i++;
        }
        return objectViews;
    }
    
    public String getState() {
        return state;
    }
    
    public int getHeight() {
        return attributes.length;
    }
    
    public int getX() {
        return xpos;
    }
    
    public int getY() {
        return ypos;
    }
    
    public void print() {
        if (parts != null) {
            for(String s: parts) {
                System.out.println(s);
            }
        }
        
    }
    
    public void addLine(int x, int y) {
        line  = new int[2];
        line[0] = x;
        line[1] = y;
    }
    
    public void paintComponent(Graphics g) {
        int x = xpos;
        //System.out.println("x: " + xpos + " y: " + ypos);
        
        //paints all objects and operations of the stream
        for ( Iterator iter = objects.iterator(); iter.hasNext(); ) {
            ObjectView object = (ObjectView)iter.next();
            
            object.paintComponent( g, x, ypos, Color.black);
            if (!iter.hasNext() && active && !hasNext){
                g.setColor(Color.red);
            }
            g.drawLine(10 + x*120 + 90, 10 + ypos*80 + 25, 10 + x*120 + 120, 10 + ypos*80 + 25);
            x++;
            
            if (!iter.hasNext() && (line != null)) {
                g.setColor(Color.black);
                g.drawLine(10 + x*120, 10 + ypos*80 + 25, 10 + line[0]*120, 10 + ypos*80 + 25);
                g.drawLine(10 + line[0]*120, 10 + ypos*80 + 25, 10 + line[0]*120, 10 + line[1]*80 + 25);
            }
        }

    }
    
}
