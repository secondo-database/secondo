/*
 * JpegTreeModel.java
 *
 * Created on 13. Januar 2004, 17:12
 */

package viewer.jpeg;

/** Provide a tree Model for Relations.
 *
 *  @author <a mailto=stefan.wich@fernuni-hagen.de>Stefan Wich</a>
 */
public class JpegTreeModel extends javax.swing.tree.DefaultTreeModel{
    
    JpegTreeModel(JpegTreeNode root){
        //stop should be replaced by query result.
        super(root);
    }
    
}
