/*
 * JPEGViewer.java
 *
 * Created on 25. Dezember 2003, 19:35
 */

package viewer.jpeg;

import gui.idmanager.ID;
import gui.SecondoObject;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.*;
import java.util.Vector;
import java.util.Hashtable;
import javax.imageio.*;
import javax.swing.*;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.plaf.basic.BasicButtonListener;
import javax.swing.tree.*;
import sj.lang.ListExpr;
import tools.Base64Decoder;
import viewer.SecondoViewer;


/** The viewer for JPEG <strong>SECONDO</strong>-objects.
 *  <ul><h4>History</h4> only versions published are recorded
 *      <li><strong>0.1alpha</strong> frame of functionality 2004-01-10
 *      <li><strong>0.6beta</strong> functionality for jpegs 2004-02-04
 *      <li><strong>0.7beta</strong> jinfo analysed completly, display partly 2004-02-10
 *      <li><strong>0.8beta</strong> jinfo completed, tool tips added 2004-02-12N
 *  </ul>
 *  @author Stefan Wich mailto:stefan.wich@fernuni-hagen.de
 *  @version 0.8beta
 */
public class JPEGViewer extends SecondoViewer {

    private static final String C_JPEG= "jpeg";
    private static final String C_JINFO= "jinfo";
    private static final String C_STRING= "string";

    private static final ListExpr JPEG_TYPELIST = ListExpr.symbolAtom("jpeg");
    private static final ListExpr JINFO_TYPELIST = ListExpr.symbolAtom("jinfo");
    /** Holds objectList in the west and jScPicturesPane in the east.*/
    //private JSplitPane jSpMainPanel;
    private JScrollPane jSpMainPanel;
    /** Area where JPEGs are diplayed.*/
    private JScrollPane jScPicturesPane;
    /** The Button holding the icons.*/
    private JButton iconButton;
    /** Header of the pictures pane */
    private JViewport jVPicPaneHeader;
    /** Area holding a tree with the objects in a relation (view).*/
    private JTree jTrObjListPane;
    /** The model to e displayed as a tree.*/
    private JpegTreeModel jTMcurrentModel = null;
    /** Dropdown holding list of objects questioned before.*/
    private JComboBox jCObjChooserDD;
    /** The objects cached by the Viewer.*/
    //private sj.lang.SecondoObjectVector vecJpegObj;
    private Vector vecJpegObj;
    /** The objects with content decoded. For faster handling. Key by object ID of SecondoObject. */
    private Hashtable hashDecodedJpegObj;
    /** The name for the viewer application, to add as menu entry. */
    private static final String viewerName="JPEG Viewer";
    /** The Listener for mouse clicks on picture buttons. */
    private MouseListener mLPicButton;
    /** Metadata of currently displayed object.*/
    private JPGViewerMetaData jVMDCurrentDisplay = null;
    //    /** The index of the currently focussed <code>SecondoObject</code>. */
    //    private int focussedID=-1;

    /** Creates a new instance of JPEGViewer */
    public JPEGViewer() {
        setDebugMode(true);
        vecJpegObj = new Vector();
        hashDecodedJpegObj = new Hashtable();
        initPane();
        //        validate();
    }

    /** Initialize the viewers working pane. Create all necessary objects. */
    private void initPane(){
        setLayout(new BorderLayout());
        //Prepare object drop dwon for selection.
        jCObjChooserDD = new JComboBox();
        jCObjChooserDD.addItemListener(new ItemListener(){
            public void itemStateChanged(ItemEvent evt){
                if (DEBUG_MODE) System.err.println(evt.getSource().toString() + ": " + evt.getItem());
                SecondoObject sObj = (SecondoObject)jCObjChooserDD.getSelectedItem();
                if (sObj!=null){
                    setFocus(sObj);
                }
            }
        });
        jCObjChooserDD.setFocusable(false);
        add(jCObjChooserDD,BorderLayout.NORTH);
        //Prepare object list for relations
        //jTrObjListPane = new JTree(jTMcurrentModel);
        jTrObjListPane = initTreeTest();
        //jTrObjListPane = new JScrollPane(JScrollPane.VERTICAL_SCROLLBAR_NEVER, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        //Prepare pictures pane for display of objects.
        jScPicturesPane = new JScrollPane(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        jVPicPaneHeader = new JViewport();
        jScPicturesPane.setColumnHeader(jVPicPaneHeader);
        //put label holding image in pictures pane.
        iconButton = new JButton();
        iconButton.setFocusable(false);
        iconButton.addMouseListener(new MouseListener(){
            public void mouseClicked(MouseEvent e) {
                putPicture(jVMDCurrentDisplay.getNextImgIcon(), null);
            }
            public void mouseEntered(MouseEvent e) {}
            public void mouseExited(MouseEvent e) {}
            public void mousePressed(MouseEvent e) {}
            public void mouseReleased(MouseEvent e) {}
        });
        jScPicturesPane.getViewport().add(iconButton);
        //Put panes together into main panel.
        jSpMainPanel = jScPicturesPane;
	// new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, jTrObjListPane, jScPicturesPane);
        add(jSpMainPanel, BorderLayout.CENTER);
        resetPane();
    }

    private JTree initTreeTest(){
        JTree tree;
        String[][] sampleData = {
            {"Amy"}, {"Brandon", "Bailey"},
            {"Jodi"}, {"Trent", "Garrett", "Paige", "Dylan"},
            {"Donn"}, {"Nancy", "Donald", "Phyllis", "John", "Pat"}
        };
        
        Hashtable h = new Hashtable();
        // build up the hashtable using every other entry in the String[][] as a key
        // followed by a "value" which is a String[]
        for (int i = 0; i < sampleData.length; i+=2) {
            h.put(sampleData[i][0], sampleData[i + 1]);
        }
        return new JTree(h);
    }
    
    
    
    /** Reset the main panels content to its initial values.*/
    private void resetPane(){
        jCObjChooserDD.setEnabled(true);
        jSpMainPanel.setPreferredSize(new Dimension(600, 400));
//        jSpMainPanel.setDividerLocation(.0001);
    }
    
    
    /** Add a JPEG object to the viewer.
     *  The function checks whether or not the given object can be displayed, that the
     *  object is of a supported kind and well formed.
     *  In case it can be displayed the object is added to display and focussed.
     *  Adding is skipped whenever an object is already displayed.
     *  @param sObj The object to be displayed.
     *  @return <code>true</code> if the object could be displayed, <code>false</code>
     *  otherwise.
     *  @see JPEGviewer#canDisplay
     */
    public boolean addObject(SecondoObject sObj) {
        if (canDisplay(sObj)){
            //sw stop
            if (!isDisplayed(sObj)) {
                decode(sObj);
                vecJpegObj.add(sObj);
                jCObjChooserDD.addItem(sObj);
                //add in object tree
            }
            setFocus(sObj);
            return true;
        }
        return false;
    }
    
    /** Decode the object stored as NestedList and store unfolded.
     *  Unfolded objects are stored in a hashtable keyed by <code>sObj</code>s ID.
     * @pre List structure requirements, see algebra-spec.
     * @param sObj The secondo object to be decoded.
     */
    private void decode(SecondoObject sObj){
        ListExpr sObjList = sObj.toListExpr();
        ListExpr typeList = sObjList.first();
        ListExpr valueList = sObjList.rest();
        JPGViewerMetaData metaData = null;
        if (!isListTerminator(valueList))
            throw new IllegalStateException("Precondition violated; why? See last output before exception.");
        valueList = valueList.first();
        if (isJpegOrJinfo(typeList)){
            metaData = decode(typeList, valueList, sObj.getID());
        } else {
            Vector typesVec = isRelWithJpegOrJinfo(sObjList);
            if(typesVec!=null){
                JRelMetaData jRelMD = new JRelMetaData(sObj);
                //stop
                //build a tree model{
                JpegTreeNode jTNRoot = (JpegTreeNode)jRelMD.getModel().getRoot();
                //scan typeList: done in isRelWithJpegOrJinfo;
                if (DEBUG_MODE){
                    valueList.writeListExpr();
                }
                //create TreeNodes out of valueList
                while (valueList.first()!=null){
                    boolean first=true;
                    JpegTreeNode jTNRelEntry = new JpegTreeNode("Rel entry");
                    Iterator i=typesVec.iterator();
                    ListExpr itemList = valueList.first();
                    while(i.hasNext()){
                        JPGViewerMetaData jVMDNode;
                        String type = (String)i.next();
                        String value = itemList.first().writeListExprToString();
                        if (type.equalsIgnoreCase(C_STRING)){
                            jTNRelEntry.add(new JpegTreeNode(value));
                        } else {
                            ListExpr lEStaticType;
                            if (type.equalsIgnoreCase(C_JINFO)){
                                lEStaticType = JINFO_TYPELIST;
                            }else{
                                lEStaticType = JPEG_TYPELIST;
                            }
                            jVMDNode = decode(lEStaticType, itemList.first(), sObj.getID());
                            jTNRelEntry.add(new JpegTreeNode(jVMDNode));
                            if (first){
                                first = false;
                                jRelMD.setImgIcon(jVMDNode.getImgIcon());
                            }
                        }
                        itemList = itemList.rest();
                    }
                    //put treenodes together;
                    jTNRoot.add(jTNRelEntry);
                    valueList=valueList.rest();
                }
                //throw new NoSuchMethodError("Relations can't currently be displayed with this viewer.");
                metaData = jRelMD;
            }else throw new IllegalArgumentException("Found no objects I was able to decode.");
        }
        hashDecodedJpegObj.put(new Integer(metaData.hashCode()),metaData);
    }
    
    private JPGViewerMetaData decode(ListExpr typeList, ListExpr valueList, ID oID){
        String jpegString;
        JPGViewerMetaData metaData;
        if (typeList.symbolValue().equalsIgnoreCase(C_JPEG)){
            metaData = new JPGViewerMetaData(oID);
            if (!valueList.isAtom() || valueList.atomType()!=ListExpr.TEXT_ATOM){
                if (DEBUG_MODE) valueList.writeListExpr();
                throw new IllegalArgumentException("Base 64 encoded jpeg-string expected");
            }
            jpegString = valueList.textValue();
        }else{//jinfo
            JInfoMetaData jInfoMD = new JInfoMetaData(oID);
            //generalList: (height width coloured colorSpace numComponents)
            ListExpr generalList = valueList.first();
            //ignore width and height as it can be taken directly from jpeg
            if (!generalList.third().isAtom()||generalList.third().atomType()!=ListExpr.BOOL_ATOM){
                if (DEBUG_MODE) generalList.third().writeListExpr();
                throw new IllegalArgumentException("Boolean atom giving info if colored expected.");
            }
            boolean colored = generalList.third().boolValue();
            if (!generalList.fourth().isAtom()||generalList.fourth().atomType()!=ListExpr.INT_ATOM)
                throw new IllegalArgumentException("Integer atom giving info about colorspace expected.");
            int colSpace = generalList.fourth().intValue();
            switch (colSpace){
                case JPEGColorSpace.COL_SPACE_GRAY:
                    if (colored) {
                        if (DEBUG_MODE) generalList.writeListExpr();
                        System.err.println("Divergence in color-space given (gray) and colored attribute (true).\n"+
                        "Use of colorspace value forced.");
                    }
                    break;
                case JPEGColorSpace.COL_SPACE_RGB:
                case JPEGColorSpace.COL_SPACE_CMYK:
                    if (!colored) {
                        if (DEBUG_MODE) generalList.writeListExpr();
                        System.err.println("Divergence in color-space given (RGB/CMYK) and colored attribute (false).\n"+
                        "Use of colorspace value forced.");
                    }
                    break;
                default:
                    throw new IllegalArgumentException("Invalid value for colorspace of " + colSpace + ".");
            }
            jInfoMD.setColSpace(colSpace);
            if (!generalList.fifth().isAtom()||generalList.fifth().atomType()!=ListExpr.INT_ATOM)
                throw new IllegalArgumentException("Integer atom giving number of components expected.");
            int numComponents = generalList.fifth().intValue();
            //distList ( (a1, ..., a256) (b1, ..., b256) (c1, ..., c256) (d1, ..., d256) )
            //the meaning of the ai, bj, ck, dl's depends on the colorspace value in general list
            ListExpr distrList = valueList.second();
            if (distrList==null)
                throw new NullPointerException("Expected distribution list. Found \"null\"-reference.");
            switch (numComponents){
                case 4:
                    jInfoMD.setDistList(getFloatArray(distrList.fourth()), 3);
                case 3:
                    jInfoMD.setDistList(getFloatArray(distrList.third()), 2);
                case 2:
                    jInfoMD.setDistList(getFloatArray(distrList.second()), 1);
                case 1:
                    jInfoMD.setDistList(getFloatArray(distrList.first()), 0);
                    break;
                default:
                    throw new IllegalArgumentException("Number of components out of range (1-{1,3,4}): " + numComponents);
            }
            if (!isListTerminator(distrList.rest().rest().rest())){
                if (DEBUG_MODE) distrList.rest().rest().rest().writeListExpr();
                throw new IllegalArgumentException("List too long.");
            }
            //(B64codPict)
            ListExpr jpegList = valueList.third();
            if (jpegList==null)
                throw new NullPointerException("Expected distribution list. Found \"null\"-reference.");
            jpegList=jpegList.first();
            if (!isListTerminator(jpegList) || !(jpegList.first().isAtom() && jpegList.first().atomType()==ListExpr.TEXT_ATOM)){
                if (DEBUG_MODE) jpegList.first().writeListExprToString();
                throw new IllegalArgumentException("Base 64 encoded jpeg-string expected");
            }
            jpegString = jpegList.first().textValue();
            metaData = jInfoMD;
        }
        BufferedImage decImg = null;
        try{
            decImg = decodeImg(jpegString);
        }catch(IOException ioe){
            ioe.printStackTrace(System.err);
        }
        ImageIcon iIcon = new ImageIcon(decImg);
        metaData.setImgIcon(iIcon);
        return metaData;
    }
    
    /** Checks if the object can be displayed.
     * For this task the list representation of the object sObj is checked.
     * If the object is a JPEG, JINFO, REL everything is ok.
     * @param sObj The object to check our capability to display.
     * @return <CODE>TRUE</CODE> if we can display the object, <CODE>FALSE</CODE> otherwise.
     */
    public boolean canDisplay(SecondoObject sObj) {
        ListExpr listExpr = sObj.toListExpr();
        if (DEBUG_MODE) System.out.println(listExpr.writeListExprToString());
        ListExpr typeList = listExpr.first();
        if (typeList.isAtom() && typeList.atomType()==ListExpr.SYMBOL_ATOM && isListTerminator(listExpr.rest())){
            if ( !isJpegOrJinfo(typeList) ){
                if (DEBUG_MODE) System.out.println("None of the types jpeg, jinfo found.");
                return false;
            }
        }else if ( isRelWithJpegOrJinfo(listExpr)!=null ) {
            return true;
        }else{
            if (DEBUG_MODE) {
                typeList.writeListExpr();
                System.out.println("Found neither of jpeg, jinfo, rel.");
            }
            return false;
        }
        return true;
    }
    
    /** Give information about whether or not an object is currently displayed.
     *  Being displayed means, the object is in the viewers object list.
     *  It does <u>not</u> mean that the object is visible.
     *  This function should be in the abstract class as the object vector should be too.
     *  @param sObj The (valid) object to obtain information about its display status.
     *  @return <code>true</code> if the object is currently displayed, <code>false</code>
     *  otherwise.
     *  @throws <code>NullPointerException</code> if <code>sObj</code> is <code>null</code>.
     */
    public boolean isDisplayed(SecondoObject sObj) {
        if (sObj==null) throw new
        NullPointerException("\"null\" reference given instead of valid SecondoObject");
        return vecJpegObj.contains(sObj);
    }
    
    /** Removes an object from display.
     *  If the object is currently displayed it is removed from display.
     *  Otherwise nothing changes.
     *  @param sObj The object to remove from display.
     */
    public void removeObject(SecondoObject sObj) {
        if (sObj==null)
            throw new NullPointerException("Can not delete \"null\" as it is not stored.");
        hashDecodedJpegObj.remove(new Integer(sObj.getID().hashCode()));
        vecJpegObj.remove(sObj);
        if (sObj==jCObjChooserDD.getSelectedItem()) {
            //select previous object.
            int sId = jCObjChooserDD.getSelectedIndex()-1;
            //if first is selected and to be deleted, go to end of object list.
            if (sId==-1) sId+=jCObjChooserDD.getItemCount();
            jCObjChooserDD.setSelectedIndex(sId);
        }
        jCObjChooserDD.removeItem(sObj);
    }
    
    /** The given object is brought to pictures pane. In case of a relation the first picture
     * in relation is displayed.
     * @param sObj The object to be displayed.
     * @return true on success, false otherwise.
     */
    public boolean selectObject(SecondoObject sObj) {
        setFocus(sObj);
        return true;
    }
    
    /** Give the name of the viewer.
     *  This is used in wrapper application for the list of known viewers.
     *  @return the viewers name.
     */
    public String getName() {
        return viewerName;
    }
    
    /** Remove all objects currently being displayed. */
    public void removeAll() {
        resetPane();
        vecJpegObj.clear();
        hashDecodedJpegObj.clear();
        jCObjChooserDD.removeAllItems();
    }
    
    /** Tell the quality of display for the given <code>SecondoObject</code>.
     * The quality is best (=1.0) if the object is a JPEG or a JINFO,
     * otherwise if there is a Relation (REL) it can't
     * be displayed (=0.0).
     * @param sObj The object to check display quality for.
     * @return 1.0 if object is a <strong>jpeg</strong> or jinfo, 0.0 otherwise.
     */
    public double getDisplayQuality(SecondoObject sObj) {
        ListExpr listExpr=sObj.toListExpr();
        if (listExpr.listLength()==2){
            if (isJpegOrJinfo(listExpr.first()))
                //We are the best for pure jpeg or jinfo.
                return 1.;
            else if (isRelWithJpegOrJinfo(listExpr)!=null)
                //If a jpeg or jinfo is found in a relation it is very likely
                //that we are the best player to do the job.
                return 0.8;
        }
        return 0.;
    }
    
    /** Check a list expression if it contains a JPEG or JINFO object.
     *  @param listExpr The list to check for the objects.
     *  @return <code>TRUE</code>, for JPEG and JINFO objects.
     */
    private boolean isJpegOrJinfo(ListExpr listExpr){
        if (!listExpr.isAtom()){
            if (DEBUG_MODE) System.out.println("Head of list must be a symbol atom, if jpeg or jinfo.");
            return false;
        }else if (!listExpr.symbolValue().equalsIgnoreCase(C_JPEG) &&
        !listExpr.symbolValue().equalsIgnoreCase(C_JINFO))
            return false;
        return true;
    }
    
    /** Check a list expression if it contains a relation with jpeg or jinfo in it.
     * @param listExpr the list to check for a relation with jpegs or jinfos.
     * @return true, if this list is a relation and it contains jpeg or jinfo objects; flase,
     * otherwise.
     */
    private Vector isRelWithJpegOrJinfo(ListExpr listExpr){
        Vector retVector = null;
        boolean found = false;
        
        ListExpr typeList = listExpr.first();
        if (typeList.isAtom() || !isListTerminator(listExpr.rest())) {
            if (DEBUG_MODE) System.err.println("List of length 2 expected.");
        }
        if (typeList.first().isAtom() && typeList.first().symbolValue().equalsIgnoreCase("rel")){
            ListExpr tupleList = typeList.rest().first();
            if (tupleList.first().isAtom() && tupleList.first().symbolValue().equalsIgnoreCase("tuple")){
                retVector = new Vector();
                //now an arbitrary long list of (name, type) pairs will be read.
                tupleList = tupleList.rest().first();//tL.first() is 'tuple'
                if (DEBUG_MODE)
                    tupleList.writeListExpr();
                while (tupleList.first()!=null){//.endOfList()) {
                    ListExpr tElem = tupleList.first().rest().first();
                    if(tElem.isAtom() && tElem.atomType()==ListExpr.SYMBOL_ATOM){
                        String typeName = tElem.symbolValue();
                        if (typeName.equalsIgnoreCase(C_JPEG) || typeName.equalsIgnoreCase(C_JINFO)){
                            retVector.add(typeName);
                            found=true;
                        }else{
                            retVector.add(C_STRING);
                        }
                    }else{
                        found = false;
                        if (DEBUG_MODE)
                            tElem.writeListExpr();
                        System.err.println("Expected (name type) pairs.");
                        break;
                    }
                    tupleList = tupleList.rest();
                    if (DEBUG_MODE && tupleList.first()!=null)
                        tupleList.first().writeListExpr();
                }
            }else{
                if (DEBUG_MODE) {
                    tupleList.writeListExpr();
                    System.err.println("Expected tuple after rel.");
                }
            }
        }
        if (!found) retVector = null;
        if (DEBUG_MODE && retVector!=null) System.err.println(retVector.toString());
        return retVector;
    }
    
    /** Set focus for the given <code>SecondoObject</code>.
     *  Depending on type of given object the method to display is selected.
     *  <ul>
     *      <li><strong>jpeg</strong> - only display it in pictures pane.</li>
     *      <li><strong>jinfo</strong> - display in pictures pane.</li>
     *      <li><strong>rel</strong> - build a tree of thumbnails enriched with
     *          information from relation.
     *      </li>
     *  </ul>
     *  @param sObj The object to focus on.
     */
    private void setFocus(SecondoObject sObj){
        if (sObj!=jCObjChooserDD.getSelectedItem()) jCObjChooserDD.setSelectedItem(sObj);
        //Get info about object decoded beforehand.
        JPGViewerMetaData metaData = (JPGViewerMetaData)(hashDecodedJpegObj.get(new Integer(sObj.getID().hashCode())));
        if (metaData==null) {
            if (DEBUG_MODE) System.err.println(sObj.getID().hashCode());
            throw new IllegalStateException("setFocus must not be called before decoding an object.");
        }
        if (metaData instanceof JRelMetaData){
            JRelMetaData jRelMD = (JRelMetaData) metaData;
            //throw new NoSuchMethodError("setFocus implementation missing for Relation.");
            //display tree representation in jTrObjListPane
            jTMcurrentModel = new JpegTreeModel((JpegTreeNode)jRelMD.getModel().getRoot());
            jTrObjListPane = new JTree(jTMcurrentModel);
            //select first element and
            putPicture(jRelMD.getImgIcon(), "Picture taken from " + sObj.getName());
        }else {//jpeg, jinfo
            ImageIcon tempIcon = metaData.getImgIcon();
            if (metaData instanceof JInfoMetaData)
                tempIcon.setDescription("More info available. Click picture to see it.");
            putPicture(tempIcon,sObj.getName());
        }
        jVMDCurrentDisplay=metaData;
        //stop
    }
    
    /** Puts the picture into the pictures pane. Also adds a title with the query
     *  the object is originating from and its width and height.
     *  @param iIcon the image to be displayed.
     *  @param imgSrc the query source of the object, if <code>null</code> no update occurs for the header.
     */
    private void putPicture(ImageIcon iIcon,String imgSrc){
        if (iIcon==null){
            if (DEBUG_MODE)
                System.err.println("No icon to display.");
            return;
        }
        if (imgSrc!=null)
            jScPicturesPane.getColumnHeader().add(new JLabel(imgSrc + " ("+iIcon.getIconWidth()+","+iIcon.getIconHeight()+")"));
        iconButton.setIcon(iIcon);
        iconButton.setToolTipText(iIcon.getDescription());
        iconButton.setSize(iIcon.getIconWidth(), iIcon.getIconHeight());
    }
    
    /** Create a <code>BufferedImage</code> out of an base64 encoded string.
     * @return The decode BufferedImage.
     * @param jpegBase64 the string to decode.
     * @throws IOException in the unlikely case an error occurs whilst reading the pictures data.
     */
    private BufferedImage decodeImg(String jpegBase64)throws IOException{
        BufferedImage decImg=null;
        Base64Decoder jpegDecoder = new Base64Decoder(new java.io.StringReader(jpegBase64));
        ByteArrayOutputStream byteBuffer = new ByteArrayOutputStream();
        int byteRead=jpegDecoder.getNext();
        while (byteRead>=0){
            byteBuffer.write(byteRead);
            byteRead=jpegDecoder.getNext();
        }
        decImg = ImageIO.read(new ByteArrayInputStream(byteBuffer.toByteArray()));
        return decImg;
    }
    
    /** Check if the given (value-)list terminates the <code>ListExpr</code>.
     *  Using this method gives debugging info (and avoids using the
     *  ListExpr.endOfList() with uncaught <code>null</code>-reference).
     * @param valueList the list to test if it terminates a list.
     * @return true, if list has no next and itself is not null; false, otherwise.
     */
    private final boolean isListTerminator(ListExpr valueList){
        if (valueList==null){
            if (DEBUG_MODE) System.out.println("List too short.");
            return false;
        }else if (!valueList.rest().isEmpty()){
            if (DEBUG_MODE) System.out.println("List too long.");
            return false;
        }
        return true;
    }
    
    /** Converts a list of 256 float list values into an array.
     *  @param float256List a list that should consist of 256 float list values.
     *  @return the list converted into an array.
     */
    private static float [] getFloatArray(ListExpr float256List){
        ListExpr tempList;
        if (float256List==null)
            throw new NullPointerException("Cannot create float[] from \"null\"-list");
        float[] values = new float[256];
        for (int i=0;i<256;i++){
            tempList = float256List.first();
            if (tempList==null || !tempList.isAtom() || tempList.atomType()!=ListExpr.REAL_ATOM || (values[i]=(float)tempList.realValue())<0 || values[i]>5.0)
                throw new IllegalArgumentException("List must consist only of floats in range 0<=x<=5.0 .");
            float256List = float256List.rest();
            if (float256List==null)
                throw new IllegalArgumentException("Premature list end, expected 256 floats; broken at "+i);
        }
        return values;
    }
    
    /** Main method for fast testing.
     *  To be removed on roll-out.
     * @param args the command line arguments.
     */
    public static final void main(java.lang.String[] args) {
        JFrame mainWin = new JFrame("JPEG Viewers Test");
        Container contPane = mainWin.getContentPane();
        contPane.add(new JPEGViewer());
        mainWin.addWindowListener(new WindowAdapter(){
            public void windowClosing(WindowEvent evt){
                System.exit(0);
            }
        });
        mainWin.pack();
        mainWin.setVisible(true);
    }
}
