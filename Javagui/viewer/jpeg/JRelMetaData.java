/*
 * JRelMetaData.java
 *
 * Created on 10. Februar 2004, 21:35
 */

package viewer.jpeg;

import javax.swing.tree.*;
import gui.*;
import gui.idmanager.ID;


/** This class holds meta-data of Relations of <strong>Secondo</strong>-objects
 *  containing JPEG- or JINFO objects.
 *
 *  @author Stefan Wich <mailto:stefan.wich@fernuni-hagen.de>
 */
public class JRelMetaData extends JPGViewerMetaData {
    
    private JpegTreeModel jTMRelation;
    /** Creates a new instance of JRelMetaData */
    public JRelMetaData(ID objID) {
        super(objID);
        jTMRelation = new JpegTreeModel(null);
    }
    
    public JRelMetaData(SecondoObject sObj){
        super(sObj.getID());
        jTMRelation = new JpegTreeModel(new JpegTreeNode(sObj.toString()));
    }
    
    public void setRoot(SecondoObject sObj){
        JpegTreeNode jTNRoot = new JpegTreeNode(sObj.toString());
        jTMRelation.setRoot(jTNRoot);
    }
    
    public JpegTreeModel getModel(){ return jTMRelation; }
}
