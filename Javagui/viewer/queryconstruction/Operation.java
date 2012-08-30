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
import javax.swing.BorderFactory;
import javax.swing.JTextArea;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class Operation extends ObjectView { 
    
    private String name;
    private String label;
    private String[] objects;
    private String[] parameter;
    
    private String result;
    private String brackets;
    private String signature;
    private ObjectView view;
    
    /**
     * 
     * @param name name of the operation
     * @param objects type of objects which the operation needs
     * @param signature signature of the operation
     * @param parameter type of parameter which the operation needs
     * @param result type of the result, when needed
     */
    public Operation(String name, String[] objects, String signature, String[] parameter, String result){
        this.setOpaque(false);
        this.setPreferredSize(new Dimension(120, 30));
        this.name = name;
        this.label = name;
        this.signature = signature;
        this.result = result;
        this.parameter = parameter;
        this.objects = objects;
        Arrays.sort(this.objects);
        
        view = new ObjectView(ObjectType.OPERATION, name);
        view.setSignature(signature);
    }  
    
    public void paintComponent(Graphics g) {
        this.setBorder(BorderFactory.createEtchedBorder());
        int w = g.getFontMetrics().stringWidth(label);
        String s = label;
        if (w > 80) {
            s = label.substring(0, 12);
        }
        
        g.drawString(s, 20, this.getSize().height/2 + 5);
        
    }
    
    public String getSignature() {
        return signature;
    }
    
    protected Operation copy() {
        return new Operation(this.name, this.objects, this.signature, this.parameter, this.result);
    }
    
    public void setResultType(String result) {
        this.result = result;
    }
    
    public String getResultType() {
        return result;
    }
    
    protected String getOperationName() {
        return getView().getObjectName();
    }
    
    protected String getLabel() {
        return label;
    }
    
    protected void setLabel() {
        if ((0 < getView().getObjectName().length()) && (getView().getObjectName().length() < 10))
            label = getView().getObjectName();
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
    
    public String getBrackets() {
        if (brackets == null)
            return "";
        else return brackets;
    }
    
    public String[] getParameter() {
        return this.parameter;
    }
    
    public void setParameter(String par, int index) {
        this.parameter[index] = par;
    }
    
}
