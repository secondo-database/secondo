/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.Color;
import java.util.*;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.*;
import javax.swing.JTextArea;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class Operation extends JComponent { 
    
    private String name;
    private String label;
    private String[] objects;
    private String parameter = "none";
    private String result;
    private char[] brackets = new char[2];
    private ObjectView view;
    
    public Operation() {
        this.setOpaque(false);
        this.setPreferredSize(new Dimension(120, 30));
    }
    
    /**
     * 
     * @param name name of the operation
     * @param objects type of objects which the operation needs
     * @param brackets brackets around the parameter
     * @param parameter type of parameter which the operation needs
     * @param result type of the result, when needed
     */
    public Operation(String name, String[] objects, String brackets, String parameter, String result){
        
        this.name = name;
        this.label = name;
        if (brackets != null) {
            this.brackets[0] = brackets.toCharArray()[0];
            this.brackets[1] = brackets.toCharArray()[1];
        }
        if (parameter != null) {
            this.parameter = parameter;
        }
        
        this.objects = objects;
        Arrays.sort(this.objects);
        this.view = new ObjectView(ObjectType.OPERATION, name);
        this.result = result;
        
    }
    
    
    public void paintComponent(Graphics g) {
        g.drawLine(0, 30, 120, 30);
        
        int w = g.getFontMetrics().stringWidth(label);
        String s = label;
        if (w > 80) {
            s = label.substring(0, 12);
        }
        
        g.drawString(s, 20, 20);
    }
    
    public void setResultType(String result) {
        this.result = result;
    }
    
    public String getResultType() {
        return result;
    }
    
    public String getName() {
        return this.name;
    }
    
    public void setName(String name) {
        this.name = name;
        view.setName(name);
    }
    
    public String getLabel() {
        return this.label;
    }
    
    public void setLabel(String label) {
        this.label = label;
    }
    
    public ObjectView getView() {
        return this.view;
    }
    
    public String[] getObjects() {
        return this.objects;
    }
    
    public int countObjects() {
        return objects.length;
    }
    
    public char[] getBrackets() {
        return brackets;
    }
    
    public String getParameter() {
        return this.parameter;
    }
    
}
