/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.Dimension;
import java.awt.GridLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 *
 * @author lrentergent
 */
public class StreamView extends JPanel {
    
    private String[] parts;
    private String[] attributes;
    private String[] attrtypes;
    private String name;
    
    protected static final int EMPTY = 0;
    protected static final int TUPEL = 1;
    protected static final int STREAM = 2;
    protected static final int TWOSTREAMS = 3;
    protected static final int TWORELATIONS = 4;
    
    public StreamView(String name, String str) {
        this.name = name;
        this.setAttributes(str.split("\\("));
        this.parts = str.split("\\(");
        setLayout(new GridLayout(0,2));
    }
    
    public String[] getParts() {
        return parts;
    }
    
    public final void setAttributes(String[] attributes) {
        int i = 4;
        int j = 0;
        
        this.attributes = new String[attributes.length -4];
        this.attrtypes = new String[attributes.length -4];
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
    
    public String getState() {
        return parts[1];
    }
    
    public int getHeight() {
        return attributes.length;
    }
    
    public void print() {
        for(String s: parts) {
            System.out.println(s);
        }
    }
    
    public void view(InfoDialog dialog) {
        this.setPreferredSize(new Dimension(200, (attributes.length + 1) * 50));
        
        dialog.add(new JLabel(name));
        dialog.add(new JLabel(parts[1]));
        int i = 0;
        for (String att: attributes) {
            JLabel label = new JLabel( att, JLabel.LEADING );
            dialog.add(label);
            
            dialog.add(new JLabel( attrtypes[i] ));
            i++;
        }
    }
    
}
