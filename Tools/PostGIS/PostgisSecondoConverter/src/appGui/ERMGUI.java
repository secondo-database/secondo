package appGui;

import com.mxgraph.layout.hierarchical.mxHierarchicalLayout;
import com.mxgraph.model.mxIGraphModel;
import com.mxgraph.swing.mxGraphComponent;
import com.mxgraph.util.mxConstants;
import com.mxgraph.view.mxGraph;
import com.mxgraph.view.mxStylesheet;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.logging.Logger;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JViewport;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;
import postgis.Tabelle;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;

public class ERMGUI
  implements IGlobalParameters
{
  public JFrame mERMFrame;
  JPanel panelMainFrame;
  private mxGraphComponent mgraphComponent;
  private mxGraph mgraph;
  
  public ERMGUI()
  {
    this.mERMFrame = new JFrame();
    this.panelMainFrame = new JPanel(new BorderLayout(5, 5));
  }
  
  public void init(DefaultMutableTreeNode dmtnDatenbank, String strShowDB, ArrayList<String> alRelations)
  {
    LogFileHandler.mlogger.info("ERM init by: DefaultMutableTreeNode,String,ArrayList");
    
    this.mgraph = new mxGraph();
    this.mgraph.setAutoSizeCells(true);
    this.mgraph.setCellsBendable(false);
    this.mgraph.setCellsDeletable(false);
    this.mgraph.setCellsDisconnectable(false);
    this.mgraph.setCellsEditable(false);
    this.mgraph.setDisconnectOnMove(false);
    this.mgraph.setCellsCloneable(false);
    this.mgraph.setEdgeLabelsMovable(false);
    

    Map<String, Object> edge = new HashMap();
    edge.put(mxConstants.STYLE_ROUNDED, Boolean.valueOf(true));
    edge.put(mxConstants.STYLE_ORTHOGONAL, Boolean.valueOf(true));
    edge.put(mxConstants.STYLE_EDGE, "topToBottomEdgeStyle");
    edge.put(mxConstants.STYLE_SHAPE, "connector");
    edge.put(mxConstants.STYLE_ENDARROW, mxConstants.NONE);
    edge.put(mxConstants.STYLE_VERTICAL_ALIGN, "middle");
    edge.put(mxConstants.STYLE_ALIGN, "center");
    edge.put(mxConstants.STYLE_STROKECOLOR, "#FF0000");
    edge.put(mxConstants.STYLE_STROKEWIDTH, "3");
    
    edge.put(mxConstants.STYLE_SOURCE_PERIMETER_SPACING, Integer.valueOf(0));
    
    edge.put(mxConstants.STYLE_FONTCOLOR, "#446299");
    
    mxStylesheet stylesheet = this.mgraph.getStylesheet();
    stylesheet.putCellStyle("Begin", edge);
    

    Object parent = this.mgraph.getDefaultParent();
    
    HashMap<String, Object> hmCells = new HashMap();
    
    this.mgraph.getModel().beginUpdate();
    Iterator<String> itCells;
    Dimension d;
    try
    {
      this.mgraph.setHtmlLabels(true);
      this.mgraph.setAutoSizeCells(true);
      this.mgraph.setCellsResizable(true);
      this.mgraph.setCellsBendable(false);
      this.mgraph.setCellsMovable(false);
      int iIndDatenbank = 0;
      int iIndTabellen = 0;
      for (Enumeration<?> e = dmtnDatenbank.children(); e.hasMoreElements();)
      {
        String strDatenbank = e.nextElement().toString();
        
        strDatenbank = strReplace(strDatenbank);
        if (strShowDB.equals(strDatenbank))
        {
          this.mERMFrame.setTitle("ERM - " + strDatenbank);
          
          iIndTabellen = 0;
          for (Enumeration<?> e1 = dmtnDatenbank.getChildAt(iIndDatenbank).children(); e1.hasMoreElements();)
          {
            String strTabellen = e1.nextElement().toString();
            strTabellen = strReplace(strTabellen);
            
            strTabellen = strTabellen.substring(0, strTabellen.indexOf(Tabelle.sbTableRowDelimiter.toString()));
            String strVertexCode = "";
            
            strVertexCode = generateVertexHeader(strTabellen);
            for (Enumeration<?> e2 = dmtnDatenbank.getChildAt(iIndDatenbank).getChildAt(iIndTabellen).children(); e2.hasMoreElements();)
            {
              String strSpalten = e2.nextElement().toString();
              strSpalten = strReplace(strSpalten);
              
              strVertexCode = strVertexCode + generateVertexColumns(strSpalten);
            }
            strVertexCode = strVertexCode + generateCloseVertex();
            
            Object cell = this.mgraph.insertVertex(parent, "Begin", strVertexCode, 120.0D, 120.0D, 20.0D, 20.0D);
            
            cell = this.mgraph.updateCellSize(cell);
            


            hmCells.put(strTabellen, cell);
            iIndTabellen++;
          }
          break;
        }
        iIndDatenbank++;
      }
      for (int i = 0; i < alRelations.size(); i++)
      {
        String[] strSplitRelation = ((String)alRelations.get(i)).split(":");
        
        this.mgraph.insertEdge(parent, null, "", hmCells.get(strSplitRelation[0]), hmCells.get(strSplitRelation[1]), "Begin");
      }
    }
    catch (Exception e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    catch (Error err)
    {
      Iterator<String> itCells1;
      Dimension d1;
      LogFileHandler.mlogger.severe(err.getMessage());
    }
    finally
    {
      Iterator<String> itCells1;
      Dimension d1;
      this.mgraph.getModel().endUpdate();
      
      this.mgraph.getModel().beginUpdate();
      this.mgraph.setAutoSizeCells(true);
      
      /*Etie */
      Iterator<String> itCells2 = hmCells.keySet().iterator();
      while (itCells2.hasNext()) {
        this.mgraph.updateCellSize(hmCells.get(itCells2.next()));
      }
      this.mgraph.getModel().endUpdate();
      
      this.mgraphComponent = new mxGraphComponent(this.mgraph);
      this.mgraphComponent.setBackground(Color.RED);
      this.mgraphComponent.getViewport().setOpaque(true);
      
      this.mgraphComponent.setConnectable(false);
      
      this.panelMainFrame.add(new JLabel(), "First");
      this.panelMainFrame.add(new JLabel(), "West");
      this.panelMainFrame.add(new JLabel(), "East");
      this.panelMainFrame.add(new JLabel(), "Last");
      this.panelMainFrame.add(this.mgraphComponent, "Center");
      
      generateGraph();
      
      this.panelMainFrame.setBackground(Color.blue);
      this.mERMFrame.add(this.panelMainFrame);
      
      this.mERMFrame.setSize(500, 500);
      

      Dimension d2 = this.mERMFrame.getToolkit().getScreenSize();
      
      //Etie d2
      this.mERMFrame.setLocation((int)((d2.getWidth() - this.mERMFrame.getWidth()) / 2.0D), 
        (int)((d2.getHeight() - this.mERMFrame.getHeight()) / 2.0D));
      
      this.mERMFrame.setIconImage(gimp_S2P);
      this.mERMFrame.setVisible(true);
    }
  }
  
  private void generateGraph()
  {
    LogFileHandler.mlogger.info("generateGraph");
    
    this.mgraph.getModel().beginUpdate();
    try
    {
      mxHierarchicalLayout layout = new mxHierarchicalLayout(this.mgraph);
      layout.setIntraCellSpacing(10.0D);
      layout.setOrientation(7);
      layout.execute(this.mgraph.getDefaultParent());
    }
    catch (Exception e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    catch (Error err)
    {
      LogFileHandler.mlogger.severe(err.getMessage());
    }
    finally
    {
      this.mgraph.getModel().endUpdate();
    }
  }
  
  private String generateVertexString()
  {
    LogFileHandler.mlogger.info("generateVertexString");
    
    StringBuffer sbHtmlCode = new StringBuffer();
    
    sbHtmlCode.append("<html>");
    
    sbHtmlCode.append("<div align='center' color='red'><b>");
    sbHtmlCode.append("Vertexname");
    sbHtmlCode.append("</b></div>");
    
    sbHtmlCode.append("<hr noshade size='3'>");
    
    sbHtmlCode.append("<table cellspacing=0 cellpadding=2>");
    
    sbHtmlCode.append("<tr><td><i>");
    sbHtmlCode.append("col1");
    sbHtmlCode.append("</i></td><td>");
    sbHtmlCode.append("col2");
    sbHtmlCode.append("</td></tr>");
    

    sbHtmlCode.append("</table>");
    

    sbHtmlCode.append("</html>");
    
    return sbHtmlCode.toString();
  }
  
  private String generateVertexHeader(String strTabellenname)
  {
    LogFileHandler.mlogger.info("generateVertexHeader");
    
    StringBuffer sbHtmlCode = new StringBuffer();
    
    sbHtmlCode.append("<html>");
    
    sbHtmlCode.append("<div align='center' color='red'><b>");
    sbHtmlCode.append(strTabellenname);
    sbHtmlCode.append("</b></div>");
    
    sbHtmlCode.append("<hr noshade size='3'>");
    
    sbHtmlCode.append("<table cellspacing=0 cellpadding=2><tr>");
    
    return sbHtmlCode.toString();
  }
  
  private String generateVertexColumns(String strCol)
  {
    LogFileHandler.mlogger.info("generateVertexColumns");
    
    StringBuffer sbHtmlCode = new StringBuffer();
    String[] strTeile = strCol.split(":");
    if (strTeile.length == 2)
    {
      sbHtmlCode.append("<tr><td><i>");
      sbHtmlCode.append(strTeile[1]);
      sbHtmlCode.append("</i></td><td>");
      sbHtmlCode.append(strTeile[0]);
      sbHtmlCode.append("</td></tr>");
    }
    else if (strTeile.length == 1)
    {
      sbHtmlCode.append("<tr><td><i>");
      sbHtmlCode.append(" ");
      sbHtmlCode.append("</i></td><td>");
      sbHtmlCode.append(strTeile[0]);
      sbHtmlCode.append("</td></tr>");
    }
    else
    {
      sbHtmlCode.append("<tr><td><i>");
      sbHtmlCode.append(" ");
      sbHtmlCode.append("</i></td><td>");
      sbHtmlCode.append(" ");
      sbHtmlCode.append("</td></tr>");
    }
    return sbHtmlCode.toString();
  }
  
  private String generateCloseVertex()
  {
    StringBuffer sbHtmlCode = new StringBuffer();
    sbHtmlCode.append("</table>");
    
    sbHtmlCode.append("</html>");
    
    return sbHtmlCode.toString();
  }
  
  public static String strReplace(String strReplace)
  {
    String strRep = "";
    
    strRep = strReplace;
    
    strRep = strRep.replace("<html>", "");
    strRep = strRep.replace("</html>", "");
    strRep = strRep.replaceAll("(<div style=\"color:)*", "");
    strRep = strRep.substring(strRep.indexOf(">") + 1, strRep.length());
    strRep = strRep.replaceAll("\">", "");
    strRep = strRep.replace("</div>", "");
    
    strRep = strRep.replace("<i>", "");
    strRep = strRep.replace("</i>", "");
    strRep = strRep.replace("]", "");
    
    return strRep;
  }
}

