/*
 * JpegTreeNode.java
 *
 * Created on 13. Februar 2004, 17:52
 */

package viewer.jpeg;

/**
 *
 * @author  sopra
 */



import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;


public class JpegTreeNode extends javax.swing.tree.DefaultMutableTreeNode {
    
    /** Creates a new instance of JpegTreeNode with content type "text" */
    public JpegTreeNode(String text) {
        this.setUserObject(text);
    }

    /** Creates a new instance of JpegTreeNode with content type "image" */
    public JpegTreeNode(JPGViewerMetaData jpg) {
        this.setUserObject(jpg.getImgIcon().getImage().getScaledInstance(48, 48, jpg.getImgIcon().getImage().SCALE_DEFAULT));
    }
    
}
