/**
 * 
 */
package gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;

import postgres.Tabelle;

import com.mxgraph.layout.hierarchical.mxHierarchicalLayout;
import com.mxgraph.swing.mxGraphComponent;
import com.mxgraph.util.mxConstants;
import com.mxgraph.view.mxGraph;
import com.mxgraph.view.mxStylesheet;


/**
 * @author Bill
 *
 *Klasse die für das erzeugen eines entsprechendem ERR Modell zuständig ist
 */
public class ERMGUI {

	public JFrame mERMFrame;
	JPanel panelMainFrame;
	
	private mxGraphComponent mgraphComponent;
	private mxGraph mgraph;
	
	/**
	 * 
	 */
	public ERMGUI()
	{
		mERMFrame = new JFrame();
		panelMainFrame = new JPanel(new BorderLayout(5,5));
		
		//graphComponent = new mxGraphComponent(null);
	}
	
	/*
	 * 
	 */
	//Hieran kann ich dann die Dateninhalte für die zuständigen View übergeben
	public void init(DefaultMutableTreeNode dmtnDatenbank, String strShowDB, ArrayList<String>alRelations)
	{
		
		mgraph = new mxGraph();
		mgraph.setAutoSizeCells(true);
		mgraph.setCellsBendable(false);
		mgraph.setCellsDeletable(false);
		mgraph.setCellsDisconnectable(false);
		mgraph.setCellsEditable(false);
		mgraph.setDisconnectOnMove(false);
		mgraph.setCellsCloneable(false);
		mgraph.setEdgeLabelsMovable(false);
	
		
		Map<String, Object> edge = new HashMap<String, Object>();
	    edge.put(mxConstants.STYLE_ROUNDED, true);
	    edge.put(mxConstants.STYLE_ORTHOGONAL, true);
	    edge.put(mxConstants.STYLE_EDGE, mxConstants.EDGESTYLE_TOPTOBOTTOM);
	    edge.put(mxConstants.STYLE_SHAPE, mxConstants.SHAPE_CONNECTOR);
	    edge.put(mxConstants.STYLE_ENDARROW, mxConstants.NONE);
	    edge.put(mxConstants.STYLE_VERTICAL_ALIGN, mxConstants.ALIGN_MIDDLE);
	    edge.put(mxConstants.STYLE_ALIGN, mxConstants.ALIGN_CENTER);
	    edge.put(mxConstants.STYLE_STROKECOLOR, "#FF0000"); // default is #6482B9
	    edge.put(mxConstants.STYLE_STROKEWIDTH,"3");

	    edge.put(mxConstants.STYLE_SOURCE_PERIMETER_SPACING,0);
	    
	    
	    edge.put(mxConstants.STYLE_FONTCOLOR, "#446299");

	   
	    
	    mxStylesheet stylesheet = mgraph.getStylesheet();
        stylesheet.putCellStyle("Begin", edge);
		
		
		
		Object parent = mgraph.getDefaultParent();

		
		
		
		HashMap<String, Object> hmCells = new HashMap<String, Object>();
		
		mgraph.getModel().beginUpdate();
		try
		{
			
			mgraph.setHtmlLabels( true);
			mgraph.setAutoSizeCells( true);
			mgraph.setCellsResizable(true);
			mgraph.setCellsBendable(false);
			mgraph.setCellsMovable(false);
						
			
			int iIndDatenbank =0;
			int iIndTabellen =0;
			
			
			
			for (Enumeration e = dmtnDatenbank.children() ; e.hasMoreElements() ;) {
		    	 
				String strDatenbank = (String) e.nextElement().toString();
				
				strDatenbank = this.strReplace(strDatenbank);
				
				if(strShowDB.equals(strDatenbank) == true)
				{
					//System.out.println("Show find");
					
					//System.out.println(strDatenbank);
					mERMFrame.setTitle("ERM - " + strDatenbank);
					
					iIndTabellen = 0;
					
					//Ab hier werden dann die verschiedenen Keys gezeichnet
					for (Enumeration e1 = dmtnDatenbank.getChildAt(iIndDatenbank).children();  e1.hasMoreElements() ;) 
					{
						String strTabellen = (String) e1.nextElement().toString();
						strTabellen = this.strReplace(strTabellen);
						
						strTabellen = strTabellen.substring(0,strTabellen.indexOf(Tabelle.sbTableRowDelimiter.toString()));
						//System.out.println(strTabellen);
						String strVertexCode = "";
					
						strVertexCode = this.generateVertexHeader(strTabellen);
						
						for (Enumeration e2 = dmtnDatenbank.getChildAt(iIndDatenbank).getChildAt(iIndTabellen).children();  e2.hasMoreElements() ;) 
						{
							String strSpalten = (String) e2.nextElement().toString();
							strSpalten = this.strReplace(strSpalten);
							
							//System.out.println("\t\t"+strSpalten);
							strVertexCode += this.generateVertexColumns(strSpalten);
						}
					
						strVertexCode += this.generateCloseVertex();
						
						Object cell = mgraph.insertVertex(parent, "Begin", strVertexCode, 120, 120, 20, 20);
						
						
						
						cell = mgraph.updateCellSize( cell);
						
						//Hier muss ich noch <leerzeichen>-<leerzeichen>zahl entfernen, damit das digramm richtig dargestelkt wird
						
						hmCells.put(strTabellen, cell);
						iIndTabellen++;
					}
					
					
					
					break;
				}
				
				
				
				 
				iIndDatenbank++;
		     }

			
			
			
		        
		        for (int i=0; i<alRelations.size(); i++)
				{
					String strSplitRelation [] = alRelations.get(i).split(":");
					
					mgraph.insertEdge(parent, null, "", hmCells.get(strSplitRelation[0]), hmCells.get(strSplitRelation[1]),"Begin");
				}
			
		        
		        
		       
		        
		//	mgraph.insertEdge(parent, null, "Edge", v1, v2,"startArrow=diamond;endArrow=none;strokeWidth=1;starteSize=12;strokeColor=#66FF00; startFill=0;edgeStyle=orthogonalEdgeStyle");
			
			
		//	mgraph.insertEdge(parent, null, "Edge1", cell, v1,"Begin");
		}
		finally
		{
			mgraph.getModel().endUpdate();
			
			mgraph.getModel().beginUpdate();
			mgraph.setAutoSizeCells( true);
			Iterator<String> itCells = hmCells.keySet().iterator();
		    while(itCells.hasNext())
		    {
		    	mgraph.updateCellSize(hmCells.get(itCells.next()));
		    }
		    mgraph.getModel().endUpdate(); 
		        
		        
		}

		mgraphComponent = new mxGraphComponent(mgraph);
		mgraphComponent.setBackground(Color.RED);
		mgraphComponent.getViewport().setOpaque(true);
		
		
		
		mgraphComponent.setConnectable(false);
		
		
		
		
		panelMainFrame.add( new JLabel(), BorderLayout.PAGE_START );
		panelMainFrame.add( new JLabel(), BorderLayout.WEST);
		panelMainFrame.add( new JLabel(), BorderLayout.EAST);
		panelMainFrame.add( new JLabel(), BorderLayout.PAGE_END);
		panelMainFrame.add(mgraphComponent,BorderLayout.CENTER);
		
		

		generateGraph();
		
		
		panelMainFrame.setBackground(Color.blue);
		
		
		mERMFrame.add(panelMainFrame);
		
		
		mERMFrame.setSize(500, 500);
		//mERMFrame.pack();
		

		Dimension d = mERMFrame.getToolkit().getScreenSize(); 
		 
		this.mERMFrame.setLocation((int) ((d.getWidth() - this.mERMFrame.getWidth()) / 2), (int) ((d.getHeight() - 
				this.mERMFrame.getHeight()) / 2));
		
		
		mERMFrame.setVisible(true);
		//mERMFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	
		
	}
	
	
	private void generateGraph()
	{
		
		mgraph.getModel().beginUpdate();
		try {
			//Hierarchical Layout
			mxHierarchicalLayout layout = new mxHierarchicalLayout(mgraph);
	        layout.setIntraCellSpacing(10);
	        layout.setOrientation(SwingConstants.WEST);
        	layout.execute(mgraph.getDefaultParent());
		} finally {
			mgraph.getModel().endUpdate();
		}
		
	}
	
	
	
	private String generateVertexString()
	{
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
		StringBuffer sbHtmlCode = new StringBuffer();
		String strTeile [] = strCol.split(":");
		
			sbHtmlCode.append("<tr><td><i>");
				sbHtmlCode.append(strTeile[1]);
			sbHtmlCode.append("</i></td><td>");
				sbHtmlCode.append(strTeile[0]);
			sbHtmlCode.append("</td></tr>");
				
			
		return sbHtmlCode.toString();
	
		
	}	
	
	
	private String generateCloseVertex()
	{
		StringBuffer sbHtmlCode = new StringBuffer();
		sbHtmlCode.append("</table>");
	
		
		sbHtmlCode.append("</html>");

		return sbHtmlCode.toString();
	}
	
	
	public String strReplace(String strReplace)
	{
		String strRep = "";
		
		strRep = strReplace;
		
		strRep = strRep.replace("<html>", "");
		strRep = strRep.replace("</html>", "");
		//strRep = strRep.replace("<div style=\"color:green\">", "");
		strRep = strRep.replaceAll("(<div style=\"color:)*", "");
		strRep = strRep.substring(strRep.indexOf(">")+1, strRep.length());
		strRep = strRep.replaceAll("\">", "");
		strRep = strRep.replace("</div>","");
		
		strRep = strRep.replace("<i>","");
		strRep = strRep.replace("</i>","");
		strRep = strRep.replace("]","");
		
		//erstmal noch mit ersetzen
		//strRep = strRep.replace("<b>","");
		//strRep = strRep.replace("</b>","");
		//strRep = strRep.replace("<u>","");
		//strRep = strRep.replace("</u>","");
		
		
		
		return strRep;
	}
	
	
}
