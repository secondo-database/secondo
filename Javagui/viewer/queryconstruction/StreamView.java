package viewer.queryconstruction;

import java.awt.Color;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.Iterator;

/**
 *
 * @author lrentergent
 */
public class StreamView {

    private ArrayList<ObjectView> objects = new ArrayList<ObjectView>();
    private String[] parts;
    private String[] attributes;
    private String[] attrtypes;
    private char[] signature;
    private String name;
    private String type;
    private String state;
    private int[] line;
    private boolean active = true;
    private boolean hasNext = false;
    private ArrayList<StreamView> inputStreams = new ArrayList<StreamView>();
    private ArrayList<StreamView> paramStreams = new ArrayList<StreamView>();
    private int xpos;
    private int ypos;

    public StreamView(String name, String signature, int x, int y) {
        this.name = name;
        this.xpos = x;
        this.ypos = y;
        this.signature = signature.toCharArray();
    }

    protected void addInputStream(StreamView stream) {
        inputStreams.add(stream);
    }

    protected void addObject(ObjectView object) {
        objects.add(object);
    }
    
    protected void addParamStream(StreamView stream) {
        paramStreams.add(stream);
    }

    protected void change() {
        if (objects.size() > 0) {
            this.active = !this.active;
        }
    }

    protected boolean isActive() {
        return this.active;
    }

    protected void setNext(StreamView nextStream) {
        hasNext = true;
        this.active = false;
        this.addLine(nextStream.getX(), nextStream.getY());
    }
    
    protected void setSignature(String sig) {
        this.signature = sig.toCharArray();
    }
    
    /**
     * remove the stream and set the input streams active
     */
    protected void remove(){
        for ( Iterator iter = inputStreams.iterator(); iter.hasNext(); ) {
            StreamView stream = (StreamView)iter.next();
            stream.removeNext();
        }
    }

    /**
     * remove the following stream
     */
    protected void removeNext() {
        hasNext = false;
        line = null;
        active = true;
    }

    /**
     * returns the last object of a stream
     * @return last object of type ObjetView
     */
    protected ObjectView getLastComponent() {
        if (this.objects.size() > 0) {
            return this.objects.get(objects.size() - 1);
        }
        return null;
    }

    /**
     * returns all objects of the stream
     * @return ArrayList of all ObjectView objects
     */
    protected ArrayList<ObjectView> getObjects() {
        return this.objects;
    }

    protected String getString() {
        String result = "";
        int iS = 0;
        int pS = 0;
        if (signature.length == 0) {
            for (Iterator iter = objects.iterator(); iter.hasNext();) {
                ObjectView object = (ObjectView) iter.next();
                result += object.getObjectName().trim() + " ";
            }
            if (result.length()>0)
                result = result.substring(0, result.length() - 1);
        }
        for (char c : signature) {
            switch(c) {
                case OperationsDialog.obChar: 
                    if (iS < inputStreams.size()){
                        result += inputStreams.get(iS).getString();
                    }
                    iS++;
                    break;
                case OperationsDialog.opChar:
                    for (Iterator iter = objects.iterator(); iter.hasNext();) {
                        ObjectView object = (ObjectView) iter.next();
                        result += object.getObjectName().trim()+" ";
                    }
                    if (iS == inputStreams.size()) {
                        return result;
                    }
                    break;
                case OperationsDialog.pChar:
                    if (pS < paramStreams.size()){
                        result += paramStreams.get(pS).getTypeString();
                    }
                    pS++;
                    break;
                default:
                    if (iS <= inputStreams.size()) {
                        result += c;
                    }
                    break;
            }
        }
        return result;
    }
    
    protected String getTypeString(){
        String result = "";
        int i = 0;
        int pAt = 0;
        if (signature.length == 0) {
            for (Iterator iter = objects.iterator(); iter.hasNext();) {
                ObjectView object = (ObjectView) iter.next();
                /* Attribute objects should be treated as constants in inner queries. */
                if (object.getObjectName().startsWith(".") && !object.getType().equals("param")) {
                    result += object.getConst() + " ";
                }
                else {
                    result += object.getObjectName().trim() + " ";
                }
            }
            if (result.length() > 0)
                result = result.substring(0, result.length() - 1);
        }
        for (char c : signature) {
            switch(c) {
                case OperationsDialog.obChar: 
                    if (i < inputStreams.size()){
                        result += inputStreams.get(i).getTypeString();
                    }
                    i++;
                    break;
                case OperationsDialog.opChar:
                    for (Iterator iter = objects.iterator(); iter.hasNext();) {
                        ObjectView object = (ObjectView) iter.next();
                        if (object.getObjectName().startsWith(".")) {
                            result += object.getConst()+" ";
                        }
                        else {
                            result += object.getObjectName().trim()+" ";
                        }
                    }
                    if (i == inputStreams.size()) {
                        return result;
                    }
                    break;
                case OperationsDialog.pChar:
                    if (pAt < paramStreams.size()){
                        result += paramStreams.get(pAt).getTypeString();
                    }
                    pAt++;
                    break;
                default:
                    if (i <= inputStreams.size()) {
                        result += c;
                    }
                    break;
            }
        }
        return result;
    }

    protected String getName() {
        return this.name;
    }

    protected int getLength() {
        return this.objects.size();
    }

    protected String[] getAttributes() {
        return attributes;
    }

    protected ObjectView[] getAttrObjects(String dot) {
        int i = 0;
        ObjectView objectViews[] = new ObjectView[attributes.length];
        for (String objName : attributes) {
            objectViews[i] = new ObjectView(attrtypes[i], dot + objName);
            i++;
        }
        return objectViews;
    }

    protected String getType() {
        return type;
    }
    
    protected String getState() {
        return state;
    }

    protected int getX() {
        return xpos;
    }

    protected int getY() {
        return ypos;
    }

    private void addLine(int x, int y) {
        line = new int[2];
        line[0] = x;
        line[1] = y;
    }
    
    /**
     * paints all objects and operations of the stream
     * @param g 
     */
    protected void paintComponent(Graphics g) {
        int x = xpos;

        for (Iterator iter = objects.iterator(); iter.hasNext();) {
            ObjectView object = (ObjectView) iter.next();

            object.paintComponent(g, x, ypos, Color.black);
            if (!iter.hasNext() && active && !hasNext) {
                g.setColor(Color.red);
            }
            g.drawLine(10 + x * 120 + 90, 10 + ypos * 80 + 25, 10 + x * 120 + 120, 10 + ypos * 80 + 25);
            x++;

            if (!iter.hasNext() && (line != null)) {
                g.setColor(Color.black);
                g.drawLine(10 + x * 120, 10 + ypos * 80 + 25, 10 + line[0] * 120, 10 + ypos * 80 + 25);
                g.drawLine(10 + line[0] * 120, 10 + ypos * 80 + 25, 10 + line[0] * 120, 10 + line[1] * 80 + 25);
            }
        }

    }
    
    protected void setState(String str) {
        this.state = str;
        this.parts = str.split("\\(");
        if (parts.length > 4) {
            this.setAttributes(str.split("\\("));
        }
        if (parts.length > 1) {
            this.type = parts[1];
        } else {
            this.type = str;
        }
    }
    
    protected final void setAttributes(String[] attributes) {
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
}
